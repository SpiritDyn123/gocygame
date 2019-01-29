/*===============================================================
* @Author: car
* @Created Time : 2017年06月26日 星期一 15时02分05秒
*
* @File Name: RankData.cc
* @Description:
*
================================================================*/

#include <string>
#include "../proto/SvrProtoID.pb.h"
#include "../core/dispatcher.h"
#include "../core/redis_client.h"

#include "KeyPrefixDef.h"
#include "RankData.h"
#include "GeoData.h"
#include "PlayerData.h"
#include "Arena.h"
#include <iomanip>
#include "BehaviourRank.h"
#include "Pavilion.h"

using namespace std;


const string RankData::kGRankInfoKey;
const string RankData::kRankKey;
const string kRankSplit("#");


RankData rankData;

//---------------------------------------------------
// 每个榜保存1000名
static const string kScriptSetMultiGeoRank =
"local limit = (#ARGV == 3) and tonumber(ARGV[3]) or 999\n"
"for i = 1, #KEYS do\n"
"  local r = redis.call('ZREVRANGE', KEYS[i], limit, limit, 'WITHSCORES')\n"
"  if r[2] == nil or (r[2] ~= nil and tonumber(r[2])<tonumber(ARGV[2])) then\n"
"    redis.call('ZADD', KEYS[i], ARGV[2], ARGV[1])\n"
"  end\n"
"end\n"
"return 0";

static const string kScriptSetMultiGeoRankUnconditional =
"for i = 1, #KEYS do\n"
"	redis.call('ZADD', KEYS[i], ARGV[2], ARGV[1])\n"
"	if tonumber(ARGV[3]) > 0 then\n"
"		redis.call('EXPIREAT', KEYS[i], ARGV[3])\n"
"	end\n"
"end\n"
"return 0";

// 从榜中删除
static const string kScriptDelMultiGeoRank =
"for i = 1, #KEYS do\n"
"  redis.call('ZREM', KEYS[i], ARGV[1])\n"
"end\n"
"return 0";

static const string kScriptIncMultiGeoRank =
"for i = 1, #KEYS do\n"
"  local r = redis.call('ZSCORE', KEYS[i], ARGV[1])\n"
"  if r then\n"
"    if tonumber(r) + ARGV[2] > 0 then\n"
"      redis.call('ZINCRBY', KEYS[i], ARGV[2], ARGV[1])\n"
"    else\n"
"      redis.call('ZREM', KEYS[i], ARGV[1])\n"
"    end\n"
"  end\n"
"end\n"
"return 0";


RankData::RankData()
{
	gMsgDispatcher.RegisterHandler(DBProtoRankGetList, *this, &RankData::getRankList, new db::GetRankListReq, new db::GetRankListRsp);	
	gMsgDispatcher.RegisterHandler(DBProtoRankGetByKey, *this, &RankData::getRankByKey, new db::GetRankByKeyReq, new db::GetRankByKeyRsp);	
	gMsgDispatcher.RegisterHandler(DBProtoRankUpdateScore, *this, &RankData::updateRank, new db::UpdateScoreToRankReq, new db::UpdateScoreToRankRsp);	
	gMsgDispatcher.RegisterHandler(DBProtoRankDelScore, *this, &RankData::delRank, new db::DelScoreToRankReq, new db::DelScoreToRankRsp);	
	gMsgDispatcher.RegisterHandler(DBProtoClearRank, *this, &RankData::clearRank, new db::ClearRankReq, new db::ClearRankRsp);	
	gMsgDispatcher.RegisterHandler(DBProtoDelGeoRankVec, *this, &RankData::delGeoRankVec, new db::DelGeoRankVec);
}

void RankData::RestoreRank() 
{	
}

