#include <vector>
#include <sstream>
#include <unordered_map>
#include <libant/time/time_utils.h>
#include <libant/utils/StringUtils.h>
#include "Arena.h"
#include "../core/dispatcher.h"
#include "../core/redis_client.h"
#include "../proto/db.pb.h"
#include "../proto/Battle.pb.h"
#include "../proto/CSCoreMsg.pb.h"
#include "../proto/SvrProtoID.pb.h"
#include "../proto/CommonMsg.pb.h"
#include "../proto/Arena.pb.h"
#include "../core/ScriptMgr.h"
#include "PlayerData.h"
#include "GeoData.h"
#include "RankData.h"
#include "../TimeUtils.h"
#include <StringUtils.h>
#include "MailData.h"
#include "../CSV/CSVArenaWeeklyBonus.h"
#include "../CSV/CSVArenaRewardDrop.h"
#include "../CSV/CSVArenaBonusDrop.h"
#include "../CSV/CSVArenaRegionScore.h"
#include "../CSV/CSVArenaWeeklyRankBonus.h"
#include "../CSV/CSVArenaTop30Award.h"

#include <functional>
#include <string>
#include "BtlReplay.h"
#include "CapRes.h"
#include "../global.h"
#include <time.h>
#include "../core/MysqlProxy.h"

namespace ArenaDrop {
	class Drops : public Singleton<Drops>
	{
	public:
		struct Item {
			Item(const std::vector<int>& info){
				type = info[0];
				id = info[1];
				cnt = info[2];
				exData.assign(info.begin() + 3, info.end());
			}
			void AttachToMail(cs::MailBase* mail);
			uint32_t type;
			uint32_t id;
			uint32_t cnt;
			std::vector<int> exData;
		};
	private:
		struct Drop {
			Drop() : rate(0) {}

			struct Info {
				uint32_t rate;
				std::vector<Item> items;
			};
			uint32_t id;
			uint32_t rate;
			std::vector<Info> infos;	//只掉落一组Info
		};
		struct DList {
			DList(uint32_t id, uint32_t c, uint32_t r) : infoId(id), cnt(c), rate(r) {}
			uint32_t infoId;		//指向Drop
			uint32_t cnt;
			uint32_t rate;
		};
		std::unordered_map<uint32_t, std::vector<DList> > mDlists;
		std::unordered_map<uint32_t, Drop> mDrops;
	public:
		void Load();
		void GetRewardDrop(uint32_t id, std::unordered_map<uint64_t, Item>& output) const;
	};

	void Drops::Item::AttachToMail(cs::MailBase* mail)
	{
		auto attach = mail->add_attach();
		attach->set_type((cs::PlayerResourceType)type);
		attach->set_id(id);
		attach->set_cnt(cnt);
		for (auto& v : exData) {
			attach->add_extra_datas(v);
		}
	}

	void Drops::Load()
	{
		mDrops.clear();
		mDlists.clear();
		for (auto it : gCSVArenaRewardDrop.AllItems) {
			auto& vec = mDlists[it.first];
			for (auto git : it.second->RewardDrop) {
				auto info = SplitAndToIntVec(git, '#');
				if (info.size() < 3) {
					continue;
				}
				vec.emplace_back((uint32_t)info[0],(uint32_t)info[1], (uint32_t)info[2]);
			}
		}
		for (auto it : gCSVArenaBonusDrop.AllItems) {
			uint32_t groupId = it.second->DropName;
			uint32_t rate = it.second->Weight;
			auto& group = mDrops[groupId];
			group.id = groupId;
			Drop::Info info;
			info.rate = rate;
			group.rate += rate;
			for (auto sIt : it.second->DropItemID) {
				auto arr = SplitAndToIntVec(sIt, '#');
				if (arr.size() < 3) {
					continue;
				}
				info.items.emplace_back(arr);
			}
			group.infos.push_back(info);
		}
	}
	void Drops::GetRewardDrop(uint32_t id, std::unordered_map<uint64_t, Item>& output) const
	{
		auto lit = mDlists.find(id);
		if (lit == mDlists.end()) {
			return;
		}
		auto& lvec = lit->second;
		for (auto& ld : lvec) {
			for (uint32_t i = 0; i < ld.cnt; ++i) {
				if (rand() % 100 >= ld.rate) {
					continue;
				}
				auto gIt = mDrops.find(ld.infoId);
				if (gIt == mDrops.end()) {
					break;
				}
				auto& group = gIt->second;
				if (group.rate == 0) {
					break;
				}
				int val = rand() % group.rate;
				int test = 0;
				for (auto& it : group.infos) {
					test += it.rate;
					if (val < test) {
						for (auto& sit : it.items) {
							if (sit.cnt == 0) {
								continue;
							}
							uint64_t tmpId = (uint64_t)sit.type << 32 | sit.id;
							auto mit = output.find(tmpId);
							if (mit != output.end()) {
								mit->second.cnt += sit.cnt;
							}
							else {
								output.insert(std::make_pair(tmpId, sit));
							}
						}
						break;
					}
				}
			}
		}
	}
};

Arena gArena;
static uint32_t sRegionScore[] = {
	200,
	500,
	800,
	1200,
	1600,
	2000,
	2500,
	3000,
};

