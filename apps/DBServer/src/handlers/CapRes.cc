/*===============================================================
* @Author: car
* @Created Time : 2018年05月23日 星期三 20时31分20秒
*
* @File Name: CapRes.cc
* @Description:
*
================================================================*/
#include <iomanip>
#include <sstream>
#include <libant/hash/hash_algo.h>
#include <libant/utils/StringUtils.h>
#include <google/protobuf/util/json_util.h>
#include <libant/time/time_utils.h>
#include "../CSV/CSVNpcRes.h"
#include "../CSV/CSVResInfo.h"
#include "../CSV/CSVResOption.h"
#include "../CSV/CSVMonster.h"
#include "../CSV/CSVModuleUnlock.h"
#include "../CSV/CSVDailyMonsterGift.h"
#include "KeyPrefixDef.h"
#include "../TimeUtils.h"
#include "../proto/SvrProtoID.pb.h"
#include "../proto/db.pb.h"
#include "../core/dispatcher.h"
#include "../core/redis_client.h"
#include "ProbGen.h"
#include "../core/ScriptMgr.h" 
#include "HatchData.h"

#include "Arena.h"
#include "CapRes.h"
#include "HomeRes.h"


using namespace std;

CapResData gCapResData;
CapResRegionConfig gCapResRegionConfig;

CapResData::CapResData()
{
	// 资源抢占
	gMsgDispatcher.RegisterHandler(DBProtoUpdatePResCapInfo, *this, &CapResData::updateCapResInfo, new db::PlayerResCapInfo);
	gMsgDispatcher.RegisterHandler(DBProtoCheckPResCapInfo, *this, &CapResData::checkCapResInfo, new db::CheckResPointInfoReq, new db::CheckResPointInfoRsp);
	gMsgDispatcher.RegisterHandler(DBProtoGetPResCapInfo, *this, &CapResData::scanCapResInfo, new db::Uint32Req, new db::PlayerResCapInfo);
	gMsgDispatcher.RegisterHandler(DBProtoUpdatePResStatus, *this, &CapResData::updateCapResStatus, new cs::OfflineInfo);
	gMsgDispatcher.RegisterHandler(DBProtoFreeCapRes, *this, &CapResData::freeCapRes, new db::Uint32Req);

	gMsgDispatcher.RegisterHandler(DBProtoPendantInfo, *this,  &CapResData::getPendantInfo, new db::Uint32Req, new db::PendantInfo);
}


