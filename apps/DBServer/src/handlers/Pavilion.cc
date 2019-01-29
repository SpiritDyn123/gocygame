#include "../core/dispatcher.h"
#include "../core/redis_client.h"
#include "../proto/db.pb.h"
#include "../proto/SvrProtoID.pb.h"
#include "../proto/Battle.pb.h"
#include "KeyPrefixDef.h"
#include "Pavilion.h"
#include "RankData.h"
#include "GeoData.h"

Pavilion gPavilion;

using namespace std;

const int kMaxPavilionLogCnt = 50;

// 只保存一定量的日志
static const string kScriptWriteLog =
"local key = KEYS[1]\n"
"local limit = tonumber(ARGV[1])\n"
"for i = 2, #ARGV do\n "
"	local r = redis.call('LPUSH', key, ARGV[i])\n"
"end\n"
"local r = redis.call('LTRIM', key, 0, limit-1)\n"
"return 0";


Pavilion::Pavilion()
{
	gMsgDispatcher.RegisterHandler(DBProtoPavilionChallenge, *this, &Pavilion::Challenge, nullptr, new cs::PavilionInfo);
	gMsgDispatcher.RegisterHandler(DBProtoPavilionGetLog, *this, &Pavilion::GetLog, nullptr, new cs::PavilionLog);
	gMsgDispatcher.RegisterHandler(DBProtoPavilionWriteLog, *this, &Pavilion::onWriteLog, new cs::PavilionLog, new db::PavilionExtraData);
	gMsgDispatcher.RegisterHandler(DBProtoPavilionClearLog, *this, &Pavilion::onClearLog, nullptr, nullptr);
	
	//gMsgDispatcher.RegisterHandler(DBProtoPavilionUpdateLevel, *this, &Pavilion::UpdateScore, new db::PavilionUpdateLevel);
}

void Pavilion::Init()
{

}


uint32_t Pavilion::GetPlayerPavilionScore(uint32_t plid)
{
	string strPlayerID = to_string(plid);
	string key(kKeyPrefixPlayerData + strPlayerID);
	unordered_map<string, string> m;
	string& pavilionInfoStr = m[kPavilion];
	if (!gRedis->hget(key, m)) {
		WARN_LOG("hgetall failed: %s! plid=%u", gRedis->last_error_cstr(), plid);
		return ErrCodeDB;
	}
	cs::PavilionInfo info;
	info.ParseFromString(pavilionInfoStr);
	return info.base_info().score();
}

ErrCodeType Pavilion::Challenge(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	std::string key(kKeyPrefixPlayerData + to_string(h.TargetID));
	unordered_map<string, string> m;
	auto& pavilionStr = m[kPavilion];
	if (!gRedis->hget(key, m)) {
		WARN_LOG("hget failed: %s!", gRedis->last_error_message().c_str());
		return ErrCodeDB;
	}

	// 判断该玩家是否存在
	bool pavilionFound = false;
	for (const auto& v : m) {
		if (v.second.size()) {
			pavilionFound = true;
			break;
		}
	}
	if (!pavilionFound) {
		DEBUG_LOG("the pavilion not inited! plid=%u target=%u proto=%u origProto=%u",
			h.PlayerID, h.TargetID, h.ProtoID, h.OrigProtoID);
		return ErrCodePavilionTargetNotOpenPavilion;
	}

	db::PlayerData playerData;
	auto& pavilionData = *playerData.mutable_pavilion_data();
	pavilionData.ParseFromString(pavilionStr);
	REAL_PROTOBUF_MSG(outMsg, cs::PavilionInfo, rsp);
	rsp.CopyFrom(pavilionData);
	return ErrCodeType::ErrCodeSucc;
}

ErrCodeType Pavilion::GetLog(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(outMsg, cs::PavilionLog, rsp);
	const auto& key = makePavilionLogKey(h.PlayerID);
	std::vector<std::string> vals;

	if (gRedis->lrange(key, 0, kMaxPavilionLogCnt - 1, vals)) {
		for (const auto& v : vals) {
			rsp.add_records()->ParseFromString(v);
		}
	} else {
		DEBUG_LOG("redis lrange err. plid=%u", h.PlayerID);
		return ErrCodeDB;
	}
	return ErrCodeType::ErrCodeSucc;
}

ErrCodeType Pavilion::onWriteLog(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, cs::PavilionLog, req);
	REAL_PROTOBUF_MSG(outMsg, db::PavilionExtraData, rsp);
	const auto& logKey = makePavilionLogKey(h.TargetID);
	uint32_t tarPlayerID = h.TargetID;
	if (!tarPlayerID) {
		DEBUG_LOG("failed to write pavilion challenge log. tarID=0");
		return ErrCodeDB;
	}
	string s;
	std::vector<std::string> vals;
	string strPlayerID = to_string(tarPlayerID);

	// 道馆防守数据
	std::string pavilionExtraStr;
	if (!gRedis->get(makePavilionExtraDataKey(tarPlayerID), pavilionExtraStr)) {
		WARN_LOG("get pavilionExtraData failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}
	rsp.ParseFromString(pavilionExtraStr);


	std::vector<string> scriptKeys = {logKey};
	std::vector<string> scriptVals = {std::to_string(kMaxPavilionLogCnt) };
	for (const auto& record : req.records() ) {
		s.clear();
		record.SerializeToString(&s);
		vals.push_back(s);
		scriptVals.push_back(s);
		if (record.is_win()) {
			rsp.set_total_defend_lose_cnt(rsp.total_defend_lose_cnt() + 1);
		} else {
			rsp.set_total_defend_win_cnt(rsp.total_defend_win_cnt() + 1);
		}
	}
	pavilionExtraStr.clear();
	rsp.SerializeToString(&pavilionExtraStr);

	// 写防守日志
	ScopedReplyPointer reply = gRedisRank->eval(kScriptWriteLog, &scriptKeys, &scriptVals);
	CHECK_REPLY_EC(reply, REDIS_REPLY_INTEGER, tarPlayerID);
	//gRedis->rpush(logKey, vals);
	// 写extradata
	if (!gRedis->set(makePavilionExtraDataKey(tarPlayerID), pavilionExtraStr)) {
		WARN_LOG("set pavilionExtraData failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}
	DEBUG_LOG("add log. targetId=%u", h.TargetID);
	return ErrCodeType::ErrCodeSucc;
}


ErrCodeType Pavilion::onClearLog(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	const auto& logKey = makePavilionLogKey(h.TargetID);
	std::vector<std::string> delKeys {logKey };
	gRedis->del(delKeys);
	DEBUG_LOG("del pavilion log. targetId=%u", h.TargetID);
	return ErrCodeType::ErrCodeSucc;
}

ErrCodeType Pavilion::UpdateScore(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	/*REAL_PROTOBUF_MSG(inMsg, db::PavilionUpdateLevel, req);
	
	cs::GeoPos geo;
	gGeoData.GetGeoData(h.PlayerID, geo);
	geo.set_game_region(req.game_region());
	auto strPid = std::to_string(h.PlayerID);
	rankData.DoSetGeoRank(strPid, req.level(), 0, 0, geo, kRankFieldPavilionRank, true, false, false);*/
	return ErrCodeType::ErrCodeSucc;
}