Arena::Arena()
{
	gMsgDispatcher.RegisterHandler(DBProtoArenaGetTarget, *this, &Arena::GetTarget, new db::ArenaGetTar, new db::ArenaTarInfoArr);
	gMsgDispatcher.RegisterHandler(DBProtoArenaUpate, *this, &Arena::UpdateInfo, new db::ArenaUpdateArr);
	gMsgDispatcher.RegisterHandler(DBProtoArenaGetInfo, *this, &Arena::GetInfo, new cs::CSUint32Req, new cs::ArenaInfo);
	gMsgDispatcher.RegisterHandler(DBProtoArenaDefUpdate, *this, &Arena::UpdateDefInfo, new cs::BtlPlayerInfo);
	gMsgDispatcher.RegisterHandler(DBProtoArenaUpdateReplay, *this, &Arena::ReplayUpate, new cs::ArenaBltReplay);
	gMsgDispatcher.RegisterHandler(DBProtoArenaAddReplay, *this, &Arena::ReplayAdd, new cs::ArenaBltReplay, new cs::ArenaBltReplay);
	gMsgDispatcher.RegisterHandler(DBProtoArenaReplayList, *this, &Arena::GetReplayList, nullptr, new cs::ArenaBltReplayArr);
	gMsgDispatcher.RegisterHandler(DBProtoArenaWeekendAward, *this, &Arena::WeekendAward, new db::ArenaSendWeekendAward, new cs::CSBoolReq);
	gMsgDispatcher.RegisterHandler(DBProtoAreanSetScore, *this, &Arena::SetArenaScore, new db::ModifyArenaScore);
	gMsgDispatcher.RegisterHandler(DBProtoAreanGetScores, *this, &Arena::GetPlayerScore, new db::GetPlayerArenaScore, new cs::IntKVPairArr);
	gMsgDispatcher.RegisterHandler(DBProtoArenaGetRegionForWorship, *this, &Arena::GetRegionForWorship, nullptr, new db::Uint32Rsp);
	gMsgDispatcher.RegisterHandler(DBProtoArenaUpdateYesterdayRegionId, *this, &Arena::UpdateYesterdayRegionId, new cs::CSUint32Req);
	gMsgDispatcher.RegisterHandler(DBProtoArenaTop30, *this, &Arena::RecordTop30, new cs::CSUint32Req);

	gMsgDispatcher.RegisterHandler(DBProtoArenaGetSimpleInfo, *this, &Arena::GetSimpleInfo, nullptr, new cs::ArenaTarDetail);
	gMsgDispatcher.RegisterHandler(DBProtoArenaUpdateSimpleInfo, *this, &Arena::UpdateSimpleInfo, new cs::ArenaTarDetail);

	arenaReplayMaxCnt_ = 50;
}

void Arena::Init()
{
	
	ArenaDrop::Drops::Instance().Load();
	Test();
}

bool Arena::GetRegionKey(uint32_t index, std::string& key, int gameRegion)
{
	auto& vec = GetRegionKeyVec(gameRegion);

	if (index < vec.size()) {
		key = vec[index];
		return true;
	}
	return false;
}

ErrCodeType Arena::GetTarget(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::ArenaGetTar, req);
	REAL_PROTOBUF_MSG(outMsg, db::ArenaTarInfoArr, resp);
	if (IsLock(kKeyArenaLock, req.game_region())) {
		return ErrCodeType::ErrCodeArenaSystemBusy;
	}
	uint32_t tarId = h.TargetID;
	std::vector<uint32_t> tarVec;
	if (tarId == 0) {
		const auto& script = ScriptMgr::Instance().GetScriptStr("script/arena_get_target.lua", false);
		if (script != ScriptMgr::Instance().Empty()) {
			std::vector<std::string> keys;
			std::vector<std::string> args;

			auto& vec = GetRegionKeyVec(req.game_region());
			keys.assign(vec.begin(), vec.end());

			keys.push_back(kKeyArenaInfo);

			args.push_back(std::to_string(h.PlayerID));

			args.push_back(std::to_string(req.begin()));
			args.push_back(std::to_string(req.end()));
			args.push_back(std::to_string(req.cnt()));

			auto reply = gRedis->eval(script, &keys, &args);
			switch (reply->type)
			{
			case REDIS_REPLY_INTEGER:
				tarVec.push_back(reply->integer);
				break;
			case REDIS_REPLY_NIL:
				break;
			case REDIS_REPLY_ARRAY:
				for (size_t i = 0; i < reply->elements; ++i) {
					tarVec.push_back(reply->element[i]->integer);
				}
				break;
			case REDIS_REPLY_ERROR:
				WARN_LOG("Redis error: %s! plid=%u", reply->str, h.PlayerID);
				return ErrCodeDB;
			default:
				WARN_LOG("Redis error: unexpected reply type %d! plid=%u", reply->type, h.PlayerID);
				return ErrCodeDB;
			}
		}
	}
	else {
		tarVec.push_back(tarId);
	}
	if (tarVec.size() != 0) {
		for (auto id : tarVec) {
			std::stringstream ss;
			ss << kKeyArenaDefInfo << id;
			std::string data;
			auto info = resp.add_tars();
			if (gRedis->get(ss.str(), data)) {
				info->mutable_btl_info()->ParseFromString(data);
			}
			GetInfoImp(id, *info->mutable_score(), false, false, req.game_region());
			std::string pdKey(kKeyPrefixPlayerData + std::to_string(id));
			std::unordered_map<std::string, std::string> m;
			auto& strWear = m[kGarWearing];
			if (!gRedis->hget(pdKey, m)) {
				WARN_LOG("hget failed: %s! plid=%u", gRedis->last_error_message().c_str(), id);
				return ErrCodeType::ErrCodeDB;
			}
			info->mutable_btl_info()->mutable_wear()->ParseFromString(strWear);
		}
	}
	return ErrCodeType::ErrCodeSucc;
}

ErrCodeType Arena::UpdateInfo(const SSProtoHead & h, google::protobuf::Message * inMsg, google::protobuf::Message * outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::ArenaUpdateArr, req);
	const auto& script = ScriptMgr::Instance().GetScriptStr("script/arena_update.lua", false);
	if (script != ScriptMgr::Instance().Empty()) {
		for (auto it : req.data()) {
			DEBUG_LOG("Arena::UpdateInfo id:%u inc:%d", it.id(), it.modify());
			if (IsLock(kKeyArenaLock, it.game_region())) {
				continue;
			}
			auto& keyVec = GetRegionKeyVec(it.game_region());
			std::vector<std::string> keys;
			std::vector<std::string> args;
			keys.assign(keyVec.begin(), keyVec.end());
			keys.push_back(kKeyArenaInfo);
			
			args.push_back(std::to_string(it.id()));
			args.push_back(std::to_string(it.modify()));
			args.push_back(std::to_string(time(0)));

			auto reply = gRedis->eval(script, &keys, &args);
			CHECK_REPLY_EC(reply, REDIS_REPLY_INTEGER, h.PlayerID);
			ErrCodeType errCodeUpdateRank = UpdateArenaRank(it.id(), 0, it.game_region());
			if (errCodeUpdateRank) {
				return errCodeUpdateRank;
			}
		}
		return ErrCodeType::ErrCodeSucc;
	}
	else {
		return ErrCodeType::ErrCodeDB;
	}
}