ErrCodeType RankData::getRankByKey(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{

	REAL_PROTOBUF_MSG(inMsg, db::GetRankByKeyReq, req);
	REAL_PROTOBUF_MSG(outMsg, db::GetRankByKeyRsp, rsp);

	uint32_t targetID = h.TargetID;
	string rankKey = MakeRankKey(req.rank_key());

	long long pos = 0;
	if (req.desc()) {
		if (!gRedisRank->zrevrank(rankKey, to_string(req.plid()), &pos)) {
			WARN_LOG("zrevrank failed: %s! plid=%u", gRedisRank->last_error_cstr(), targetID);
			return ErrCodeDB;

		}
	} else {
		if (!gRedisRank->zrank(rankKey, to_string(req.plid()), &pos)) {
			WARN_LOG("zrank failed: %s! plid=%u", gRedisRank->last_error_cstr(), targetID);
			return ErrCodeDB;

		}
	}

	rsp.mutable_rank_key()->CopyFrom(req.rank_key());
	rsp.set_rank(pos);

	return ErrCodeSucc;	
}


ErrCodeType RankData::getRankList(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::GetRankListReq, req);
	REAL_PROTOBUF_MSG(outMsg, db::GetRankListRsp, rsp);

	uint32_t targetID = h.TargetID;
	// 设置排行榜的key
	stringstream ssRankName;
	ssRankName << GetRankName(req.rank_name(), req.geo().game_region());
	if (req.has_geo()) {
		ssRankName << GeoData::GetGeoRankStr(req.geo());
	}
	req.mutable_rank_key()->set_rank_name(ssRankName.str());
	string rankKey = MakeRankKey(req.rank_key());
	//DEBUG_LOG("get rank data. plid=%u, rankKey=%s", targetID, rankKey.c_str())	;
	if (req.following_size() > 0) {
		GetFollowingRankList(req, rsp); // 拉取关注的
	} else {
		vector<string> result;
		if (req.desc()) {
			if (!gRedisRank->zrevrange(rankKey, req.from_pos(), req.to_pos(), result, req.withscore())) {
				WARN_LOG("zrevrange failed: %s! plid=%u", gRedisRank->last_error_cstr(), targetID);
				return ErrCodeDB;
			}
		} else {
			if (!gRedisRank->zrange(rankKey, req.from_pos(), req.to_pos(), result, req.withscore())) {
				WARN_LOG("zrange failed: %s! plid=%u", gRedisRank->last_error_cstr(), targetID);
				return ErrCodeDB;
			}
		}
		
		auto iter = result.begin();
		for (; iter != result.end(); ++iter) {
			auto* data = rsp.add_datas();
			data->set_plid(atoi(iter->c_str()));
			if (req.withscore()) {
				++iter;
				data->set_score(atoi(iter->c_str()));
			}
		}
	}

	// 竞技场自己的分数从数据库里读取
	if (req.rank_name() == cs::RankNameArena) {
		req.set_my_score(gArena.GetPlayerScore(targetID, req.geo().game_region()));
	}

	rsp.mutable_rank_key()->CopyFrom(req.rank_key());
	rsp.set_withscore(req.withscore());
	rsp.set_my_score(req.my_score());
	
	return ErrCodeSucc;	
}

static const string kScriptUpdateRankForce =
		"redis.call('ZREM', KEYS[1], ARGV[1])"
		"local r = redis.call('ZADD', KEYS[1], ARGV[2])\n"
		"if r == 1 then\n"
		"  return 0\n"
		"end\n"
		"return 1";
	


ErrCodeType RankData::updateRank(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::UpdateScoreToRankReq, req);
	REAL_PROTOBUF_MSG(outMsg, db::UpdateScoreToRankRsp, rsp);
	
	string strPlayerID = to_string(req.plid());
	string rankKey = MakeRankKey(req.rank_key());
	//DEBUG_LOG("update rank data. plid=%u, key=%s, data=%s", req.plid(), rankKey.c_str(), req.DebugString().c_str());
	double score = req.score();
	if (req.auto_time_trial()) {
		score += GetTimeTail();
	}
	vector<string> keys = { rankKey };
	if (req.way() == 0) {
		vector<string> args = { strPlayerID, to_string(score) };
		ScopedReplyPointer reply = gRedisRank->eval(kScriptIncMultiGeoRank, &keys, &args);
		CHECK_REPLY_EC(reply, REDIS_REPLY_INTEGER, atoi(strPlayerID.c_str()));
	} else if (req.way() == 1) {
		vector<string> args = { strPlayerID, to_string(score) };
		ScopedReplyPointer reply = gRedisRank->eval(kScriptSetMultiGeoRank, &keys, &args);
		CHECK_REPLY_EC(reply, REDIS_REPLY_INTEGER, atoi(strPlayerID.c_str()));
	}

	rsp.mutable_rank_key()->CopyFrom(req.rank_key());
	rsp.set_plid(req.plid());
	rsp.set_way(req.way());

	return ErrCodeSucc;	
}