ErrCodeType CapResData::updateCapResInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::PlayerResCapInfo, req);
	std::unordered_map<std::string, std::string> capRes;
	uint32_t region = gArena.GetYesterdayRegionId(h.TargetID);
	uint32_t now = time(0);
	bool isProtect = false;
	if (now < req.binfo(0).ptime()) {
		isProtect = true;
	}

	for (int  i=0; i<req.res_size(); i++) {
			auto& iter = capRes[to_string(req.res(i).guid())];
			auto& res = *(req.mutable_res(i));
			registerRes(req.game_region(), region, h.TargetID, isProtect, res);
			req.res(i).SerializeToString(&iter);
	}

	req.binfo(0).SerializeToString(&capRes[kKeyCapResBasicInfo]);
	req.last().SerializeToString(&capRes[kKeyCapResLastStoreInfo]);

	std::string resKey = makeCapResKey(to_string(h.TargetID));
	if (!gRedis->hset(resKey, capRes)) {
		WARN_LOG("hset failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}

	if (isProtect) {
		checkPlayerResProtect(req.game_region(), region, h.TargetID);
	}

	return ErrCodeSucc;	
}

ErrCodeType CapResData::checkCapResInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::CheckResPointInfoReq, req);
	REAL_PROTOBUF_MSG(outMsg, db::CheckResPointInfoRsp, rsp);
	std::string resKey;
	if (req.is_ai()) {
		// 新版本 ai是独立到个人必然是可以打的
		return ErrCodeSucc;
	} else {
		resKey = makeCapResKey(to_string(h.TargetID));
	}
	
	std::unordered_map<std::string, std::string> capRes;
	auto& buff = capRes[to_string(req.res_index())];
	auto& buff2 = capRes[kKeyCapResBasicInfo];

	if (!gRedis->hget(resKey, capRes)) {
		WARN_LOG("hget failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}

	uint32_t ret = 0;
	time_t now = time(0);
	
	db::PlayerResCapBasicInfo binfo;
	binfo.ParseFromString(buff2);
	uint32_t pt = binfo.ptime();
	if (now < pt) {
		ret = ErrCodeCapResInProtect;
	} else {
		auto& res = *(rsp.mutable_res());
		res.ParseFromString(buff);
		if (res.ftime() + 1800 < now) {
			res.set_is_fight(false);
		}

		if (!res.res_id()) {
			ret = ErrCodeWrongCapResId;
		} else if (res.is_fight()) {
			ret = ErrCodeCapResInFight;
		} else if (res.lose_time()) {
			ret = ErrCodeWrongCapResId;
		} else {
			//auto item = gCSVResInfo.GetItem(res.res_id());
			//DEBUG_LOG("item %u %ld",res.occ_time(), now);
			//if (!item || (!req.is_ai() && (res.occ_time() + item->ProTime) < now)) {
			//	ret = ErrCodeResProFinish;
			//} else {
			capRes.clear();
			res.set_is_fight(true);
			res.set_ftime(now);
			res.SerializeToString(&capRes[to_string(req.res_index())]);
			if (!gRedis->hset(resKey, capRes)) {
				WARN_LOG("hset failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
				return ErrCodeDB;
			}
			if (req.is_ai()) {
				res.set_occ_time(now);
			}
			//}
		}
	}
	rsp.set_ret(ret);

	return ErrCodeSucc;	
}

ErrCodeType CapResData::scanCapResInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::Uint32Req, req);
	REAL_PROTOBUF_MSG(outMsg, db::PlayerResCapInfo, rsp);
	uint32_t region = gArena.GetYesterdayRegionId(h.TargetID);

	uint32_t game_region = req.u32();
	std::string lockkey = makeCapRegionLock(game_region, region);
	while (!gCapResData.TryLock(lockkey)) {
		usleep(10 * 1000);
	}


	CheckProtectCapRes(game_region, region);
	long long result;
	std::string capCandidate = makeTodayCandidateResKey(game_region, region);
	if (!gRedis->scard(capCandidate, result)) {
		WARN_LOG("scard failed! %s",  gRedis->last_error_cstr());
		gCapResData.Unlock(lockkey);
		return ErrCodeDB;
	}


	uint32_t total = 100;
	uint32_t num = 0;
	uint32_t resnum = 5;
	if (result < total) {
		num = result * resnum / total;
		if (num < resnum) {
			num = resnum;
		}
	}
	

	std::vector<std::string> res;
	std::vector<uint32_t> plids;
	std::vector<std::string> vals;
	if (!gRedis->srandmembers(capCandidate, 25, vals)) {
		WARN_LOG("srandmembers failed: %s!", gRedis->last_error_cstr());
		gCapResData.Unlock(lockkey);
		return ErrCodeDB;
	}
	for (auto& v : vals) {
		db::ResBasicInfo info;
		info.ParseFromString(v);
		if (info.owner_id() == h.TargetID) {
			continue;
		}

		std::unordered_map<std::string, std::string> capRes;
		capRes[to_string(info.guid())];
		std::string resKey = makeCapResKey(to_string(info.owner_id()));
		plids.emplace_back(info.owner_id());
		if (!gRedis->hget(resKey, capRes)) {
			WARN_LOG("hgetall failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
			gCapResData.Unlock(lockkey);
			return ErrCodeDB;
		}
		for (const auto& v : capRes) {
			res.emplace_back(v.second);
		}

		num++;
		if (num == resnum) {
			break;
		}
	}

	gCapResData.Unlock(lockkey);

	//--------------这里开始数据操作完毕了------------------------------------------
	uint32_t i = 0;
	for (auto& v : res) {
		auto* info = rsp.add_res();
		info->ParseFromString(v);
		if (info->res_index()) {
			auto* binfo = rsp.add_binfo();
			string accountkey(kKeyPrefixPlayerData + to_string(plids[i]));
			unordered_map<string, string> mAccount;
			auto& accInfoStr = mAccount[kAccountInfo];
			auto& playerAttrStr = mAccount[kPlayerAttr];
			if (!gRedis->hget(accountkey, mAccount)) {
				WARN_LOG("hget failed: %s! plid=%u", gRedis->last_error_message().c_str(), h.TargetID);
				return ErrCodeDB;
			}
			db::AccountInfo accInfo;
			accInfo.ParseFromString(accInfoStr);

			cs::PlayerAttr playerAttr;
			playerAttr.ParseFromString(playerAttrStr);

			binfo->set_name(accInfo.player_name());
			binfo->set_lv(playerAttr.lv());
			binfo->set_pic(accInfo.avatar());
			binfo->set_avatar_frame(accInfo.avatar_frame());

		} else {
			WARN_LOG("should not happen region %u index %u", info->arena_region(), info->res_index());
			return ErrCodeDB;
		}
		i++;
	}

	uint32_t ainum = resnum - num + 1;

	std::list<uint32_t> resNpc;
	BuildNpc(ainum, region, resNpc);
	auto iter = resNpc.begin();
	for (uint32_t i = 0; i < ainum; ++i) {
		auto* info = rsp.add_res();
		info->set_res_id(*iter);
		info->set_arena_region(region);
		iter++;
	}
	return ErrCodeSucc;
}

#if 0
ErrCodeType CapResData::scanCapResInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::Uint32Req, req);
	REAL_PROTOBUF_MSG(outMsg, db::PlayerResCapInfo, rsp);
	uint32_t region = gArena.GetYesterdayRegionId(h.TargetID);

	std::stringstream ss;
	/*
	ss<<kKeyArenaRegonYestorday<<region;
	std::string key = ss.str(); 

	if (!gRedis->zcard(key, &result)) {
		WARN_LOG("zcard failed!");
		return ErrCodeDB;
	}
	
	ss.str("");
	ss.clear();*/


	uint32_t game_region = req.u32();
	std::string lockkey = makeCapRegionLock(game_region, region);
	while (!gCapResData.TryLock(lockkey)) {
		usleep(10 * 1000);
	}

	CheckCapAIResInfo(game_region, region);
	CheckProtectCapRes(game_region, region);

	std::string capAINumLefted = kKeyCapAINumLefted + to_string(region);
	std::string capCapRes = kKeyCapAICapRes + to_string(region);


	std::unordered_map<string, string> mp;
	mp[capCapRes]; 
	mp[capAINumLefted]; 
	std::string hkey = makeCapAIHash(game_region);
	if (!gRedis->hget(hkey, mp)) {
		WARN_LOG("hget failed!");
		gCapResData.Unlock(lockkey);
		return ErrCodeDB;
	}

	long long result;
	//std::string capCandidate = kKeyCandidateResPref + to_string(region;
	std::string capCandidate = makeTodayCandidateResKey(game_region, region);
	if (!gRedis->scard(capCandidate, result)) {
		WARN_LOG("scard failed! %s",  gRedis->last_error_cstr());
		gCapResData.Unlock(lockkey);
		return ErrCodeDB;
	}
	

	uint32_t ailefted = atoi(mp[capAINumLefted].c_str());
	uint32_t total = ailefted + result;

	bool flag = true;
	if (total) {
		uint32_t temp = rand()%total;
		if (temp < result) {
			std::vector<std::string> vals;
			if (!gRedis->srandmembers(capCandidate, 2, vals)) {
				WARN_LOG("srandmembers failed: %s!", gRedis->last_error_cstr());
				gCapResData.Unlock(lockkey);
				return ErrCodeDB;
			}
			db::CandidateResInfo info;
			info.ParseFromString(vals[0]);
			if (info.cid() != h.TargetID || vals.size() != 1) {
				if (info.cid() == h.TargetID) {
					info.ParseFromString(vals[1]);
				}
				std::string resKey;
				if (info.is_ai()) {
					resKey = makeAICapResKey(to_string(info.cid()));
				} else {
					resKey = makeCapResKey(to_string(info.cid()));
				}

				std::unordered_map<std::string, std::string> capRes;
				if (!gRedis->hgetall(resKey, capRes)) {
					WARN_LOG("hgetall failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
					gCapResData.Unlock(lockkey);
					return ErrCodeDB;
				}

				for (const auto& v : capRes) {
					if (v.first == kKeyCapResBasicInfo) {
						rsp.mutable_binfo()->ParseFromString(v.second);
					} else if (v.first == kKeyCapResLastStoreInfo) {
						continue;
					} else {
						auto* temp = rsp.add_res();
						temp->ParseFromString(v.second);
					}
				}
				rsp.mutable_last()->set_plid(info.cid());
				rsp.mutable_last()->set_is_ai(info.is_ai());
				flag = false;

			} else {
				DEBUG_LOG("REAL NOT FIND CAN WAN DE PEOPLE !!!");
				flag = true;
			}
		}
		if (flag && ailefted) {
			uint32_t plid = 1;
			std::list<uint32_t> res;
			BuildNpcRes(game_region, region, res, plid);
			plid = CalAIID(region, plid);

			// 将此ai的数据加入到候选中
			AddToCandidateRes(game_region, region, CalCandidateId(true, plid));
			for (auto& v : res) {
				rsp.mutable_last()->mutable_binfo()->add_res_ids(v);
			}
			rsp.mutable_last()->mutable_binfo()->set_region(region);
			rsp.mutable_last()->set_plid(plid);
			rsp.mutable_last()->set_is_ai(true);
		}
	}

#if 0
	if (total) {
		uint32_t temp = rand()%total;
		if (temp < result) {
			std::vector<std::string> vals;
			if (!gRedis->srandmembers(capCandidate, 2, vals)) {
				WARN_LOG("srandmembers failed: %s!", gRedis->last_error_cstr());
				gCapResData.Unlock(lockkey);
				return ErrCodeDB;
			}
			db::CandidateResInfo info;
			info.ParseFromString(vals[0]);
			if (info.cid() != h.TargetID || vals.size() != 1) {
				if (info.cid() == h.TargetID) {
					info.ParseFromString(vals[1]);
				}
				std::string resKey;
				if (info.is_ai()) {
					resKey = makeAICapResKey(to_string(info.cid()));
				} else {
					resKey = makeCapResKey(to_string(info.cid()));
				}

				std::unordered_map<std::string, std::string> capRes;
				if (!gRedis->hgetall(resKey, capRes)) {
					WARN_LOG("hgetall failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
					gCapResData.Unlock(lockkey);
					return ErrCodeDB;
				}

				for (const auto& v : capRes) {
					if (v.first == kKeyCapResBasicInfo) {
						rsp.mutable_binfo()->ParseFromString(v.second);
					} else if (v.first == kKeyCapResLastStoreInfo) {
						continue;
					} else {
						auto* temp = rsp.add_res();
						temp->ParseFromString(v.second);
					}
				}
				rsp.mutable_last()->set_plid(info.cid());
				rsp.mutable_last()->set_is_ai(info.is_ai());

			}
		} else {
			uint32_t plid = 0;
			std::list<uint32_t> res;
			BuildNpcRes(region, res, plid);
			plid = CalAIID(region, plid);

			// 将此ai的数据加入到候选中
			AddToCandidateRes(region, CalCandidateId(true, plid));
			for (auto& v : res) {
				rsp.mutable_last()->mutable_binfo()->add_res_ids(v);
			}
			rsp.mutable_last()->mutable_binfo()->set_region(region);
			rsp.mutable_last()->set_plid(plid);
			rsp.mutable_last()->set_is_ai(true);
		}
	}
#endif
	// 万一真没有,造一个空ai
	if (!rsp.mutable_last()->plid()) {
		uint32_t plid = 0;
		plid = CalAIID(region, plid);
		rsp.mutable_last()->mutable_binfo()->set_region(region);
		rsp.mutable_last()->set_plid(plid);
		rsp.mutable_last()->set_is_ai(true);
	}

	if (!rsp.last().is_ai() && !rsp.binfo().lv()) {
		string accountkey(kKeyPrefixPlayerData + to_string(rsp.mutable_last()->plid()));
		unordered_map<string, string> mAccount;
		auto& accInfoStr = mAccount[kAccountInfo];
		auto& playerAttrStr = mAccount[kPlayerAttr];
		if (!gRedis->hget(accountkey, mAccount)) {
			WARN_LOG("hget failed: %s! plid=%u", gRedis->last_error_message().c_str(), h.TargetID);
			return ErrCodeDB;
		}
		db::AccountInfo accInfo;
		accInfo.ParseFromString(accInfoStr);

		cs::PlayerAttr playerAttr;
		playerAttr.ParseFromString(playerAttrStr);

		rsp.mutable_binfo()->set_name(accInfo.player_name());
		rsp.mutable_binfo()->set_lv(playerAttr.lv());
		rsp.mutable_binfo()->set_pic(accInfo.avatar());
		rsp.mutable_binfo()->set_avatar_frame(accInfo.avatar_frame());
	}


	if (!rsp.last().is_ai()) {
		unordered_map<string, string> social;
		auto& WealthLv = social[kSocialFieldWealthLv];
		if (!gRedis->hget(kKeyPrefixSocialData + to_string(rsp.mutable_last()->plid()), social)) {
			WARN_LOG("hgetall failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
			return ErrCodeDB;
		}

		rsp.mutable_binfo()->set_wealth_lv(atoi(WealthLv.c_str()) == 0 ? 1 : atoi(WealthLv.c_str()));
	}


	rsp.mutable_binfo()->set_region(region);
	//DEBUG_LOG("CapResData::updateCapResStatus %s", rsp.Utf8DebugString().c_str());

	gCapResData.Unlock(lockkey);

	return ErrCodeSucc;	
}
#endif

ErrCodeType CapResData::updateCapResStatus(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, cs::OfflineInfo, req);
	
	db::PlayerResCapInfo info;
	info.ParseFromString(req.pay_load());

	std::string resKey;
	uint32_t region = 0;
	uint32_t game_region = info.game_region();
	if (info.ai_op()) {
		resKey = makeAICapResKey(to_string(h.TargetID));
		region = gArena.GetYesterdayRegionId(req.from_player_id());
	} else {
		resKey = makeCapResKey(to_string(h.TargetID));
		region = gArena.GetYesterdayRegionId(h.TargetID);
	}
	std::unordered_map<std::string, std::string> capRes;

	for (int i = 0; i < info.res_size(); ++i) {
		capRes[to_string(info.res(i).guid())];
	}

	if (!gRedis->hget(resKey, capRes)) {
		WARN_LOG("hgetall failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}

	uint32_t losenum = 0;
	for (int i = 0; i < info.res_size(); ++i) {
		auto iter = capRes.begin();
		for (; iter != capRes.end(); iter++) {
			db::ResPointInfo restemp;
			restemp.ParseFromString(iter->second);
			if (restemp.guid() == info.res(i).guid()) {
				restemp.set_is_fight(info.res(i).is_fight());
				if (info.res(i).lose_time()) {
					restemp.set_lose_time(info.res(i).lose_time());
					losenum++;
				}
				restemp.SerializeToString(&(iter->second));
				break;
			}
		}
	}

	if (!gRedis->hset(resKey, capRes)) {
		WARN_LOG("hset failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}

#if TODO
	if (losenum) {
		if (checkCapResCanFight(resKey)) {
			AddToCandidateRes(game_region, region, CalCandidateId(info.ai_op(), h.TargetID));
			if (!info.ai_op()) {
				AddToResDis(game_region, h.TargetID);
			}
		} else {
			RemoveFromCandidateRes(game_region, region, CalCandidateId(info.ai_op(), h.TargetID));
			if (!info.ai_op()) {
				RemoveFromResDis(game_region, h.TargetID);
			}
		}
	}
#endif

	return ErrCodeSucc;	
}


//TODEL
ErrCodeType CapResData::freeCapRes(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::Uint32Req, req);
	uint32_t region = gArena.GetYesterdayRegionId(h.TargetID);

	uint32_t game_region = req.u32();
	std::string lockkey = makeCapRegionLock(game_region, region);
	while (!gCapResData.TryLock(lockkey)) {
		usleep(10 * 1000);
	}

	std::string capAINumLefted = kKeyCapAINumLefted + to_string(region);
	std::string capAICapRes = kKeyCapAICapRes + to_string(region);
	std::string capResRegionTotal = kKeyCapResRegionTotal + to_string(region);
	std::string capResPlayerCurRes = kKeyCapResPlayerCurRes + to_string(region);


	std::unordered_map<string, string> mp;
	mp[capAINumLefted];
	mp[capAICapRes];
	mp[capResRegionTotal];
	mp[capResPlayerCurRes];
	
	std::string hkey = makeCapAIHash(game_region);
	if (!gRedis->hget(hkey, mp)) {
		WARN_LOG("hset failed!");
		gCapResData.Unlock(lockkey);
		return ErrCodeDB;
	}

	uint32_t aiNumLefted = atoi(mp[capAINumLefted].c_str());
	uint32_t aiPreRes = atoi(mp[capAICapRes].c_str());
	uint32_t total = atoi(mp[capResRegionTotal].c_str());
	uint32_t playerCurRes = atoi(mp[capResPlayerCurRes].c_str());
	
	if (playerCurRes) {
		--playerCurRes;
	}
	mp[capResPlayerCurRes] = to_string(playerCurRes);
	if (playerCurRes + aiPreRes + 1 <= total) {
		aiPreRes++;
		if (aiNumLefted * 5 < aiPreRes) {
			aiNumLefted++;
		}

		mp[capAINumLefted] = to_string(aiNumLefted);
		mp[capAICapRes] = to_string(aiPreRes);
	}


	if (!gRedis->hset(hkey, mp)) {
		WARN_LOG("hset failed!");
		gCapResData.Unlock(lockkey);
		return ErrCodeDB;
	}


	gCapResData.Unlock(lockkey);

	std::string resKey = makeCapResKey(to_string(h.TargetID));
	if (!checkCapResCanFight(resKey)) {
		RemoveFromResDis(game_region, h.TargetID);
	}

	/*
	REAL_PROTOBUF_MSG(inMsg, db::CapResUpdateBtlInfo, req);
	std::string resKey = makeCapResKey(to_string(h.TargetID));
	std::unordered_map<std::string, std::string> capRes;
	auto& capResStr = capRes[to_string(req.guid())];
	if (!gRedis->hget(resKey, capRes)) {
		WARN_LOG("hgetall failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}

	db::ResPointInfo restemp; 
	restemp.ParseFromString(capResStr);
	restemp.mutable_blt_info()->CopyFrom(req.blt_info());
	restemp.SerializeToString(&capResStr);

	if (!gRedis->hset(resKey, capRes)) {
		WARN_LOG("hgetall failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}*/

	return ErrCodeSucc;
}



ErrCodeType CapResData::getPendantInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::Uint32Req, req);
	REAL_PROTOBUF_MSG(outMsg, db::PendantInfo, rsp);
	uint32_t plid = req.u32();

	unordered_map<string, string> m;
	string key(kKeyPrefixPlayerData + to_string(plid));
	auto& playerAttrStr = m[kPlayerAttr];
	if (!gRedis->hget(key, m)) {
		WARN_LOG("hget failed: %s! plid=%u", gRedis->last_error_cstr(), plid);
		return ErrCodeDB;
	}

	cs::PlayerAttr playerAttr;
	playerAttr.ParseFromString(playerAttrStr);



	// 家园资源
	packHomeRes(plid, rsp);

	auto* item = gCSVModuleUnlock.GetItem(21);

	tm tmNow(*GetNowTm());
	uint32_t now = mktime(&tmNow);
	if (item->Lv <= playerAttr.lv()) {
		//资源保卫战
		std::string key = kKeyPrefixPlayerAcInfo + to_string(plid);
		unordered_map<string, string> fields;
		auto& str = fields[to_string(cs::kResOccTimes)];

		if (!gRedis->hget(key, fields)) {
			WARN_LOG("hget failed: %s! plid=%u ", gRedis->last_error_message().c_str(), plid);
			return ErrCodeDB;
		}

		cs::PlayerAcInfo acinfo;
		acinfo.ParseFromString(str);

		uint32_t times = acinfo.val();
		auto* item = gCSVResOption.GetItem(1);
		if (acinfo.deadtm() >  now) {
			if ((uint32_t)item->Val > acinfo.val()) {
				times = item->Val - acinfo.val();
			} else {
				times = 0;
			}
		} else {
			times = item->Val;	
		}

		rsp.set_capres(times);
	} else {
		rsp.set_capres(0);
	}

	// 冠军赛次数
	item = gCSVModuleUnlock.GetItem(15);
	if (item->Lv <= playerAttr.lv()) {
		packArena(plid, rsp);
	} else {
		rsp.set_arena(0);
	}
 
	// 孵化
	//  
	packHatch(plid, rsp);


	// 打包主宠 
	packPrimaryMon(plid, rsp);

	// 打包抚摸奖励
	std::string pkey = kKeyPrefixPlayerAcInfo + to_string(plid);
	unordered_map<string, string> fields;
	auto& times = fields[to_string(cs::kDailyMonRewardIndex)];
	auto& time = fields[to_string(cs::kDailyMonRewardTime)];

	if (!gRedis->hget(pkey, fields)) {
		WARN_LOG("hget failed: %s! plid=%u ", gRedis->last_error_message().c_str(), plid);
		return ErrCodeDB;
	}

	cs::PlayerAcInfo acinfo1, acinfo2;
	acinfo1.ParseFromString(times);
	acinfo2.ParseFromString(time);

	uint32_t left = 86400;
	if (acinfo1.deadtm() >  now) {
		 auto* item = gCSVDailyMonsterGift.GetItem(acinfo1.val() + 1);
		 if (item) {
			 if (acinfo2.val() + item->Time*60 < now) {
				 left = 0;
			 } else {
				 left = acinfo2.val() + item->Time * 60 - now;
			 }
		 }

	} else {
		left = 0;
	}

	rsp.set_mon_reward_time(left);
	return ErrCodeSucc;	
}


void CapResData::CheckCapAIResInfo(uint32_t game_zone, uint32_t region)
{
	std::unordered_map<string, string> mp;

	std::string timekey = kKeyCapAITime + to_string(region);
	mp[timekey];
	std::string hkey = makeCapAIHash(game_zone);
	if (!gRedis->hget(hkey, mp)) {
		WARN_LOG("hget failed!");
		return;
	}

	uint32_t passtime = atoi(mp[timekey].c_str());

	time_t now = time(NULL);
	// 已经初始化完毕
	if (now < passtime) {
		return;
	}

	std::stringstream ss;
	ss<<kKeyArenaRegonYestorday<<region;
	std::string key = ss.str(); 

	long long result;
	if (!gRedis->zcard(key, &result)) {
		WARN_LOG("hget failed!");
		return;
	}

	DEBUG_LOG("Check Cap Ai Res Info GameZone %u Region %u Num %lld", game_zone, region, result);
	uint32_t num = result;
	uint32_t capResPlayerCurRes = 0;

	string resdisKey = makeCapRegionDis(game_zone, region);
	vector<string> plids;
	if(!gRedis->smembers(resdisKey, plids)) {
		WARN_LOG("hget failed!");
		// gCapResData.Unlock(lockkey);
		return;
	}

	vector<string> datas;
	for (auto& v : plids) {
		string key = makeCapResKey(v);
		uint32_t temp = getCapResNum(key);
		DEBUG_LOG("plid %s %u", key.c_str(), temp);
		if (temp) {
			capResPlayerCurRes += temp;
			
			datas.emplace_back(CalCandidateId(false, atoi(v.c_str())));
		}
	}

	std::string candidateKey = makeTodayCandidateResKey(game_zone, region);

	if (!gRedis->del({candidateKey})) {
		WARN_LOG("del failed!");
		// gCapResData.Unlock(lockkey);
		return;
	}

	if (!datas.empty()) {
		if (!gRedis->sadd(candidateKey, datas)) {
			WARN_LOG("sadd failed!");
			// gCapResData.Unlock(lockkey);
			return;
		}
	}


	now = GetNextFiveClock();
	mp[timekey] = to_string(now);


	/*
	std::unordered_map<string, string> pres;
	pres[presKey];
	if (!gRedis->hget(kKeyCapAIHash, mp)) {
		WARN_LOG("hget failed!");
		gCapResData.Unlock(lockkey);
		return;
	}*/

	//uint32_t presnum = atoi(pres[presKey].c_str());

	auto* config = gCapResRegionConfig.GetRegionInfo(region);
	uint32_t per = config->ResPro;

	if (!num) {	// 该段位无人 则自动500 资源
		num = 500;
	} else {
		num = (per * num  + 99 )/ 100;
		if (num < 6) {
			num = 6;	// 起码是6个资源
		}
	}

	// 今日总的流通资源
	string capResRegionTotal = kKeyCapResRegionTotal + to_string(region);
	mp[capResRegionTotal] = to_string(num); 
	uint32_t ainum = 0;
	if (num <= capResPlayerCurRes) {
		num = 0;
	} else {
		num = num - capResPlayerCurRes;
		uint32_t min = (num + 4) / 5;
		uint32_t max = ((num<<2) + 4) / 5;
		ainum = min + rand() % max;
		// 最多是num
		if (!num) {
			ainum = 0;
		} else if (ainum > num) {
			ainum = num;
		}
	}

	//  当前ai所拥有的资源
	std::string capAICurNum = kKeyCapAICurNum + to_string(region);
	std::string capAINumLefted = kKeyCapAINumLefted + to_string(region);
	std::string capAICapRes = kKeyCapAICapRes + to_string(region);
	std::string presKey = kKeyCapResPlayerCurRes + to_string(region);

	mp[capAICurNum] = "0";
	mp[capAINumLefted] = to_string(ainum);
	mp[capAICapRes] = to_string(num); 
	mp[presKey] = to_string(capResPlayerCurRes);


	if (!gRedis->hset(hkey, mp)) {
		WARN_LOG("hset failed!");
		// gCapResData.Unlock(lockkey);
		return;
	}

	// gCapResData.Unlock(lockkey);
	DEBUG_LOG("CheckCapAIResInfo per %u num %u ainum %u", per, num, ainum);
}


void CapResData::BuildNpcRes(uint32_t game_region, uint32_t region, std::list<uint32_t>& res, uint32_t& plid)
{

	/* std::string lockkey = kKeyCapRegionLock + to_string(region);
	while (!gCapResData.TryLock(lockkey)) {
		usleep(10 * 1000);
		DEBUG_LOG("try lock key %s fail", lockkey.c_str());
	}*/


	std::unordered_map<string, string> mp;

	std::string capAICurNum = kKeyCapAICurNum + to_string(region);
	std::string capAINumLefted = kKeyCapAINumLefted + to_string(region);
	std::string capAICapRes = kKeyCapAICapRes + to_string(region);

	mp[capAICurNum]; 
	mp[capAICapRes]; 
	mp[capAINumLefted];

	std::string key = makeCapAIHash(game_region);

	if (!gRedis->hget(key, mp)) {
		WARN_LOG("hget failed!");
		return;
	}

	uint32_t cainum = atoi(mp[capAICurNum].c_str());
	
	uint32_t left = atoi(mp[capAINumLefted].c_str());
	uint32_t resnum = 1;
	uint32_t leftres = atoi(mp[capAICapRes].c_str());
	uint32_t leftmax = (leftres << 1) / left;

	if (left == 1) {	
		resnum = leftres;
		left--;
	} else {
		if (leftres < 5) {
			resnum = rand() % leftres  + 1;
		} else {
			resnum = rand() % 4 + 1;
		}

		if (resnum > leftmax) {
			resnum = leftmax;
		}

		left--;
		while (left > (leftres - resnum )) {
			resnum --;
		}
		while ( (left * 5) < (leftres - resnum )) {
			resnum ++;
		}
	}

	BuildNpc(resnum, region, res);
	cainum++;
	leftres = leftres - resnum;
	mp[capAICurNum] = to_string(cainum);
	mp[capAICapRes] = to_string(leftres);
	mp[capAINumLefted] = to_string(left);

	if (!gRedis->hset(key, mp)) {
		WARN_LOG("hget failed!");
		return;
	}
	plid = cainum;

	// gCapResData.Unlock(lockkey);
}


void CapResData::BuildNpcResTest(uint32_t num, uint32_t region)
{
    uint32_t min = (num + 4) / 5;
    uint32_t max = ((num<<2) + 4) / 5;
    uint32_t ainum = min + rand() % max;
	uint32_t cainum = 0;
	uint32_t pres = num;
	uint32_t cres = 1;

	for (int i = 0; i < (int)ainum; i++) {
		uint32_t left = ainum - cainum - 1;
		uint32_t resnum = 1;
		uint32_t leftres = pres - cres;
		std::list<uint32_t>  res;
		uint32_t leftmax = (leftres << 1) / (ainum - cainum);

		if (!left) {
			resnum = pres - cres;
		} else {
			if (leftres < 5) {
				resnum = rand() % leftres  + 1;
			} else {
				resnum = rand() % 4 + 1;
			}

			if (resnum > leftmax) {
				resnum = leftmax;
			}

			while (left > (pres - cres - resnum )) {
				resnum --;
			}
			while ( (left * 5) < (pres - cres - resnum )) {
				resnum ++;
			}
		}

		BuildNpc(resnum, region, res);
		cainum++;
		cres = cres + resnum;
		stringstream ss;
		for (auto& v : res) {
			ss<<v<<" ";
		}
		DEBUG_LOG("Player %u # %s", i, ss.str().c_str());
	}
	DEBUG_LOG("After test all %u c %u rall %u rc %u", ainum, cainum, pres, cres);
}


uint32_t CapResData::CalAIID(uint32_t region, uint32_t num)
{
	return region * 10000000 + num;	
}


void CapResData::AddToCandidateRes(uint32_t game_region, uint32_t region, const std::string& key)
{
	std::string todayCandidateResKey = makeTodayCandidateResKey(game_region, region);
	vector<string> ckey { key };
	if (!gRedis->sadd(todayCandidateResKey, ckey)) {
		WARN_LOG("sadd failed: %s!", gRedis->last_error_cstr());
	}
}


void CapResData::RemoveFromCandidateRes(uint32_t game_region, uint32_t region, const std::string& key)
{
	std::string todayCandidateResKey = makeTodayCandidateResKey(game_region, region);
	vector<string> ckey { key };
	if (!gRedis->srem(todayCandidateResKey, ckey)) {
		WARN_LOG("sadd failed: %s!", gRedis->last_error_cstr());
	}
}

// TODEL
bool CapResData::checkCapResCanFight(std::string& resKey)
{
	unordered_map<string, string> capRes;
	if (!gRedis->hgetall(resKey, capRes)) {
		WARN_LOG("hgetall failed: %s!", gRedis->last_error_cstr());
		return ErrCodeDB;
	}

	db::ResPointInfo info;
	for (const auto& v : capRes) {
		if (v.first != kKeyCapResBasicInfo 
				&& v.first != kKeyCapResLastStoreInfo) {
			info.ParseFromString(v.second);
			//DEBUG_LOG("%s", info.Utf8DebugString().c_str());
			if (info.res_id() 
					&& !info.is_fight()
					&& !info.lose_time()) {
				return true;
			}
		}
	}


	return false;	
}


// TODEL
uint32_t CapResData::getCapResNum(std::string& resKey)
{
	unordered_map<string, string> capRes;
	if (!gRedis->hgetall(resKey, capRes)) {
		WARN_LOG("hgetall failed: %s!", gRedis->last_error_cstr());
		return ErrCodeDB;
	}

	uint32_t num = 0;

	db::ResPointInfo info;
	for (const auto& v : capRes) {
		if (v.first != kKeyCapResBasicInfo 
				&& v.first != kKeyCapResLastStoreInfo) {
			info.ParseFromString(v.second);
			if (info.res_id() 
					&& !info.lose_time()) {
				num++;
			}
		}
	}

	return num;	
}


//TODEL
void CapResData::randGetPlayerResInfo(uint32_t region, uint32_t result, string& key, uint32_t explid, uint32_t& plid)
{
	std::vector<std::string> tempResult;
	uint32_t num = result > 100 ? 100 : result; 
	// 随机到真人
	if (!gRedis->zrevrange(key, 0, num, tempResult)) {
		WARN_LOG("zrevrange failed!");
	}

	uint32_t index = rand() % num;
	plid = atoi(tempResult[index].c_str());
	if (plid == explid) {
		index = (index+1) % num;
		plid = atoi(tempResult[index].c_str());
	}
}

//TODEL
string CapResData::CalCandidateId(bool isAI, uint32_t id)
{
	string temp;
	db::CandidateResInfo info;
	info.set_is_ai(isAI);
	info.set_cid(id);
	info.SerializeToString(&temp);
	return temp;
}


void CapResData::CheckProtectCapRes(uint32_t game_region, uint32_t region)
{
	vector<string> keys;
	if (!gRedis->zrangebyscore(makeProtectRes(game_region, region), 0.0f, time(NULL), keys)) {
		WARN_LOG("zrangebyscore failed: %s!", gRedis->last_error_cstr());
		return;
	}

	if (!keys.empty()) {
		if (!gRedis->zrem(makeProtectRes(game_region, region), keys)) {
			WARN_LOG("zrem failed: %s!", gRedis->last_error_cstr());
			return;
		}
	}

	for (auto& v : keys) {
		uint32_t plid = atoi(v.c_str());
		uint32_t region = gArena.GetYesterdayRegionId(plid);
		AddToCandidateRes(game_region, region, CalCandidateId(false, plid));
	}
}


void CapResData::PushProtectCapRes(uint32_t game_region, uint32_t region, uint32_t ptime, uint32_t pid)
{
	if (!gRedis->zadd(makeProtectRes(game_region, region), ptime, to_string(pid))) {
		WARN_LOG("hgetall failed: %s!", gRedis->last_error_cstr());
	}
}


//TODEL
void CapResData::RemoveFromResDis(uint32_t game_region, uint32_t plid)
{
	string key = makeResRegionHash(game_region);
	unordered_map<string, string> regions;
	regions[to_string(plid)];
	bool exist;
	if (!gRedis->hexists(key, to_string(plid), exist)) {
		WARN_LOG("hexists failed: %s!", gRedis->last_error_cstr());
		return;
	}

	if (exist) {
		if (!gRedis->hget(key, regions)) {
			WARN_LOG("hget failed: %s!", gRedis->last_error_cstr());
			return;
		}
		string resDis = makeCapRegionDis(game_region, atoi(regions[to_string(plid)].c_str()));	
		vector<string> del = { to_string(plid) };
		if (!gRedis->srem(resDis, del)) {
			WARN_LOG("srem failed: %s!", gRedis->last_error_cstr());
			return;
		}

		if (!gRedis->hdel(key, del)) {
			WARN_LOG("srem failed: %s!", gRedis->last_error_cstr());
			return;
		}
	}

}


void CapResData::AddToResDis(uint32_t game_region, uint32_t plid)
{
	unordered_map<string, string> regions;
	regions[to_string(plid)];
	uint32_t region = 0;
	cs::ArenaInfo aInfo;
	gArena.GetInfoImp(plid, aInfo);
	region = aInfo.region();
	bool exist;
	vector<string> del = { to_string(plid) };
	std::string key = makeResRegionHash(game_region);
	if (!gRedis->hexists(key, to_string(plid), exist)) {
		WARN_LOG("hexists failed: %s!", gRedis->last_error_cstr());
		return;
	}

	if (exist) {
		if (!gRedis->hget(key, regions)) {
			WARN_LOG("hget failed: %s!", gRedis->last_error_cstr());
			return;
		}
		uint32_t oregion = atoi(regions[to_string(plid)].c_str());
		if (oregion == region) {
			return;
		}

		string resDis = makeCapRegionDis(game_region, oregion);	
		if (!gRedis->srem(resDis, del)) {
			WARN_LOG("srem failed: %s!", gRedis->last_error_cstr());
			return;
		}
	}

	string resDis = makeCapRegionDis(game_region, region);
	DEBUG_LOG("resDis %s game region %u %u region %u", resDis.c_str(), game_region, plid, region);
	if (!gRedis->sadd(resDis, del)) {
		WARN_LOG("sadd failed: %s!", gRedis->last_error_cstr());
		return;
	}

	regions[to_string(plid)] = to_string(region);
	if (!gRedis->hset(key, regions)) {
		WARN_LOG("hset failed: %s!", gRedis->last_error_cstr());
		return;
	}

}


void CapResData::checkPlayerResProtect(uint32_t game_region, uint32_t region, uint32_t plid)
{
	std::string lockkey = makeCapRegionLock(game_region, region);
	while (!gCapResData.TryLock(lockkey)) {
		usleep(10 * 1000);
	}

	//========================= 如果不在塞入保护set
	//========================= 将所有res Remove出 候选	
	gCapResData.Unlock(lockkey);
}


void CapResData::registerRes(uint32_t game_region, uint32_t region, uint32_t plid, bool isProtect, db::ResPointInfo& info)
{
	db::ResBasicInfo cinfo;
	std::string temp;

	// res_id是0 定是领取的时候
	if (!info.res_id()) {
		if (info.res_index()) {
			cinfo.set_arena_region(info.arena_region());
			cinfo.set_res_index(info.res_index());
			cinfo.set_owner_id(plid);
			cinfo.set_guid(info.guid());
			cinfo.SerializeToString(&temp);
			RemoveFromCandidateRes(game_region, cinfo.arena_region(), temp);
		}
		return;
	}

#if 0
	// 被别人打了
	if (info.lose_time()) {
		if (info.res_index()) {
			db::ResBasicInfo cinfo;
			std::string temp;
			cinfo.set_arena_region(info.arena_region());
			cinfo.set_res_index(info.res_index());
			cinfo.set_owner_id(plid);
			cinfo.set_guid(info.guid());
			cinfo.SerializeToString(&temp);
			RemoveFromCandidateRes(game_region, cinfo.arena_region(), temp);
		} else {
			DEBUG_LOG("Should not happend!!!");
		}
		return;
	}
#endif
	
	// 占领ai新资源
	if (!info.res_index()) {
		uint32_t res_index = 0;
		//=========================补上
		info.set_arena_region(region);
		info.set_res_index(res_index);
	}
	if (!isProtect) {
		cinfo.set_arena_region(region);
		cinfo.set_res_index(info.res_index());
		cinfo.set_owner_id(plid);
		cinfo.set_guid(info.guid());
		cinfo.SerializeToString(&temp);
		AddToCandidateRes(game_region, region, temp);
	}
}

//===========================================================================

void CapResData::Init()
{
	gCapResRegionConfig.Init();
}

bool CapResData::TryLock(const std::string& key)
{
	long long result;
	gRedis->incrby(key, 1, &result);
	if (result == 1) {
		// 设置最小的死锁判定时间, 五秒
		gRedis->expire(key, 5);
		return true;
	}
	return false;
}

void CapResData::Unlock(const std::string& key)
{
	gRedis->set(key, "0");
}


uint32_t CapResData::GetNextFiveClock()
{
	time_t now = time(NULL);
	return ant::today_begin_time(now) + 29 * 3600;
}


uint32_t CapResData::GetLastFiveClock()
{
	time_t now = time(NULL);
	return ant::today_begin_time(now) + 5 * 3600;
}


// num 1- 5 之间
void CapResData::BuildNpc(uint32_t num, uint32_t region, std::list<uint32_t>& res)
{
	auto* config = gCapResRegionConfig.GetRegionInfo(region);
	ProbGen<uint32_t> temp;
	int32_t inc = 1;
	if (config->Probs.size() < num) {
		inc = (num + config->Probs.size()  - 1) / config->Probs.size();
	}

	for (int i = 0; i < inc; i++) {
		for (auto& v : config->Probs) {
			temp.Push(v.first, v.second);
		}
	}

	int32_t total = num;
	while(total != 0) {
		std::unordered_map<uint32_t, uint32_t> types;
		uint32_t resid = temp.GetOneWithRemove();
		auto* item = gCSVResInfo.GetItem(resid);
		if (item) {
			auto iter = types.find(item->ResType);
			if (iter == types.end()) {
				types[item->ResType] = 1;
				res.emplace_back(resid);
				total--;
			} else {
				if (types[resid] == 1) {
					types[item->ResType]++;
					res.emplace_back(resid);
					total--;
				}
			}
		}
	}	

}

//===========================================================================
void CapResData::packHomeRes(uint32_t plid, db::PendantInfo& info)
{
	uint32_t finish = 0;
	std::ostringstream oss;
	oss << kKeyPrefixHomeResFactory << "_" << plid;
	auto uKey = oss.str();
	std::vector<std::string> list;
	std::unordered_map<std::string, std::string> data;
	gRedis->hgetall(uKey, data);
	tm tmNow(*GetNowTm());
	uint32_t now = mktime(&tmNow);

	for (auto it : data) {
		cs::HomeResFactory fac;
		fac.ParseFromString(it.second);
		//DEBUG_LOG("fac %s", fac.Utf8DebugString().c_str());
		if (fac.factory_id() == 14) {
			if ((fac.update_time() + fac.produce_time()) <= now) {
				finish = 1;
				break;
			} else {
				continue;
			}	
		} else if ((fac.produce_state() == cs::Produce && fac.finish_time() <= now) || fac.produce_state() == cs::Complete){
			finish = 1;
			break;
		}
	}
	info.set_home_res(finish);
}

void CapResData::packArena(uint32_t plid, db::PendantInfo& info)
{
	std::string key = kKeyPrefixPlayerData + to_string(plid);
	unordered_map<string, string> m;
	auto& arenaInfo = m[kArenaInfo];
	cs::PlayerArenaInfo paInfo;

	if (!gRedis->hget(key, m)) {
		WARN_LOG("hget failed: %s! plid=%u", gRedis->last_error_message().c_str(), plid);
		return;
	}

	paInfo.ParseFromString(arenaInfo);

	info.set_arena(paInfo.match_cnt());
}

void CapResData::packHatch(uint32_t plid, db::PendantInfo& info)
{
	std::string key = kKeyPrefixHatchData + to_string(plid);
	unordered_map<string, string> m;
	if (!gRedis->hgetall(key, m)) {
		WARN_LOG("loadHatchData hgetall failed: %s! plid=%u", gRedis->last_error_cstr(), plid);
	}

	tm tmNow(*GetNowTm());
	uint32_t now = mktime(&tmNow);
	uint32_t finish = 0;
	for (const auto& v : m) {
		db::HatchInfo hinfo;
		hinfo.ParseFromString(v.second);
		//DEBUG_LOG("CapResData packHatch %s", hinfo.Utf8DebugString().c_str());
		if (now < hinfo.last_save_time()) {
			continue;
		}
		if ((now - hinfo.last_save_time()) >= hinfo.remaining_hatch_time()) {
			finish = 1;
			break;
		}
	}
	info.set_hatch(finish);
}

void CapResData::packPrimaryMon(uint32_t plid, db::PendantInfo& rsp)
{
	// 主宠物
	std::string key = kKeyPrefixPlayerData + to_string(plid);
	unordered_map<string, string> m;
	auto& monBagStr = m[kMonsterBag];
	db::MonsterBag monBag;

	if (!gRedis->hget(key, m)) {
		WARN_LOG("hget failed: %s! plid=%u", gRedis->last_error_message().c_str(), plid);
		return;
	}

	monBag.ParseFromString(monBagStr);

	if (monBag.primary_guid()) {
		m.clear();
		auto& monStr = m[kMonHashKeyPrefix + to_string(monBag.primary_guid())];
		gRedis->hget(key, m); // 这里拉取失败就算了，无所谓
		if (monStr.size()) {
			db::MonsterInfo mon;
			mon.ParseFromString(monStr);

			auto* item = gCSVMonster.GetItem(mon.id());
			if (item) {
				rsp.set_monid(item->ModelId);
				if (mon.nick() != "") {
					rsp.mutable_monname()->swap(*mon.mutable_nick());
				} else {
					rsp.set_monname(item->Name);
				}
			}
		}
	}
}

//============================================================================

CapResRegionItem::CapResRegionItem(CSVNpcRes::Item* item)
{
	ResPro = item->SumRatio;
	for (auto& v: item->PerRatio) {
		auto item = Split(v, '#');
		if (item.size() == 2) {
			Probs.emplace(atoi(item[0].c_str())
					,atoi(item[1].c_str()));
		}
	}
}

void CapResRegionConfig::Init()
{
	for (auto& item : gCSVNpcRes.AllItems) {
		Items.emplace(std::piecewise_construct
						,std::forward_as_tuple(item.first)
						,std::forward_as_tuple(item.second));
	}
}