ErrCodeType Arena::GetInfo(const SSProtoHead & h, google::protobuf::Message * inMsg, google::protobuf::Message * outMsg)
{
	//if (IsLock()) {
	//	return ErrCodeType::ErrCodeArenaSystemBusy;
	//}
	REAL_PROTOBUF_MSG(inMsg, cs::CSUint32Req, req);
	REAL_PROTOBUF_MSG(outMsg, cs::ArenaInfo, resp);
	return GetInfoImp(h.TargetID, resp, false, false, req.u32());
}

ErrCodeType Arena::GetInfoImp(uint32_t tarId, cs::ArenaInfo& info, bool noRank, bool noScore, int gameRegion)
{
	std::unordered_map<std::string, std::string> data;
	auto memberStr = std::to_string(tarId);
	std::string& strRegion = data[memberStr];
	if (!gRedis->hget(kKeyArenaInfo, data)) {
		return ErrCodeType::ErrCodeDB;
	}

	info.set_id(tarId);
	uint32_t region = 1;
	if(!strRegion.empty())
	{
		region = atoi(strRegion.c_str());
		if (region < 1) {
			region = 1;
		}
	}

	std::unordered_map<std::string, std::string> ldata;
	std::string& strLRegion = ldata[memberStr];
	if (!gRedis->hget(kKeyArenaInfoLast, ldata)) {
		return ErrCodeType::ErrCodeDB;
	}
	uint32_t lregion = 1;
	if (!strLRegion.empty())
	{
		lregion = atoi(strLRegion.c_str());
		if (lregion < 1) {
			lregion = 1;
		}
	}
	info.set_region(region);
	info.set_last_region(lregion);
	if (!noScore || !noRank) {
		std::string regionStr;
		GetRegionKey(info.region() - 1, regionStr, gameRegion);

		if (!noScore) {
			int32_t score = 0;
			gRedis->zscore(regionStr, memberStr, score);
			info.set_score(score);
		}
		if (!noRank) {
			long long rank = -1;
			if (!gRedis->zrevrank(regionStr, memberStr, &rank)) {
				uint32_t cnt = 0;
				gRedis->zcount(regionStr, 0, -1, cnt);
				rank = cnt;
			}
			info.set_rank(rank);
		}
	}
	return ErrCodeType::ErrCodeSucc;
}

ErrCodeType Arena::UpdateDefInfo(const SSProtoHead & h, google::protobuf::Message * inMsg, google::protobuf::Message * outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, cs::BtlPlayerInfo, req);
	std::stringstream ss;
	ss << kKeyArenaDefInfo << h.TargetID;
	std::string data;
	req.SerializeToString(&data);
	gRedis->set(ss.str(), data);
	return ErrCodeType::ErrCodeSucc;
}

ErrCodeType Arena::ReplayUpate(const SSProtoHead & h, google::protobuf::Message * inMsg, google::protobuf::Message * outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, cs::ArenaBltReplay, req);
	std::stringstream ss;
	ss << kKeyArenaReplay << h.TargetID;
	std::string key = ss.str();

	if (req.remove()) {
		gRedis->hdel(key, std::vector<std::string>{req.inner_id()});
		auto arr = Split(req.inner_id(), '_');
		if (arr.size() > 0) {
			auto uid = atoi(arr[0].c_str());
			gBtlReplay.RemoveReplay(uid, std::string("btl_replay_") + req.inner_id());
		}
	}
	else {
		std::unordered_map<std::string, std::string> data;
		req.SerializeToString(&data[req.inner_id()]);
		gRedis->hset(key, data);
	}
	return ErrCodeType::ErrCodeSucc;
}

ErrCodeType Arena::ReplayAdd(const SSProtoHead & h, google::protobuf::Message * inMsg, google::protobuf::Message * outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, cs::ArenaBltReplay, req);
	REAL_PROTOBUF_MSG(outMsg, cs::ArenaBltReplay, resp);
	std::stringstream ss;
	//防守
	if (req.type() == 1) {
		ss << kKeyArenaDefTarFailCnt << h.TargetID;
		auto key = ss.str();

		std::string strTarId = std::to_string(req.tar_id());
		// 防守成功
		if (req.is_win()) {
			std::unordered_map<std::string, std::string> data;
			auto& strFailCnt = data[strTarId];
			if (!gRedis->hget(key, data)) {
				return ErrCodeType::ErrCodeDB;
			}
			uint32_t failCnt = 0;
			//对方挑战连续失败3次可以嘲讽
			if (!strFailCnt.empty()) {
				failCnt = atoi(strFailCnt.c_str());
			}
			failCnt += 1;
			if (failCnt >= 3) {
				req.set_sneer_state(1);
			}

			data[strTarId] = std::to_string(failCnt);
			gRedis->hset(key, data);
		}
		else {
			std::vector<std::string> data = { strTarId };
			gRedis->hdel(key, data);
		}
	}
	//else {
	//	req.set_sneer_state(1);
	//}

	resp.CopyFrom(req);

	ss.str("");
	ss << kKeyArenaReplay << h.TargetID;
	std::string key = ss.str();

	std::unordered_map<std::string, std::string> data;
	req.SerializeToString(&data[req.inner_id()]);
	gRedis->hset(key, data);
	ss.str("");
	ss << kKeyArenaReplayList << h.TargetID;
	std::vector<std::string> list;
	list.push_back(req.inner_id());
	gRedis->lpush(ss.str(), list);

	RemoveRangeOutReplay(h.TargetID);

	return ErrCodeType::ErrCodeSucc;
}