ErrCodeType RankData::delRank(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::DelScoreToRankReq, req);
	REAL_PROTOBUF_MSG(outMsg, db::DelScoreToRankRsp, rsp);
	uint32_t targetID = h.TargetID;

	string rankKey = MakeRankKey(req.rank_key());

	vector<string> fields;
	fields.emplace_back(to_string(req.plid()));
	if (!gRedisRank->zrem(rankKey, fields)) {
		WARN_LOG("zrank failed: %s! plid=%u", gRedisRank->last_error_cstr(), targetID);
		return ErrCodeDB;
	}

	rsp.mutable_rank_key()->CopyFrom(req.rank_key());
	rsp.set_plid(req.plid());
	return ErrCodeSucc;	
}

ErrCodeType RankData::delGeoRankVec(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::DelGeoRankVec, req);
	DEBUG_LOG("onDelPlayerRankInfo pid:%u", req.pid());

	const std::vector<std::string> rankKeys = {
		kSocialFieldLove,
		kSocialFieldPopular,
		kSocialFieldWealth,
		kRankFieldArenaRank,
		"TrialTower",
	};
	std::vector<uint32_t> validKey;
	if (req.key_id_size() == 0) {
		for (uint32_t i = 0; i < rankKeys.size(); ++i) {
			req.add_key_id(i);
		}
	}
	std::string pid = std::to_string(req.pid());
	for (auto i : req.key_id()) {
		DoDelGeoRank(pid, req.geo(), rankKeys[i], true, true, true);
	}

	gBehaviourRank.DelRankInfo(req.pid(), req.geo().game_region());

	return ErrCodeSucc;
}

static const string kScriptCopyRank =
		"if redis.call('EXISTS', KEYS[2]) == 1 then\n"
		"  return 1\n"
		"end\n"
		"redis.call('RESTORE', KEYS[2], 0, redis.call('DUMP', KEYS[1]))\n"
		"return 0";

ErrCodeType RankData::clearRank(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::ClearRankReq, req);
	REAL_PROTOBUF_MSG(outMsg, db::ClearRankRsp, rsp);

	string rankKey = MakeRankKey(req.rank_key());

	db::RankKey snapRankKeyObj;
	snapRankKeyObj.CopyFrom(req.rank_key());
	snapRankKeyObj.set_rank_deadtm(req.clear_time());
	string snapRankKey = MakeRankKey(snapRankKeyObj);
	string progressKey(kKeyPrefixClearRankProgress + snapRankKey);
	string progress;
	if (!req.start_pos()) {
		vector<string> rankKeys = { rankKey, snapRankKey };
		ScopedReplyPointer replyCopyRank = gRedisRank->eval(kScriptCopyRank, &rankKeys);
		CHECK_REPLY_EC(replyCopyRank, REDIS_REPLY_INTEGER, 0);
		if (!gRedisRank->get(progressKey, progress)) {
			WARN_LOG("get failed: %s!", gRedisRank->last_error_cstr());
			return ErrCodeDB;
		}
		rsp.set_cleared_pos(atoi(progress.c_str()));
	} else {
		rankKey = snapRankKey;
		progress = to_string(req.start_pos());
		if (!gRedisRank->set(progressKey, progress)) {
			WARN_LOG("set failed: %s!", gRedisRank->last_error_cstr());
			return ErrCodeDB;
		}
		rsp.set_cleared_pos(req.start_pos());
	}

	vector<string> result;
	if (req.desc()) {
		if (!gRedisRank->zrevrange(rankKey, req.start_pos(), req.start_pos() + req.batch() - 1, result, true)) {
			WARN_LOG("zrevrank failed: %s!", gRedisRank->last_error_cstr());
			return ErrCodeDB;
		}
	} else {
		if (!gRedisRank->zrange(rankKey, req.start_pos(), req.start_pos() + req.batch() - 1, result, true)) {
			WARN_LOG("zrank failed: %s!", gRedisRank->last_error_cstr());
			return ErrCodeDB;
		}
	}

	rsp.mutable_clear_req()->Swap(&req);
	auto iter = result.begin();
	for(; iter != result.end(); ++iter) {
		auto* data = rsp.mutable_rank_data()->add_datas();
		data->set_plid(atoi(iter->c_str()));
		++iter;
		data->set_score(atoi(iter->c_str()));
	}
	return ErrCodeSucc;	
}