ErrCodeType Arena::GetReplayList(const SSProtoHead & h, google::protobuf::Message * inMsg, google::protobuf::Message * outMsg)
{
	REAL_PROTOBUF_MSG(outMsg, cs::ArenaBltReplayArr, arr);
	std::stringstream ss;
	ss << kKeyArenaReplay << h.TargetID;
	std::string key = ss.str();

	std::unordered_map<std::string, std::string> data;
	gRedis->hgetall(key, data);

	for (auto it : data) {
		auto rep = arr.add_replays();
		if (!rep->ParseFromString(it.second)) {
			ERROR_LOG("Parse Arena Replay Info Failed uid:%u", h.TargetID);
		}
		if (arr.replays_size() >= arenaReplayMaxCnt_) {
			break;
		}
	}
	return ErrCodeType::ErrCodeSucc;
}

ErrCodeType Arena::WeekendAward(const SSProtoHead & h, google::protobuf::Message * inMsg, google::protobuf::Message * outMsg)
{
	DEBUG_LOG("Arena::WeekendAward");
	REAL_PROTOBUF_MSG(inMsg, db::ArenaSendWeekendAward, req);
	REAL_PROTOBUF_MSG(outMsg, cs::CSBoolReq, resp);
	int gameRegion = req.game_region();
	resp.set_is_true(false);
	if (!TryLock(kKeyArenaWeekendLock, gameRegion)) {
		DEBUG_LOG("TryLock Failed");
		return ErrCodeType::ErrCodeSucc;
	}
	do {
		time_t now = time(NULL);
		tm tmNow;
		localtime_r(&now, &tmNow);
		int dd = (tmNow.tm_wday - 1) % 7;
		if (dd < 0) {
			dd += 7;
		}
		time_t curWeekend = ant::today_begin_time(now) - dd * 24 * 3600;

		std::stringstream ss;
		ss << kKeyArenaLastWeekendTime << "_" << gameRegion;
		auto keyLastTime = ss.str();
		//不是来自于GM指令
		if (!req.is_gm()) {
			bool exists = false;
			std::string strLastWeekend;

			gRedis->get(keyLastTime, strLastWeekend, &exists);
			if (exists) {
				time_t lastTime = atoi(strLastWeekend.c_str());
				if (DayDiff(curWeekend, lastTime) < 7) {
					DEBUG_LOG("Award was send at:%ld cur:%ld now:%ld", lastTime, curWeekend, now);
					break;
				}
			}
		} else {
			db::RankKey rankWorldWeekObj;
			rankWorldWeekObj.set_rank_type(RankData::kWeek);
			rankWorldWeekObj.set_rank_deadtm(NextMonday());
			std::string rkey = kRankFieldArenaRank + "_" + std::to_string(gameRegion);
			rankWorldWeekObj.set_rank_name(rkey);
			std::vector<std::string> keysToDel = { RankData::MakeRankKey(rankWorldWeekObj) };
			gRedisRank->del(keysToDel);
		}

		gRedis->set(keyLastTime, std::to_string(curWeekend));
		while (!TryLock(kKeyArenaLock, gameRegion)) {
			usleep(10 * 1000);
		}

		std::unordered_map<std::string, std::string> fields;

		gRedis->del(std::vector<std::string>{kKeyArenaInfoLast});
		/*if (!gRedis->rename(kKeyArenaInfo, kKeyArenaInfoLast)) {
			ERROR_LOG("%s", gRedis->last_error_cstr());
		}*/
		uint32_t cursor = 0;
		while (true) {
			fields.clear();
			gRedis->hscan(kKeyArenaInfo, cursor, fields);
			gRedis->hset(kKeyArenaInfoLast, fields);
			if (cursor == 0) {
				break;
			}
		}
		//top100
		std::set<uint32_t> top100;
		SendWeeklyRankAward(top100, gameRegion);

		gRedis->del(std::vector<std::string>{kKeyArenaInfo});

		cursor = 0;
		while (true) {
			fields.clear();
			gRedis->hscan(kKeyArenaInfoLast, cursor, fields);
			//std::vector<std::string> delList;
			std::unordered_map<std::string, std::string> modify;
			for (auto it : fields) {
				uint32_t region = atoi(it.second.c_str());
				if (region == 1) {
					//delList.emplace_back(it.first);
				}
				else {
					region -= 1;
					modify[it.first] = std::to_string(region);
				}
			}
			/*if (delList.size() != 0) {
				gRedis->hdel(kKeyArenaInfo, delList);
			}*/
			if (modify.size() != 0) {
				gRedis->hset(kKeyArenaInfo, modify);
				for(auto& v : modify) {
					gCapResData.AddToResDis(gameRegion, atoi(v.first.c_str()));
				}  
			}

			if (cursor == 0) {
				break;
			}
		}
		uint32_t rCnt = sizeof(sRegionScore) / sizeof(uint32_t);

		for (uint32_t i = 0; i < rCnt; ++i) {
			std::string rKey, nKey;
			GetRegionKey(i, rKey, gameRegion);
			if (i > 0) {
				GetRegionKey(i - 1, nKey, gameRegion);
			}
			cursor = 0;
			time_t stral = now;
			while (true)
			{
				std::vector<std::string> result;
				std::vector<std::string> delList;
				//zscan返回是升序
				gRedis->zscan(rKey, cursor, result);
				for (uint32_t j = 0; j < result.size() / 2; ++j) {
					auto& k = result[j * 2];
					//auto& s = result[i * 2 + 1];
					if (i == 0) {
						//0分不发奖励
						int rs = atoi(result[j * 2 + 1].c_str());
						if (rs == 0) {
							continue;
						}
					}
					uint32_t score = 0;
					if (i > 1) {
						score = (sRegionScore[i - 1] + sRegionScore[i - 2]) / 2;
					}
					//DEBUG_LOG("Arena Id:%s, score%s", k.c_str(), result[j * 2 + 1].c_str());
					--stral;
					if (i > 0) {
						// to_string 的精度只有6位小数，因此只保留尾数
						double dscore = (double)score + double((2000000000 - stral) % 1000000) / 1000000;
						gRedis->zadd(nKey, dscore, k);
						cs::GeoPos geo;
						ErrCodeType errCode = GeoData::GetGeoData(atoi(k.c_str()), geo);
						if (!errCode) {
							geo.set_game_region(gameRegion);
							//double orgScore = 0;
							//gRedis->zscore(rKey, k, orgScore);
							//double decScore = orgScore - score;
							//double scoreTail = decScore / 100000.0;
							//double newScore = score + scoreTail;
							//RankData::DoSetGeoRank(k, newScore, newScore, 0, geo, kRankFieldArenaRank, false, true, false);
							RankData::DoSetGeoRank(k, dscore, dscore, 0, geo, kRankFieldArenaRank, false, true, false);
						}
					}
					if (top100.find(atoi(k.c_str())) == top100.end()) {
						std::unordered_map<std::string, std::string> info;
						auto& val = info[kKeyArenaWeekendAward];
						ss.str("");
						ss << kKeyPlayerTemp << k;
						auto keyPlayerTmp = ss.str();
						gRedis->hget(keyPlayerTmp, info);
						ss.str("");
						ss << now << "|" << (i + 1) << ",";
						val.append(ss.str());
						gRedis->hset(keyPlayerTmp, info);
					}
				}
				if (cursor == 0) {
					break;
				}
			}
			gRedis->del(std::vector<std::string>{rKey});
		}
		resp.set_is_true(true);
		Unlock(kKeyArenaLock, gameRegion);
	} while (0);
	Unlock(kKeyArenaWeekendLock, gameRegion);
	return ErrCodeType::ErrCodeSucc;
}

ErrCodeType Arena::GetPlayerScore(const SSProtoHead & h, google::protobuf::Message * inMsg, google::protobuf::Message * outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::GetPlayerArenaScore, req);
	REAL_PROTOBUF_MSG(outMsg, cs::IntKVPairArr, resp);
	//DEBUG_LOG("pid:%u cnt:%u", h.PlayerID, req.u32_size());
	for (auto id : req.pids()) {
		int regionId;
		std::unordered_map<std::string, std::string> fields;
		std::string strId = std::to_string(id);
		std::string& strRegion = fields[strId];
		gRedis->hget(kKeyArenaInfo, fields);
		regionId = atoi(strRegion.c_str());
		if (regionId <= 0) {
			regionId = 1;
		}
		else {
			if (regionId > 8) {
				regionId = 8;
			}
		}
		int score;
		std::string regionKey;
		GetRegionKey(regionId - 1, regionKey, req.game_region());
		gRedis->zscore(regionKey, strId, score);
		auto item = resp.add_data();
		item->set_k(id);
		item->set_v(score);
	}
	return ErrCodeType::ErrCodeSucc;
}

ErrCodeType Arena::SetArenaScore(const SSProtoHead & h, google::protobuf::Message * inMsg, google::protobuf::Message * outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::ModifyArenaScore, req);
	
	const auto& script = ScriptMgr::Instance().GetScriptStr("script/arena_update.lua", false);
	if (script != ScriptMgr::Instance().Empty()) {
		std::vector<std::string> keys;
		std::vector<std::string> args;
		if (IsLock(kKeyArenaLock, req.game_region())) {
			return ErrCodeType::ErrCodeArenaSystemBusy;
		}
		auto& keyVec = GetRegionKeyVec(req.game_region());
		keys.assign(keyVec.begin(), keyVec.end());
		keys.push_back(kKeyArenaInfo);

		args.push_back(std::to_string(h.TargetID));
		args.push_back(std::to_string(req.val()));
		args.push_back(std::to_string(time(0)));
		args.push_back(std::to_string(req.inc() ? 0 : 1));

		auto reply = gRedis->eval(script, &keys, &args);
		CHECK_REPLY_EC(reply, REDIS_REPLY_INTEGER, h.TargetID);
		ErrCodeType errCodeUpdateRank = UpdateArenaRank(h.TargetID, 0, req.game_region());
		if (errCodeUpdateRank) {
			return errCodeUpdateRank;
		}

		gCapResData.AddToResDis(req.game_region(), h.TargetID);
		return ErrCodeType::ErrCodeSucc;
	}
	else {
		return ErrCodeType::ErrCodeDB;
	}
}

ErrCodeType Arena::GetRegionForWorship(const SSProtoHead & h, google::protobuf::Message * inMsg, google::protobuf::Message * outMsg)
{
	std::string strPlayerID = std::to_string(h.PlayerID);

	std::unordered_map<std::string, std::string> m;
	auto& strRegion = m[strPlayerID];
	if (!gRedis->hget(kKeyArenaInfo, m)) {
		WARN_LOG("hget failed: %s! plid=%u", gRedis->last_error_cstr(), h.PlayerID);
		return ErrCodeDB;
	}
	uint32_t region = 1;
	if (!strRegion.empty()) {
		region = atoi(strRegion.c_str());
	}
	REAL_PROTOBUF_MSG(outMsg, db::Uint32Rsp, rsp);
	rsp.set_u32(region);
	return ErrCodeSucc;
}