std::string RankData::MakeRankKey(const db::RankKey& rankKey)
{
	std::stringstream ss;
	ss << "{Rank}"
		<< kRankSplit << rankKey.rank_type()
		<< kRankSplit << rankKey.rank_index()
		<< kRankSplit << rankKey.rank_deadtm()
		<< kRankSplit << rankKey.rank_name();
	//DEBUG_LOG("MakeRankKey key=%s", ss.str().c_str());
	return ss.str();
}

ErrCodeType RankData::DoSetGeoRank(string& strPlayerID, double totalScore, double weekScore, double dailyScore, 
	const cs::GeoPos& geo, const string& rankName, bool common, bool week, bool daily)
{
	vector<string> keys;
	if (common) {
		GeoData::GetGeoRankKeys(RankData::kCommon, geo, rankName, keys);
		vector<string> args = { strPlayerID, to_string(totalScore) };
		ScopedReplyPointer reply = gRedisRank->eval(kScriptSetMultiGeoRank, &keys, &args);
		CHECK_REPLY_EC(reply, REDIS_REPLY_INTEGER, atoi(strPlayerID.c_str()));
	}

	if (week) {
		keys.clear();
		GeoData::GetGeoRankKeys(RankData::kWeek, geo, rankName, keys);
		uint32_t expireTm = NextMonday() + 604800 + 86400; // 设置一周后失效(多加一天)
		vector<string> args = { strPlayerID, to_string(weekScore), to_string(expireTm) };
		ScopedReplyPointer reply = gRedisRank->eval(kScriptSetMultiGeoRankUnconditional, &keys, &args);
		CHECK_REPLY_EC(reply, REDIS_REPLY_INTEGER, atoi(strPlayerID.c_str()));
	}

	if (daily) {
		keys.clear();
		GeoData::GetGeoRankKeys(RankData::kDaily, geo, rankName, keys);
		uint32_t expireTm = NextDay() + 86400 + 3600; // 设置一天后失效(多加一小时)
		vector<string> args = { strPlayerID, to_string(dailyScore), to_string(expireTm) };
		ScopedReplyPointer reply = gRedisRank->eval(kScriptSetMultiGeoRankUnconditional, &keys, &args);
		CHECK_REPLY_EC(reply, REDIS_REPLY_INTEGER, atoi(strPlayerID.c_str()));
	}
	
	return ErrCodeSucc;
}

ErrCodeType RankData::DoDelGeoRank(string& strPlayerID, const cs::GeoPos& geo, const string& rankName, bool common, bool week, bool daily)
{
	DEBUG_LOG("DoDelGeoRank: %s, pid: %s", rankName.c_str(), strPlayerID.c_str());
	if (common) {
		vector<string> keys;
		GeoData::GetGeoRankKeys(RankData::kCommon, geo, rankName, keys);
		vector<string> args = { strPlayerID };
		ScopedReplyPointer reply = gRedisRank->eval(kScriptDelMultiGeoRank, &keys, &args);
		CHECK_REPLY_EC(reply, REDIS_REPLY_INTEGER, atoi(strPlayerID.c_str()));
	}

	if (week) {
		vector<string> keys;
		GeoData::GetGeoRankKeys(RankData::kWeek, geo, rankName, keys);
		vector<string> args = { strPlayerID };
		ScopedReplyPointer reply = gRedisRank->eval(kScriptDelMultiGeoRank, &keys, &args);
		CHECK_REPLY_EC(reply, REDIS_REPLY_INTEGER, atoi(strPlayerID.c_str()));
	}

	if (daily) {
		vector<string> keys;
		GeoData::GetGeoRankKeys(RankData::kDaily, geo, rankName, keys);
		vector<string> args = { strPlayerID };								
		ScopedReplyPointer reply = gRedisRank->eval(kScriptDelMultiGeoRank, &keys, &args);
		CHECK_REPLY_EC(reply, REDIS_REPLY_INTEGER, atoi(strPlayerID.c_str()));
	}

	return ErrCodeSucc;
}