ErrCodeType Arena::UpdateYesterdayRegionId(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, cs::CSUint32Req, req);
	int gameRegion = req.u32();
	DEBUG_LOG("UpdateYesterdayRegionId GameRegion:%d", gameRegion);
	if (!TryLock(kKeyArenaYesterdayLock, gameRegion)) {
		ERROR_LOG("UpdateYesterdayRegionId local lock Failed");
		return ErrCodeSucc;
	}

	do {
		time_t now = time(NULL);
		bool exists = false;
		std::string strLastWeekend;
		gRedis->get(kKeyArenaUpdateYesterdayTime, strLastWeekend, &exists);
		if (exists) {
			time_t lastTime = atoi(strLastWeekend.c_str());
			if (DayDiff(now, lastTime) < 1) {
				DEBUG_LOG("Award yesterday region was update at:%ld now:%ld", lastTime, now);
				break;
			}
		}
		gRedis->set(kKeyArenaUpdateYesterdayTime, std::to_string(now));

		uint32_t waitCnt = 0;
		bool lockFailed = false;
		while (!TryLock(kKeyArenaLock, gameRegion)) {
			usleep(1000 * 1000);
			++waitCnt;
			if (waitCnt >= 60) {
				lockFailed = true;
				break;
			}
		}
		if (lockFailed) {
			ERROR_LOG("UpdateYesterdayRegionId lock Failed");
			break;
		}
		std::unordered_map<std::string, std::string> fields;
		gRedis->del(std::vector<std::string>{kKeyArenaInfoYesterday});
		uint32_t cursor = 0;
		while (true) {
			fields.clear();
			gRedis->hscan(kKeyArenaInfo, cursor, fields);
			gRedis->hset(kKeyArenaInfoYesterday, fields);
			if (cursor == 0) {
				break;
			}
		}
		std::stringstream ss;
		
		for (uint32_t i = 0; i < sizeof(sRegionScore)/sizeof(uint32_t); ++i) {
			ss.str("");
			ss << kKeyArenaRegonYestorday << "_" << gameRegion << "_" << i;
			std::string yestKey = ss.str();
			std::string key;
			GetRegionKey(i, key, gameRegion);
			gRedis->del(std::vector<std::string>{yestKey});
			while (true) {
				std::vector<std::string> result;
				gRedis->zscan(key, cursor, result);
				for (uint32_t i = 0; i < result.size() / 2; ++i) {
					auto tmp = result[i * 2];
					result[i * 2] = result[i * 2 + 1];
					result[i * 2 + 1] = tmp;
				}
				gRedis->zadd(yestKey, result);
				if (cursor == 0) {
					break;
				}
			}
		}
		Unlock(kKeyArenaLock, gameRegion);
	} while (0);
	Unlock(kKeyArenaYesterdayLock, gameRegion);
	return ErrCodeSucc;
}


ErrCodeType Arena::GetSimpleInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(outMsg, cs::ArenaTarDetail, info);
	std::stringstream ss;
	ss << kKeyArenaSimpleInfo << h.TargetID;
	std::string data;
	gRedis->get(ss.str(), data);
	info.ParseFromString(data);	
	return ErrCodeSucc;
}

ErrCodeType Arena::UpdateSimpleInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, cs::ArenaTarDetail, info);
	std::string data;
	info.SerializeToString(&data);
	std::stringstream ss;
	ss << kKeyArenaSimpleInfo << h.TargetID;
	gRedis->set(ss.str(), data);
	return ErrCodeSucc;
}

static const char dbName[] = "global";
static const char tblName[] = "arena_top_30";

ErrCodeType Arena::RecordTop30(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, cs::CSUint32Req, req);
	int gameRegion = req.u32();

	auto sqlcon = MysqlProxy::Instance().GetDefaultSqlCon();
	if (!sqlcon) {
		return ErrCodeSucc;
	}
	if (!TryLock(kKeyArenaTop30Flag, gameRegion)) {
		return ErrCodeSucc;
	}
	const int maxAwardCnt = 30;
	
	uint32_t rCnt = sizeof(sRegionScore) / sizeof(uint32_t);
	int leftCnt = maxAwardCnt;
	std::unordered_map<uint32_t, std::string>	rankMap;
	for (uint32_t i = rCnt; i > 0; --i) {
		std::string rKey;
		GetRegionKey(i - 1, rKey, gameRegion);
		std::vector<std::string> result;
		gRedis->zrevrange(rKey, 0, leftCnt - 1, result);
		uint32_t idx = 0;
		DEBUG_LOG("region: %u, cnt: %zu, lcnt:%d", i, result.size(), leftCnt);
		for (auto& id : result) {
			int rank = maxAwardCnt - leftCnt + idx + 1;
			++idx;
			rankMap[rank] = id;
		}
		leftCnt -= result.size();
		if (leftCnt <= 0) {
			break;
		}
	}
	std::stringstream ss;
	ss << "insert into " << dbName << "." << tblName << " values ";
	bool isFirst = true;
	uint32_t now = time(0);

	for (auto it : rankMap) {
		std::unordered_map<std::string, std::string> data;
		db::RegInfo regInfo;
		auto& info = data["reg_info"];
		auto key = kKeyPrefixPlayerData + it.second;
		gRedis->hget(key, data);
		regInfo.ParseFromString(info);
		auto& curAcc = regInfo.account();
		if (!isFirst) {
			ss << ",";
		}
		else {
			isFirst = false;
		}
		ss << "('" << curAcc.account_type() << "','" << curAcc.account_id() << "','" << it.second << "',"
			<< it.first << ", CURRENT_DATE, " << gameRegion << ")";
		//发送通知邮件
		db::AddMailReq mailReq;
		mailReq.set_player_id(atoi(it.second.c_str()));
		auto m = mailReq.add_mails();
		m->set_mail_type(1002);
		m->set_send_time(now);
		m->add_args(std::to_string(it.first).c_str());
		gMailData.AddMail(mailReq);
	}
	sqlcon->execute(ss.str());
	DEBUG_LOG("RecordTop30 Sql:%s", ss.str().c_str());
	return ErrCodeSucc;
}

void Arena::addArenaTop30Award(const cs::Account& accInfo, uint32_t uid)
{
	DEBUG_LOG("addArenaTop30Award uid:%u", uid);
	int gameRegion = PlayerData::GetGameRegion(uid);
	if (accInfo.account_id().empty()) {
		return;
	}
	auto sqlCon = MysqlProxy::Instance().GetDefaultSqlCon();
	if (!sqlCon) {
		return;
	}
	char sql[1024];
	sprintf(sql, "select rank from %s.%s where acc_type = '%s' and acc_id = '%s' and `game_region` = %d order by rank asc limit 1"
		, dbName, tblName, accInfo.account_type().c_str(), accInfo.account_id().c_str(), gameRegion);
	auto rep = sqlCon->store(sql);
	if (rep.num_rows() == 0) {
		return;
	}
	int rank = atoi(rep[0][0].data());

	const CSVArenaTop30Award::Item* dict = nullptr;
	for (auto it : gCSVArenaTop30Award.AllItems) {
		auto info = it.second;
		if (rank >= info->rankBegin && rank < info->rankEnd) {
			dict = info;
			break;
		}
	}
	if (!dict) {
		return;
	}

	uint32_t now = time(0);
	db::AddMailReq req;
	req.set_player_id(uid);
	auto m = req.add_mails();
	m->set_mail_type(1003);
	m->set_send_time(now);
	m->add_args(std::to_string(rank).c_str());

	for (auto& it : dict->award) {
		auto arr = SplitAndToIntVec(it, '#');
		if (arr.size() < 3) {
			continue;
		}
		MailAddAttach(m->mutable_attach(), arr);
	}
	gMailData.AddMail(req);
}

void Arena::TrySendWeekendAwardMail(uint32_t uid)
{
	int gameRegion = PlayerData::GetGameRegion(uid);
	if (IsLock(kKeyArenaLock, gameRegion)) {
		return;
	}
	std::stringstream ss;
	ss << kKeyPlayerTemp << uid;
	auto keyPlayerTmp = ss.str();
	std::unordered_map<std::string, std::string> info;
	auto& wAward = info[kKeyArenaWeekendAward];
	auto& rankAward = info[kKeyArenaWeekendRankAward];
	gRedis->hget(keyPlayerTmp, info);
	auto wMailInfo = Split(wAward, ',');
	auto rMailInfo = Split(rankAward, ',');
	wAward.clear();
	rankAward.clear();
	gRedis->hset(keyPlayerTmp, info);
	db::AddMailReq req;
	req.set_player_id(uid);
	for (auto& mi : wMailInfo) {
		if (mi.empty()) {
			continue;
		}
		auto data = Split(mi, '|');
		if (data.size() != 2) {
			continue;
		}
		auto time = atoi(data[0].c_str());
		auto region = atoi(data[1].c_str());
		auto dict = gCSVArenaWeeklyBonus.GetItem(region);
		if (!dict) {
			continue;
		}
		auto m = req.add_mails();
		m->set_mail_type(903);
		m->set_send_time(time);

		auto rDict = gCSVArenaRegionScore.GetItem(region);
		if (rDict) {
			m->add_args(rDict->name);
		}
		else {
			m->add_args("No Name");
		}

		std::unordered_map<uint64_t, ArenaDrop::Drops::Item> output;
		for (auto id : dict->DailiBonus) {
			ArenaDrop::Drops::Instance().GetRewardDrop(id, output);
		}
		for (auto it : output) {
			auto& item = it.second;
			item.AttachToMail(m);
		}
	}

	for (auto& mi : rMailInfo) {
		if (mi.empty()) {
			continue;
		}
		auto data = Split(mi, '|');
		if (data.size() != 2) {
			continue;
		}
		auto time = atoi(data[0].c_str());
		auto rank = atoi(data[1].c_str());
		const CSVArenaWeeklyRankBonus::Item* dict = nullptr;
		for (auto it : gCSVArenaWeeklyRankBonus.AllItems) {
			dict = it.second;
			if (dict->begin <= rank && rank <= dict->end) {
				break;
			}
		}
		if (!dict) {
			continue;
		}
		auto m = req.add_mails();
		m->add_args(std::to_string(rank));
		m->set_mail_type(904);
		m->set_send_time(time);

		std::unordered_map<uint64_t, ArenaDrop::Drops::Item> output;
		for (auto id : dict->DailiBonus) {
			ArenaDrop::Drops::Instance().GetRewardDrop(id, output);
		}
		for (auto it : output) {
			auto& item = it.second;
			item.AttachToMail(m);
		}
	}
	gMailData.AddMail(req);
}


void Arena::SendWeeklyRankAward(std::set<uint32_t>& vec, int gameRegion)
{
	const int maxAwardCnt = 100;
	std::stringstream ss;

	uint32_t rCnt = sizeof(sRegionScore) / sizeof(uint32_t);
	time_t now = time(nullptr);
	int leftCnt = maxAwardCnt;
	for (uint32_t i = rCnt; i > 0; --i) {
		std::string rKey;
		GetRegionKey(i - 1, rKey, gameRegion);
		std::vector<std::string> result;
		gRedis->zrevrange(rKey, 0, leftCnt - 1, result);
		int idxBegin = maxAwardCnt - leftCnt;
		for (auto& id : result) {
			std::unordered_map<std::string, std::string> info;
			auto& val = info[kKeyArenaWeekendRankAward];
			ss.str("");
			ss << kKeyPlayerTemp << id;
			auto keyPlayerTmp = ss.str();
			gRedis->hget(keyPlayerTmp, info);
			ss.str("");
			ss << now << "|" << ++idxBegin << ",";
			val.append(ss.str());
			gRedis->hset(keyPlayerTmp, info);
			vec.insert(atoi(id.c_str()));
		}
		leftCnt -= result.size();
		if (leftCnt <= 0) {
			break;
		}
	}
}


uint32_t Arena::GetYesterdayRegionId(uint32_t uid)
{
	uint32_t region = 1;
	std::unordered_map<std::string, std::string> data;
	auto memberStr = std::to_string(uid);
	do 
	{
		std::string& strRegion = data[memberStr];
		if (gRedis->hget(kKeyArenaInfoYesterday, data)) {
			if (!strRegion.empty()) {
				region = atoi(strRegion.c_str());
				break;
			}
		}
		if (gRedis->hget(kKeyArenaInfo, data)) {
			if (!strRegion.empty()) {
				region = atoi(strRegion.c_str());
				break;
			}
		}
	} while (0);
	if (region < 1) {
		region = 1;
	}
	return region;
}

bool Arena::IsLock(const std::string& key, int gameRegion)
{
	DEBUG_LOG("Arena::IsLock");
	auto rk = key + "_" + std::to_string(gameRegion);
	auto& script = ScriptMgr::Instance().GetScriptStr("script/arena_is_lock.lua", false);
	if (script != ScriptMgr::Instance().Empty()) {
		std::vector<std::string> keys{ rk };
		auto replay = gRedis->eval(script, &keys);
		if (replay->type != REDIS_REPLY_INTEGER) {
			return false;
		}
		else {
			return replay->integer == 1;
		}
	}
	else {
		return false;
	}
}