ErrCodeType RankData::DoIncGeoRank(string& strPlayerID, int inc, const cs::GeoPos& geo, const string& rankName, bool common, bool week, bool daily)
{
	if (common) {
		vector<string> keys;
		GeoData::GetGeoRankKeys(RankData::kCommon, geo, rankName, keys);
		vector<string> args = { strPlayerID, to_string(inc) };
		ScopedReplyPointer reply = gRedisRank->eval(kScriptIncMultiGeoRank, &keys, &args);
		CHECK_REPLY_EC(reply, REDIS_REPLY_INTEGER, atoi(strPlayerID.c_str()));
	}

	if (week) {
		vector<string> keys;
		GeoData::GetGeoRankKeys(RankData::kWeek, geo, rankName, keys);
		vector<string> args = { strPlayerID, to_string(inc) };
		ScopedReplyPointer reply = gRedisRank->eval(kScriptIncMultiGeoRank, &keys, &args);
		CHECK_REPLY_EC(reply, REDIS_REPLY_INTEGER, atoi(strPlayerID.c_str()));
	}

	if (daily) {
		vector<string> keys;
		GeoData::GetGeoRankKeys(RankData::kDaily, geo, rankName, keys);
		vector<string> args = { strPlayerID, to_string(inc) };
		ScopedReplyPointer reply = gRedisRank->eval(kScriptIncMultiGeoRank, &keys, &args);
		CHECK_REPLY_EC(reply, REDIS_REPLY_INTEGER, atoi(strPlayerID.c_str()));
	}

	return ErrCodeSucc;
}

std::string RankData::GetRankName(int rankName, int gameRegion)
{
	stringstream ss;
	switch (rankName) {
	case cs::RankNamePopular:
		ss << kSocialFieldPopular;
		break;
	case cs::RankNameWealth:
		ss << kSocialFieldWealth;
		break;
	case cs::RankNameWish:
		ss << kKeyPrefixWishData;
		break;
	case cs::RankNameArena:
		ss << kRankFieldArenaRank;
		break;
	case cs::RankNameTrialTowerPassRecord:
		ss << "TrialTower";
		break;
	case cs::RankNamePavilion:
		ss << kRankFieldPavilionRank;
		break;
	}
	ss << "_" << gameRegion;
	return ss.str();
}

ErrCodeType RankData::GetFollowingRankList(db::GetRankListReq& req, db::GetRankListRsp& rsp)
{
	// TODO: 是不是有优化性能的地方呢？？？

	string rankKey = MakeRankKey(req.rank_key());
	// 总榜从玩家数据中拉取(因为排行榜中不一定有数据)
	if (req.rank_key().rank_type() == cs::RankTypeCommon) {
		if (req.rank_name() == cs::RankNamePopular) {
			for (int i = 0; i < req.following_size(); i++) {
				auto* data = rsp.add_datas();
				data->set_plid(req.following(i));
				data->set_score(gPlayerData.GetPlayerTotalPopluar(req.following(i)));
			}
		} else if (req.rank_name() == cs::RankNameWealth) {
			for (int i = 0; i < req.following_size(); i++) {
				auto* data = rsp.add_datas();
				data->set_plid(req.following(i));
				data->set_score(gPlayerData.GetPlayerTotalWealth(req.following(i)));
			}
		} else if (req.rank_name() == cs::RankNameArena) {
			for (int i = 0; i < req.following_size(); i++) {
				auto* data = rsp.add_datas();
				data->set_plid(req.following(i));
				data->set_score(gArena.GetPlayerScore(req.following(i), req.geo().game_region()));
			}
		} else if(req.rank_name() == cs::RankNameTrialTowerPassRecord) {
			for (int i = 0; i < req.following_size(); i++) {
				auto* data = rsp.add_datas();
				data->set_plid(req.following(i));
				data->set_score(gPlayerData.GetPlayerTrialTowerRankScore(req.following(i)));
			}
		} else if(req.rank_name() == cs::RankNamePavilion) {
			for (int i = 0; i < req.following_size(); i++) {
				auto* data = rsp.add_datas();
				data->set_plid(req.following(i));
				data->set_score(gPavilion.GetPlayerPavilionScore(req.following(i)));
			}	
		}
		
		return ErrCodeSucc;
	}

	// 周榜、日榜从本期或者上期的排行榜里拉取
	for (int i = 0; i < req.following_size(); i++) {
		int32_t score = 0;
		string fidstr = std::to_string(req.following(i));
		if (!gRedisRank->zscore(rankKey, fidstr, score)) {
			WARN_LOG("zrevrange failed: %s! plid=%u", gRedisRank->last_error_cstr(), req.following(i));
			return ErrCodeDB;
		}
		auto* data = rsp.add_datas();
		data->set_plid(req.following(i)); 
		data->set_score(score);
	}

	return ErrCodeSucc;
}