bool Arena::TryLock(const std::string& key, int gameRegion)
{
	DEBUG_LOG("Arena::TryLock");
	auto rk = key + "_" + std::to_string(gameRegion);
	auto& script = ScriptMgr::Instance().GetScriptStr("script/arena_try_lock.lua", false);
	if (script != ScriptMgr::Instance().Empty()) {
		std::vector<std::string> keys{ rk };
		auto replay = gRedis->eval(script, &keys);
		if (replay->type != REDIS_REPLY_INTEGER) {
			return false;
		}
		else {
			return replay->integer == 1;
		}
	}
	else {
		return false;
	}
}

void Arena::Unlock(const std::string& key, int gameRegion)
{
	DEBUG_LOG("Arena::Unlock");
	auto rk = key + "_" + std::to_string(gameRegion);
	gRedis->set(rk, std::to_string(0));
}

void Arena::RemoveRangeOutReplay(uint32_t uid)
{
	std::stringstream ss;
	ss << kKeyArenaBtlReplayLock << uid;
	auto lockName = ss.str();
	// 避免其他进程冲突
	while (!TryLock(lockName, 0)) {
		usleep(100 * 1000);
	}

	do {
		ss.str("");
		ss << kKeyArenaReplayList << uid;
		auto lkey = ss.str();
		long long cnt;
		gRedis->llen(lkey, cnt);
		if (cnt <= arenaReplayMaxCnt_) {
			break;
		}
		std::vector<std::string> dels;
		gRedis->lrange(lkey, arenaReplayMaxCnt_, cnt, dels);
		gRedis->ltrim(lkey, 0, arenaReplayMaxCnt_ - 1);

		if (dels.size() != 0) {
			ss.str("");
			ss << kKeyArenaReplay << uid;
			auto hkey = ss.str();
			gRedis->hdel(hkey, dels);
		}
		for (auto& rep : dels) {
			auto arr = Split(rep, '_');
			if (arr.size() > 0) {
				auto ruid = atoi(arr[0].c_str());
				gBtlReplay.RemoveReplay(ruid, std::string("btl_replay_") + rep);
			}
		}
	} while (0);
	Unlock(lockName, 0);
}

const std::vector<std::string>& Arena::GetRegionKeyVec(int gameRegion)
{
	auto& vec = keyByGameRegion_[gameRegion];
	if (vec.size() == 0) {
		char buff[1024];
		for (uint32_t i = 0; i < 8; ++i) {
			sprintf(buff, "%s_%d_%u", kKeyArenaRegon.c_str(), gameRegion, i + 1);
			vec.emplace_back(buff);
		}
	}
	return vec;
}

void Arena::Test()
{
	DEBUG_LOG("Arena::Test");
	ScriptMgr::Instance().Visit([](const std::string& k, const std::string& v) {
		DEBUG_LOG("%s", k.c_str());
	});
}

ErrCodeType Arena::UpdateArenaRank(uint32_t playerID, int32_t score, int gameRegion)
{
	std::string strPlayerID = std::to_string(playerID);

	std::unordered_map<std::string, std::string> m;
	auto& strRegion = m[strPlayerID];
	if (!gRedis->hget(kKeyArenaInfo, m)) {
		WARN_LOG("hget failed: %s! plid=%s", gRedis->last_error_cstr(), strPlayerID.c_str());
		return ErrCodeDB;
	}
	std::string rankArenaRegion;
	uint32_t region = 0;
	if (!strRegion.empty()) {
		region = atoi(strRegion.c_str()) - 1;
	}
	if (!GetRegionKey(region, rankArenaRegion, gameRegion)) {
		WARN_LOG("invalid arena region index %s, plid=%u", strRegion.c_str(), playerID);
		return ErrCodeDB;
	}
	double orgScore = 0;
	if (!gRedisRank->zscore(rankArenaRegion, strPlayerID, orgScore)) {
		WARN_LOG("zscore failed: %s! plid=%s", gRedis->last_error_cstr(), strPlayerID.c_str());
		return ErrCodeDB;
	}
	//double timeTail = GetTimeTail();
	//double newScore = orgScore + score > 0 ? (double)(orgScore + score) + timeTail : 0;
	// 取玩家地理位位置
	cs::GeoPos geo;
	ErrCodeType errCode = GeoData::GetGeoData(playerID, geo);
	if (errCode) {
		DEBUG_LOG("get geo data failed! plid=%u", playerID);
		return errCode;
	}
	geo.set_game_region(gameRegion);
	// 设置排行榜
	if (orgScore) {
		//DEBUG_LOG("Update Arena Rank Score Id:%u score%lf", playerID, orgScore);
		RankData::DoSetGeoRank(strPlayerID, orgScore, orgScore, 0, geo, kRankFieldArenaRank, false, true, false);
	} else {
		RankData::DoDelGeoRank(strPlayerID, geo, kRankFieldArenaRank, false, true, false);
	}
	// 不判断是否降榜，直接从上下两级的榜中删掉
	// std::string delRankArenaRegion;
	// if (GetRegionKey(atoi(strRegion.c_str()) - 1, delRankArenaRegion)) {
	// 	RankData::DoDelGeoRank(strPlayerID, geo, delRankArenaRegion);
	// }
	// if (GetRegionKey(atoi(strRegion.c_str()) + 1, delRankArenaRegion)) {
	// 	RankData::DoDelGeoRank(strPlayerID, geo, delRankArenaRegion);
	// }
	return ErrCodeSucc;
}

int Arena::GetPlayerScore(uint32_t uid, int gameRegion)
{
	int regionId;
	std::unordered_map<std::string, std::string> fields;
	std::string strId = std::to_string(uid);
	std::string& strRegion = fields[strId];
	gRedis->hget(kKeyArenaInfo, fields);
	regionId = atoi(strRegion.c_str());
	if (regionId <= 0) {
		regionId = 1;
	} else {
		if (regionId > 8) {
			regionId = 8;
		}
	}

	int score = 0;
	std::string regionKey;
	GetRegionKey(regionId - 1, regionKey, gameRegion);
	gRedis->zscore(regionKey, strId, score);
	return score;
}
