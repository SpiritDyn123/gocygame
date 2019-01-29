/*===============================================================
* @Author: car
* @Created Time : 2017年09月14日 星期四 16时57分21秒
*
* @File Name: CorpsData.cc
* @Description:
*
================================================================*/
#include <vector>
#include <string>
#include <sstream>

#include <google/protobuf/util/json_util.h>
#include <algorithm>

#include "../core/redis_client.h"
#include "../core/dispatcher.h"
#include "../core/ProxyConnector.h"
#include "../proto/SvrProtoID.pb.h"
#include "../proto/CenterServer.pb.h"
#include "../proto/ConfigServer.pb.h"
#include "../TimeUtils.h"
#include "../core/ScriptMgr.h"
#include "../CSV/CSVOptionInfo.h"
#include "../tables/tbl_corp_nick.h"
#include "CorpsData.h"
#include "KeyPrefixDef.h"
#include "GeoData.h"
#include "CapRes.h"

using namespace std;


#define LOCKCORPSPOSINFO    \
	while (!gCapResData.TryLock(kKeyLockCorpPos)) {  \
		usleep(10 * 1000);   \
	}


#define UNLOCKCORPSPOSINFO  gCapResData.Unlock(kKeyLockCorpPos)

CorpsData gCorpsData;

CorpsData::CorpsData()
{
	gMsgDispatcher.RegisterHandler(DBProtoListCorpsInfo, *this, &CorpsData::listCorpsInfo, new db::ListCorpsInfoReq, new db::ListCorpInfoRsp);	
	gMsgDispatcher.RegisterHandler(DBProtoListCorpsInfoByName, *this, &CorpsData::listCorpsInfoByName, new db::ListCorpsInfoByNameReq, new db::ListCorpInfoRsp);	
	gMsgDispatcher.RegisterHandler(DBProtoNewCorpsInfo, *this, &CorpsData::newCorpsInfo, new db::NewCorpsInfoReq, new db::NewCorpsInfoRsp);	
	gMsgDispatcher.RegisterHandler(DBProtoUpdateCorpsInfo, *this, &CorpsData::updateCorpsInfo, new db::UpdateCorpsInfoReq, new db::UpdateCorpsInfoRsp);	
	gMsgDispatcher.RegisterHandler(DBProtoCheckName, *this, &CorpsData::checkName, new db::CheckNameReq, new db::CheckNameRsp);	
	gMsgDispatcher.RegisterHandler(DBProtoUpdatePlayerCInfo, *this, &CorpsData::updatePlayerCorpsInfo, new db::UpdatePlayerCorpInfoReq, new db::UpdatePlayerCorpInfoRsp);	
	gMsgDispatcher.RegisterHandler(DBProtoDelPlayerInviteInfo, *this, &CorpsData::delPlayerCorpsInfo, new db::DelPlayerInviteInfoReq, nullptr);	
	gMsgDispatcher.RegisterHandler(DBProtoUpdateTaskData, *this, &CorpsData::updateCorpsTaskInfo, new db::UpdateTaskDataReq, nullptr);	
	gMsgDispatcher.RegisterHandler(DBProtoCheckCorpsInfo, *this, &CorpsData::checkCorpsInfo, new db::RepeatedUint32Req, new db::CheckPlayersRsp);	
	gMsgDispatcher.RegisterHandler(DBProtoPushCommonLog, *this, &CorpsData::commonLogPush, new db::PushLogReq, nullptr);	
	gMsgDispatcher.RegisterHandler(DBProtoScanCommonLog, *this, &CorpsData::commonLogScan, new db::ListLog, new db::ListLog);	
	gMsgDispatcher.RegisterHandler(DBProtoListUint32Hash, *this, &CorpsData::ListUint32Hash, new db::Uint32HashList, new db::Uint32HashList);	
	gMsgDispatcher.RegisterHandler(DBProtoSetUint32Hash, *this, &CorpsData::SetUint32Hash, new db::Uint32HashUnitSet, nullptr);	
	gMsgDispatcher.RegisterHandler(DBProtoInviteRefuse, *this, &CorpsData::InviteRefuse, new db::RepeatedUint32Req, nullptr);
	gMsgDispatcher.RegisterHandler(DBProtoQueryCorpsInfo, *this, &CorpsData::listCorpsInfo, new db::ListCorpsInfoReq, new db::ListCorpInfoRsp);
	gMsgDispatcher.RegisterHandler(DBProtoCSkillUpdate, *this, &CorpsData::updateCorpsSkills, new db::PlayerCorpsSkillInfo, nullptr);

	//gMsgDispatcher.RegisterHandler(DBProtoRegisterData, *this, &CorpsData::RegisterDataCom, new db::RegisterData, new db::RegisterData);	
	//gMsgDispatcher.RegisterHandler(DBProtoFreeData, *this, &CorpsData::FreeDataCom, new db::RegisterData, new db::RegisterData);	
	//gMsgDispatcher.RegisterHandler(DBProtoGetRegisterData, *this, &CorpsData::GetRegisterDataList, new db::RepeatedUint32Req, new db::GetRegisterDataRsp);
	gMsgDispatcher.RegisterHandler(DBProtoGetAllCorpsID, *this, &CorpsData::GetAllCorpsID, nullptr, new db::RepeatedUint32Req);

}



ErrCodeType CorpsData::listCorpsInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::ListCorpsInfoReq, req);
	REAL_PROTOBUF_MSG(outMsg, db::ListCorpInfoRsp, rsp);

	if (req.keys_size()) {
		for (int i = 0; i < req.keys_size(); ++i) {
			getCorpsInfo(req.keys(i), *(rsp.add_infos()), req.cli());
		}
		for (int i = 0; i < req.tempkeys_size(); ++i) {
			rsp.add_tempkeys(req.tempkeys(i));
		}
	} else {
		getNeighborCorps(req.pos(), rsp, req.region());
	}
	//DEBUG_LOG("CorpsData::listCorpsInfo %s", rsp.Utf8DebugString().c_str());

	return ErrCodeSucc;
}


ErrCodeType CorpsData::listCorpsInfoByName(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::ListCorpsInfoByNameReq, req);
	REAL_PROTOBUF_MSG(outMsg, db::ListCorpInfoRsp, rsp);

	uint32_t index = getCidByName(req.name());
	if (index) {
		getCorpsInfo(index, *(rsp.add_infos()), true);
	}

	return ErrCodeSucc;
}


ErrCodeType CorpsData::newCorpsInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::NewCorpsInfoReq, req);
	REAL_PROTOBUF_MSG(outMsg, db::NewCorpsInfoRsp, rsp);
	auto* info = rsp.mutable_infos();

	uint32_t ret = 0;
	uint32_t region = req.region();
	long long val = 0;
	
	//uint64_t id = req.infos().info().corps_hid();
	
	unordered_map<string, string> players;
	for (int i = 0; i < req.infos().players_size(); ++i) {
		//bool exists = false;
		uint32_t plid = req.infos().players(i).plid();
		ret = checkCorps(plid);
		if (ret) {
			break;
		}
		req.infos().players(i).SerializeToString(
				&(players[std::to_string(req.infos().players(i).plid())])
				);
	}

	if (!ret) {
		/*uint32_t index = getCidByName(req.infos().info().corps_name());
		if (index) {
			ret = ErrCodeCorpNameAready;
		}*/
		bool exist;
		TblCorpNick tblNick(0);
		if (tblNick.CheckNick(req.infos().info().corps_name(), exist) != ErrCodeSucc) {
			return ErrCodeDB;
		}
		if (exist) {
			ret = ErrCodeCorpNameAready;
		}
	}

	if (!ret) {
		gRedis->incrby(kKeyCorpsGuid, 1, &val);
		uint32_t id = val + 10000;
		//id = ((id<<32) | val);
		std::string basic(makeCorpsBasicKey(std::to_string(id)));
		std::string key(makeCorpsPlayersKey(std::to_string(id)));
		std::string buff;
		req.mutable_infos()->mutable_info()->set_corps_id(id);
		req.mutable_infos()->mutable_info()->set_region(region);
		req.infos().info().SerializeToString(&buff);

		TblCorpNick tblNick(id);
		if (tblNick.InsertNick(req.infos().info().corps_name()) == ErrCodeSucc) {
			if (!gRedis->set(basic, buff)) {
				WARN_LOG("hset failed: %s! corpsid=%u", gRedis->last_error_cstr(), id);
				return ErrCodeDB;
			} else {
				DEBUG_LOG("pid=%u set corps info  corps_id %u corps_lv %u", h.TargetID, id, info->info().lv());
				if (!gRedis->hset(key, players)) {
					WARN_LOG("hset failed: %s! corpsid=%u", gRedis->last_error_cstr(), id);
					return ErrCodeDB;
				} else {
					unordered_map<string, string> c;
					c[req.infos().info().corps_name()] = to_string(id);
					if (!gRedis->hset(kKeyCorpsNameIndex, c)) {
						WARN_LOG("hset cid  failed! cid=%u err=%s", id, gRedis->last_error_cstr());
					}
				}
			}
			info->CopyFrom(req.infos());
			info->mutable_info()->set_corps_id(id);
			updateCorpsPos(*info);
		} else {
			ret = ErrCodeCorpNameAready;
		}
	}

	rsp.set_ret(ret);

	return ErrCodeSucc;
}


ErrCodeType CorpsData::updateCorpsInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::UpdateCorpsInfoReq, req);
	REAL_PROTOBUF_MSG(outMsg, db::UpdateCorpsInfoRsp, rsp);
	if (req.infos_size()) {
		for (int i = 0; i < req.infos_size(); ++i) {
			auto& info = req.infos(i); 
			uint32_t cid = info.info().corps_id();
			if (info.has_info()) {
				std::string basic = makeCorpsBasicKey(std::to_string(cid));
				std::string buff;
				info.info().SerializeToString(&buff);
				if (!gRedis->set(basic, buff)) {
					WARN_LOG("hset failed: %s! corpsid=%u", gRedis->last_error_cstr(), cid);
					return ErrCodeDB;
				} else {
					DEBUG_LOG("pid=%u set corps info  corps_id %u corps_lv %u", h.TargetID, cid, info.info().lv());
				}

			} 
			if (info.players_size()) {
				std::string key = makeCorpsPlayersKey(std::to_string(cid));
				unordered_map<string, string> players;
				for (int i = 0; i < info.players_size(); ++i) {
					info.players(i).SerializeToString(
							&(players[std::to_string(info.players(i).plid())])
							);
				}
				if (!gRedis->hset(key, players)) {
					WARN_LOG("hset failed: %s! corpsid=%u", gRedis->last_error_cstr(), cid);
					return ErrCodeDB;
				}
			}

			auto* item = gCSVOptionInfo.GetItem(cs::CorpsMaxSize);
			if (info.has_info() && item) {
				if (info.players_size() < item->Value) 
				{
					checkCorpsPos(info);
				} else {
					delCorpsPos(cid, true);
				}
			}


			if (info.del_ps_size()) {
				std::string key = makeCorpsPlayersKey(std::to_string(cid));
				vector<std::string> keys;
				for (int i = 0; i < info.del_ps_size(); ++i) {
					keys.push_back(to_string(info.del_ps(i)));
				}
				if (!gRedis->hdel(key, keys)) {
					WARN_LOG("del failed: %s!", gRedis->last_error_cstr());
					return ErrCodeDB;
				}
			}


			if (req.sync()) {
				uint32_t malenum = 0;
				rsp.set_cid(cid);
				db::AccountInfo accInfo;
				cs::PlayerInfo info;
				unordered_map<string, string> players;
				if (!gRedis->hgetall(makeCorpsPlayersKey(std::to_string(cid)), players)) {
					WARN_LOG("hgetall failed: %s!corpsid=%u", gRedis->last_error_cstr(), cid);
					rsp.set_cid(0);
				} else {
					for (auto& v : players) {
						info.ParseFromString(v.second);
						uint32_t plid = info.plid();
						string accountKey(kKeyPrefixPlayerData + to_string(plid));
						unordered_map<string, string> mAccount;
						auto& accInfoStr = mAccount[kAccountInfo];
						if (!gRedis->hget(accountKey, mAccount)) {
							WARN_LOG("hget failed: %s! plid=%u", gRedis->last_error_message().c_str(), plid);
							continue;
						}

						accInfo.ParseFromString(accInfoStr);
						if (accInfo.gender() == 1) {
							malenum++;
						}
					}
					rsp.set_male_num(malenum);
				}
			}
		
			if (info.has_minfos()) {
				DEBUG_LOG("info.has_minfos %u %s", cid, info.minfos().Utf8DebugString().c_str());
				std::string mon = makeCorpsMonsKey(std::to_string(cid));
				std::string buff;
				info.minfos().SerializeToString(&buff);
				if (!gRedis->set(mon, buff)) {
					WARN_LOG("hset failed: %s! corpsid=%u", gRedis->last_error_cstr(), cid);
					return ErrCodeDB;
				}
			}
		}
	}

	if (req.delkeys_size()) {
		vector<std::string> keys;
		for (int i = 0; i < req.delkeys_size(); ++i) {
			delCorpsPos(req.delkeys(i), true);
			//keys.push_back(makeCorpsBasicKey(std::to_string(req.delkeys(i))));
			keys.push_back(makeCorpsPlayersKey(std::to_string(req.delkeys(i))));
			keys.push_back(makeCorpsMonsKey(std::to_string(req.delkeys(i))));
		}

		if (!gRedis->del(keys)) {
			WARN_LOG("del failed: %s!", gRedis->last_error_cstr());
			return ErrCodeDB;
		}

		for (int i = 0; i < req.delkeys_size(); ++i) {
			std::string basic;
			bool exists = false;

			if (!gRedis->get(makeCorpsBasicKey(std::to_string(req.delkeys(i))), basic, &exists)) {
				WARN_LOG("get failed: %s!corpsid=%u", gRedis->last_error_cstr(), req.delkeys(i));
			}

			if (exists) {
				db::CorpsInfo info;
				info.mutable_info()->ParseFromString(basic);
				info.mutable_info()->set_corps_status(3);		// 解散
				info.info().SerializeToString(&basic);
				if (!gRedis->set(makeCorpsBasicKey(std::to_string(req.delkeys(i))), basic)) {
					WARN_LOG("get failed: %s!corpsid=%u", gRedis->last_error_cstr(), req.delkeys(i));
				}

			}

		}
	}


	if (req.delnames_size()) {
		vector<std::string> keys;
		for (int i = 0; i < req.delnames_size(); ++i) {
			keys.push_back(req.delnames(i));
		}
		if (!gRedis->hdel(kKeyCorpsNameIndex, keys)) {
			WARN_LOG("del failed: %s!", gRedis->last_error_cstr());
			return ErrCodeDB;
		}

		TblCorpNick tblNick(0);
		for (auto& nick : keys) {
			tblNick.DeleteNick(nick);
		}
	}


	return ErrCodeSucc;
}


ErrCodeType CorpsData::checkName(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::CheckNameReq, req);
	REAL_PROTOBUF_MSG(outMsg, db::CheckNameRsp, rsp);
	//rsp.set_cid(getCidByName(req.name()));

	bool exist;
	uint32_t corpsId = 0;
	TblCorpNick tblNick(0);
	if (tblNick.CheckNick(req.name(), corpsId, exist) != ErrCodeSucc) {
		return ErrCodeDB;
	}

	rsp.set_cid(corpsId);
	return ErrCodeSucc;
}


ErrCodeType CorpsData::updatePlayerCorpsInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::UpdatePlayerCorpInfoReq, req);
	REAL_PROTOBUF_MSG(outMsg, db::UpdatePlayerCorpInfoRsp, rsp);
	std::string key;
	uint32_t ret = 0;
	if (req.type() == db::kJoinUpd) {
		key = makePlayersJoinKey(to_string(h.TargetID));
		unordered_map<string, string> fields;
		fields[to_string(req.cid())] = to_string(req.cid());
		if (!gRedis->hset(key, fields)) {
			WARN_LOG("hset failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
			return ErrCodeDB;
		}
	} else if (req.type() == db::kJoinDel) {
		key = makePlayersJoinKey(to_string(h.TargetID));
		std::vector<std::string> del_fields{ req.cid() };
		if (!gRedis->hdel(key, del_fields)) {
			WARN_LOG("hset failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
			return ErrCodeDB;
		}
	} else if (req.type() == db::kInvUpd) {
		ret = checkCorps(h.TargetID);
		if (!ret) {
			key = makePlayersInvKey(to_string(h.TargetID));
			unordered_map<string, string> fields;
			cs::CorpsInvInfo vinfo;
			// vinfo.mutable_icorps_id()->set_corps_hid(req.cid()>>32);
			// vinfo.mutable_icorps_id()->set_corps_lid(req.cid() & 0xffffffff);
			vinfo.set_icorps_id(req.cid());
			vinfo.set_plid(req.plid());
			vinfo.SerializeToString(&(fields[to_string(req.plid())]));
			if (!gRedis->hset(key, fields)) {
				WARN_LOG("hset failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
				return ErrCodeDB;
			}
		}
	} else if (req.type() == db::kInvDel) {
		key = makePlayersInvKey(to_string(h.TargetID));
		unordered_map<string, string> fields;
		std::vector<std::string> del_fields{};
		if (!gRedis->hgetall(key, fields)) {
			WARN_LOG("hgetall failed: %s!", gRedis->last_error_cstr());
			return ErrCodeDB;
		}

		cs::CorpsInvInfo vinfo;
		for( auto& v : fields ) {
			vinfo.ParseFromString(v.second);
			if (vinfo.icorps_id() == req.del_cids(0)) {
				DEBUG_LOG("CorpsInvInfo %s %u", key.c_str(), vinfo.plid());
				del_fields.emplace_back(to_string(vinfo.plid()));
			}
		}

		if (!del_fields.empty()) {
			if (!gRedis->hdel(key, del_fields)) {
				WARN_LOG("hset failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
				return ErrCodeDB;
			}
		}

	} else if (req.type() == db::kCidUpd) {
		bool exist = false;
		key = makePlayersKey(to_string(h.TargetID));
		std::string value;
		if (!gRedis->get(key, value, &exist)) {
			WARN_LOG("exists failed: %s!", gRedis->last_error_cstr());
			return ErrCodeDB;
		}

		if (exist) {
			uint32_t val = atoi(value.c_str());
			if (val && req.cid()) {
				ret = ErrCodeAreadyInCorps;
			} else {
				if (!val && req.cid()) {
					ret = checkCorps(h.TargetID);
					if (!ret) {
						std::vector<std::string> keys {makePlayersInvKey(to_string(h.TargetID))};
						if (!gRedis->del(keys)) {
							WARN_LOG("hset failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
							return ErrCodeDB;
						}

					}
				}

			}
		}
		if (!ret) {
			if (!gRedis->set(key, to_string(req.cid()))) {
				WARN_LOG("hset failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
				return ErrCodeDB;
			}	

		}
	}

	rsp.set_cid(req.cid());
	rsp.set_type(req.type());
	rsp.set_plid(req.plid());
	rsp.set_ret(ret);
	// DEBUG_LOG("Update %s", rsp.Utf8DebugString().c_str());

	return ErrCodeSucc;
}


ErrCodeType CorpsData::delPlayerCorpsInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::DelPlayerInviteInfoReq, req);
	std::string key = makePlayersInvKey(to_string(h.TargetID));
	std::vector<std::string> del_fields;
	for (int i = 0; i < req.del_plids_size(); ++i) {
		del_fields.push_back(std::to_string(req.del_plids(i)));
	}
	if (!gRedis->hdel(key, del_fields)) {
		WARN_LOG("hset failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}

	
	return ErrCodeSucc;
}

ErrCodeType CorpsData::updateCorpsTaskInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::UpdateTaskDataReq, req);
	std::string key;
	if (req.type() == cs::kTaskActivityType) {
		key = makePlayerActivityTask(to_string(h.TargetID));
		std::unordered_map<std::string, std::string> task;
		for (int i = 0; i < req.infos_size(); ++i) {
			req.infos(i).SerializeToString(&(task[std::to_string(req.infos(i).plids(0))]));
		}

		if (!gRedis->hset(key, task)) {
			WARN_LOG("hgetall failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
			return ErrCodeDB;
		}
	}
	else if (req.type() == cs::kExAchievementType) {
		key = makePlayerExAcTask(to_string(h.TargetID));
		std::string fkey = makePlayerFExAcTask(to_string(h.TargetID));
		std::unordered_map<std::string, std::string> task;
		std::unordered_map<std::string, std::string> ftask;
		std::vector<std::string> ids;
		for (int i = 0; i < req.infos_size(); ++i) {
			req.infos(i).SerializeToString(&(task[std::to_string(req.infos(i).task_id())]));
		}

		for (int i = 0; i < req.ftasks_size(); i++) {
			ftask[std::to_string(req.ftasks(i))] = std::to_string(req.ftasks(i));	
			ids.emplace_back(std::to_string(req.ftasks(i)));
		}

		if (!task.empty()) {
			if (!gRedis->hset(key, task)) {
				WARN_LOG("hgetall failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
				return ErrCodeDB;
			}
		}

		if (!ftask.empty()) {
			if (!gRedis->hset(fkey, ftask)) {
				WARN_LOG("hgetall failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
				return ErrCodeDB;
			}

			if (!gRedis->hdel(key, ids)) {
				WARN_LOG("hdel failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
				return ErrCodeDB;
			}
		}

	}
	else {
		key = makePlayerCorpsTask(to_string(h.TargetID));
		std::unordered_map<std::string, std::string> task;
		for (int i = 0; i < req.infos_size(); ++i) {
			req.infos(i).SerializeToString(&(task[std::to_string(i)]));
		}

		if (!gRedis->hset(key, task)) {
			WARN_LOG("hgetall failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
			return ErrCodeDB;
		}
	}

	return ErrCodeSucc;	
}


ErrCodeType CorpsData::checkCorpsInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::RepeatedUint32Req, req);
	REAL_PROTOBUF_MSG(outMsg, db::CheckPlayersRsp, rsp);

	uint32_t ret = 0;
	for (uint32_t i = 0; i < (uint32_t)req.u32_size(); i++) {
		ret = checkCorps(req.u32(i));
		if (ret) {
			break;
		}
		rsp.add_plids(req.u32(i));
	}

	rsp.set_ret(ret);
	
	return ErrCodeSucc;	
}

ErrCodeType CorpsData::commonLogScan(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::ListLog, req);
	REAL_PROTOBUF_MSG(outMsg, db::ListLog, rsp);
	std::string key = buildLogKey(req.type(), req.cid(), h.TargetID);
	vector<string> vals;
	if (!gRedis->lrange(key, 0, 99, vals)) {
		WARN_LOG("lrange failed: %s! plid=%u", gRedis->last_error_message().c_str(), h.TargetID);
		return ErrCodeSucc; // 拉取离线消息失败不当成错误
	}

	for (auto& val : vals) {
		rsp.add_logs()->ParseFromString(val);
	}

	return ErrCodeSucc;	
}


ErrCodeType CorpsData::commonLogPush(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::PushLogReq, req);
	std::string key = buildLogKey(req.type(), req.cid(), h.TargetID);
	long long cnt = 0;
	vector<string> vals(1);
	req.log().SerializeToString(&(vals[0]));

	if (!gRedis->lpush(key, vals, &cnt)) {
		WARN_LOG("lpush failed: %s! plid=%u key=%s", gRedis->last_error_message().c_str(), h.TargetID, key.c_str());
		return ErrCodeDB;
	}

	if (cnt > 100) { // 最多保存100条
		gRedis->ltrim(key, 0, 99);
	}

	return ErrCodeSucc;	
}


std::string CorpsData::buildLogKey(uint32_t type, uint64_t cid, uint32_t target)
{
	if (type == cs::kLogCorps) {
		return makeCorpsLogsKey(std::to_string(cid));
	}  else if (type == cs::kLogCapRes) {
		return makeCapResLogsKey(std::to_string(cid));
	}
	return "";	
}

void CorpsData::getCorpsInfo(uint32_t key, db::CorpsInfo& info, bool check)
{
	std::string pkey = std::to_string(key);	
	std::string basic;
	bool exists = false;
	if (!gRedis->get(makeCorpsBasicKey(pkey), basic, &exists)) {
		WARN_LOG("hgetall failed: %s!corpsid=%u", gRedis->last_error_cstr(), key);
	}
	
	if (!exists) {
		//info.mutable_info()->set_corps_hid(key>>32);
		//info.mutable_info()->set_corps_lid(key & 0xffffffff);
		info.mutable_info()->set_corps_id(key);
		return ;
	}

	if (check) {
		db::CorpsInfo tempinfo;
		tempinfo.mutable_info()->ParseFromString(basic);
		DEBUG_LOG("getCorpsInfo %s", tempinfo.Utf8DebugString().c_str());
		if (tempinfo.mutable_info()->corps_status() == 3) {
			info.mutable_info()->set_corps_id(key);
			return ;
		}
	}

	info.mutable_info()->ParseFromString(basic);

	tm tmNow(*GetNowTm());
	if (info.info().corps_new_time() > mktime(&tmNow)) {
		info.mutable_info()->set_corps_star(0);
	} else if (info.info().active_time() < mktime(&tmNow)) {
		info.mutable_info()->set_corps_star(1);
	}

	unordered_map<string, string> players;
	if (!gRedis->hgetall(makeCorpsPlayersKey(pkey), players)) {
		WARN_LOG("hgetall failed: %s!corpsid=%u", gRedis->last_error_cstr(), key);
		return;
	}


	uint32_t i = 0;
	uint32_t malenum = 0;
	db::AccountInfo accInfo;

	for (auto& v : players) {
		info.add_players()->ParseFromString(v.second);
		uint32_t plid = info.players(i).plid();
		string accountKey(kKeyPrefixPlayerData + to_string(plid));
		unordered_map<string, string> mAccount;
		auto& accInfoStr = mAccount[kAccountInfo];
		i++;
		if (!gRedis->hget(accountKey, mAccount)) {
			WARN_LOG("hget failed: %s! plid=%u", gRedis->last_error_message().c_str(), plid);
			continue;
		}

		accInfo.ParseFromString(accInfoStr);
		if (accInfo.gender() == 1) {
			malenum++;
		}
	}
	info.mutable_info()->set_male_num(malenum);

	std::string mon = makeCorpsMonsKey(pkey);
	std::string buff;
	if (!gRedis->get(mon, buff, &exists)) {
		WARN_LOG("hset failed: %s! corpsid=%u", gRedis->last_error_cstr(), key);
		return;
	}

	info.mutable_minfos()->ParseFromString(buff);


	std::unordered_map<string, string> skills;
	if (!gRedis->hgetall(makeCorpsSkillsKey(pkey), skills)) {
		WARN_LOG("hset failed: %s! corpsid=%u", gRedis->last_error_cstr(), key);
		return;
	}

	for(auto& v : skills) {
		auto* item = info.add_cskills();
		item->ParseFromString(v.second);
	}
	//DEBUG_LOG("getCorpsInfo %u %s", key, info.Utf8DebugString().c_str());

}

ErrCodeType CorpsData::updateCorpsSkills(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::PlayerCorpsSkillInfo, req);
	std::string pkey = makeCorpsSkillsKey(std::to_string(req.cid()));

	unordered_map<string, string> skills;
	for (int i = 0; i < req.infos_size(); i++) {
		auto& str = skills[to_string(req.infos(i).skillid())];
		req.infos(i).SerializeToString(&str);
	}

	if (!gRedis->hset(pkey, skills)) {
		WARN_LOG("hgetall failed: %s!corpsid=%u", gRedis->last_error_cstr(), req.cid());
		return ErrCodeDB;
	}


	return ErrCodeSucc;	
}



uint32_t CorpsData::GetCorpsOwnerId(uint32_t key)
{
	std::string pkey = std::to_string(key);
	std::string basic;
	bool exists = false;
	if (!gRedis->get(makeCorpsBasicKey(pkey), basic, &exists)) {
		WARN_LOG("hgetall failed: %s!corpsid=%u", gRedis->last_error_cstr(), key);
		return 0;
	}

	if (!exists) {
		return 0;
	}

	db::CorpsInfo info;
	info.mutable_info()->ParseFromString(basic);
	if (info.info().corps_status() == 3) {
		return 0;
	}
	return info.info().owner_plid();
}

uint32_t CorpsData::getCidByName(const std::string& name)
{
	bool exist = false;
	if (!gRedis->hexists(kKeyCorpsNameIndex, name, exist)) {
		WARN_LOG("hexist cid  failed! cname=%s err=%s",
				name.c_str(), gRedis->last_error_cstr());
		return 0;
	} 

	if (exist) {
		unordered_map<string, string> m;
		m[name];
		if (!gRedis->hget(kKeyCorpsNameIndex, m)) {
			WARN_LOG("hget cid  failed! cname=%s err=%s",
					name.c_str(), gRedis->last_error_cstr());
			return 0;
		}
		return atoi(m[name].c_str());
	}

	return 0;
}

uint32_t CorpsData::checkCorps(uint32_t plid)
{
	string key1 = makePlayersKey(to_string(plid));

	std::string temp;
	bool exists;

	if (!gRedis->get(key1, temp, &exists)) {
		WARN_LOG("exists failed: %s!", gRedis->last_error_cstr());
		return ErrCodeDB;
	}

	if (exists) {
		uint64_t val = strtol(temp.c_str(), nullptr, 10);
		if (val) {
			return ErrCodeAreadyInCorps;
		}
	}

	unordered_map<string, string> m;
	string key(kKeyPrefixPlayerData + to_string(plid));
	auto& playerAttrStr = m[kPlayerAttr];
	if (!gRedis->hget(key, m)) {
		WARN_LOG("hget failed: %s! plid=%u", gRedis->last_error_cstr(), plid);
		return ErrCodeDB;
	}

	cs::PlayerAttr playerAttr;
	playerAttr.ParseFromString(playerAttrStr);

	auto* item = gCSVOptionInfo.GetItem(cs::CorpsLvLimit);
	if (item && playerAttr.lv() < item->Value) {
		return ErrCodePlayerLevelTooLow;
	}

	key = kKeyPrefixPlayerAcInfo + to_string(plid);
	unordered_map<string, string> fields;
	auto& str = fields[to_string(cs::kCorpsQuitTime)];

	if (!gRedis->hget(key, fields)) {
		WARN_LOG("hget failed: %s! plid=%u ", gRedis->last_error_message().c_str(), plid);
		return ErrCodeDB;
	}

	tm tmNow(*GetNowTm());
	cs::PlayerAcInfo acinfo;
	acinfo.ParseFromString(str);

	uint32_t time = acinfo.val();
	item = gCSVOptionInfo.GetItem(cs::CorpsQutiColdTm);
	if (item) {
		time += item->Value;
	}

	if (time > mktime(&tmNow)) {
		return ErrCodeQuitColdTmLimit;
	}

	//PLAYERDATA
#if 0
	unordered_map<string, string> mp;
	auto& digivice = mp[kPlayerDigivice];
	key = (kKeyPrefixPlayerData + to_string(plid));
	if (!gRedis->hget(key, mp)) {
		WARN_LOG("hget failed: %s! plid=%u ", gRedis->last_error_message().c_str(), plid);
		return ErrCodeDB;
	}
#endif

	/*
	cs::DigiviceInfo info;
	info.ParseFromString(digivice);
	for (int i = 0; i < info.digivice_stage_size(); ++i) {
		//硬代码
		if (info.digivice_stage(i).id() == 3
				&& info.digivice_stage(i).state() != 2) {
			return ErrCodeFuncLimited;
		}
	}
	*/

	return 0;	
}

ErrCodeType CorpsData::ListUint32Hash(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::Uint32HashList, req);
	REAL_PROTOBUF_MSG(outMsg, db::Uint32HashList, rsp);
	string key = buildListKey(req.key());
	if (key != "") {
		unordered_map<string, string> fields;
		if (!gRedis->hgetall(key, fields)) {
			WARN_LOG("hget failed: %s! ", gRedis->last_error_message().c_str());
			return ErrCodeDB;
		}

		for (auto& v : fields) {
			uint32_t t = atoi(v.first.c_str());
			if (t) {
				auto* field = rsp.add_fields();
				field->set_key(t);
				field->set_val(v.second);
			}
		}
	}

	rsp.mutable_key()->CopyFrom(req.key());
	return ErrCodeSucc;
}


ErrCodeType CorpsData::SetUint32Hash(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::Uint32HashUnitSet, req);
	string key = buildListKey(req.key());
	if (key != "") {
		unordered_map<string, string> fields;
		if (req.fields_size()) {
			for (int i = 0; i < req.fields_size(); i++) {
				fields[ to_string(req.fields(i).key()) ] = req.fields(i).val();
			}
			if (!gRedis->hset(key, fields)) {
				WARN_LOG("hset failed: %s! ", gRedis->last_error_message().c_str());
				return ErrCodeDB;
			}

		}
		vector<string> delListItem;
		if (req.delkeys_size()) {
			for (int i = 0; i < req.delkeys_size(); ++i) {
				delListItem.emplace_back(to_string(req.delkeys(i)));
			}
			if (!gRedis->hdel(key, delListItem)) {
				WARN_LOG("hget failed: %s! ", gRedis->last_error_message().c_str());
				return ErrCodeDB;
			}

		}
	}

	return ErrCodeSucc;
}

#if 0
ErrCodeType CorpsData::RegisterDataCom(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::RegisterData, req);
	REAL_PROTOBUF_MSG(outMsg, db::RegisterData, rsp);
	rsp.CopyFrom(req);
	if (req.type() == db::kCorpsDataType) {
		uint32_t hkey = req.args(0);
		bool exits = false;

		std::unordered_map<std::string, std::string> data;
#if 0
		std::string str;
		std::string basic(makeCorpsBasicKey("10009"));
		if (!gRedis->get(basic, str)) {
			WARN_LOG("hset failed: %s! ", gRedis->last_error_message().c_str());
			return ErrCodeDB;
		}
		cs::CorpsBasicInfo b;
		b.ParseFromString(str);
		b.set_corps_id(10009);
		b.SerializeToString(&str);
		if (!gRedis->set(basic, str)) {
			WARN_LOG("hset failed: %s! ", gRedis->last_error_message().c_str());
			return ErrCodeDB;
		}
#endif

		if (!gRedis->hexists(kKeyCorpsUniKey, to_string(hkey), exits)) {
			WARN_LOG("hexists failed: %s! ", gRedis->last_error_message().c_str());
			return ErrCodeDB;
		}

		if (!exits) {
			std::unordered_map<std::string, std::string> data;
			auto& str = data[to_string(req.args(0))];
			req.mutable_info()->SerializeToString(&str);
			if (!gRedis->hset(kKeyCorpsUniKey, data)) {
				WARN_LOG("hset failed: %s! ", gRedis->last_error_message().c_str());
				return ErrCodeDB;
			}
		} else {
			std::unordered_map<std::string, std::string> data;
			auto& str = data[to_string(req.args(0))];
			if (!gRedis->hget(kKeyCorpsUniKey, data)) {
				WARN_LOG("hset failed: %s! ", gRedis->last_error_message().c_str());
				return ErrCodeDB;
			}
			rsp.mutable_info()->ParseFromString(str);
		}
	}

	return ErrCodeSucc;
}


ErrCodeType CorpsData::FreeDataCom(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::FreeData, req);
	if (req.type() == db::kCorpsDataType) {
	}


	return ErrCodeSucc;
}


ErrCodeType CorpsData::GetRegisterDataList(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::RepeatedUint32Req, req);
	REAL_PROTOBUF_MSG(outMsg, db::GetRegisterDataRsp, rsp);

	std::unordered_map<std::string, std::string> data;
	for (int i = 0; i < req.u32_size(); ++i) {
		data[std::to_string(req.u32(i))];
	}

	if (!gRedis->hget(kKeyCorpsUniKey, data)) {
		WARN_LOG("hset failed: %s! ", gRedis->last_error_message().c_str());
		return ErrCodeDB;
	}

	std::unordered_map<uint32_t, db::DataInfo*> maps;
	db::ServerInfo info;
	for (auto v : data) {
		info.ParseFromString(v.second);
		if (info.type()) {
			auto iter = maps.find(info.serverid());
			if (iter == maps.end()) {
				auto* sinfo = rsp.add_infos();
				sinfo->mutable_info()->CopyFrom(info);
				sinfo->add_cids(atoi(v.first.c_str()));
				maps.emplace(info.serverid(), sinfo);
			} else {
				iter->second->add_cids(atoi(v.first.c_str()));
			}
		} else {
			rsp.add_cids(atoi(v.first.c_str()));
		}
	}

	DEBUG_LOG("GetRegisterDataList %s", rsp.Utf8DebugString().c_str());

	/*
	if (!exits) {
		std::unordered_map<std::string, std::string> data;
		auto& str = data[to_string(req.args(0))];
		req.mutable_info()->SerializeToString(&str);
		if (!gRedis->hset(kKeyCorpsUniKey, data)) {
			WARN_LOG("hset failed: %s! ", gRedis->last_error_message().c_str());
			return ErrCodeDB;
		}
	}*/
	return ErrCodeSucc;	
}

#endif

ErrCodeType CorpsData::InviteRefuse(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::RepeatedUint32Req, req);

	for (int i = 0; i < req.u32_size(); ++i) {
		refuseInvite(h.TargetID, req.u32(i));
	}

	return ErrCodeSucc;	
}

ErrCodeType CorpsData::GetAllCorpsID(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(outMsg, db::RepeatedUint32Req, rsp);
	vector<string> values;
	if (!gRedis->hvals(kKeyCorpsNameIndex, values)) {
		WARN_LOG("hval failed! err=%s", gRedis->last_error_cstr());
	}
	for (auto val : values) {
		rsp.add_u32(atoi(val.c_str()));
	} 
	return ErrCodeSucc;	
}


std::string CorpsData::buildListKey(const db::Keyformat& key)
{
	if (key.type() == db::kCorpsJoinList) {
		return "CPSJ{" + key.args(0) + "}";
	}
	return "";	
}


void CorpsData::refuseInvite(uint32_t plid, uint32_t cid)
{
	std::string pkey = std::to_string(cid);	

	unordered_map<string, string> players;
	auto& iter  = players[to_string(plid)];

	if (!gRedis->hget(makeCorpsPlayersKey(pkey), players)) {
		WARN_LOG("hgetall failed: %s!corpsid=%u", gRedis->last_error_cstr(), cid);
		return;
	}
	cs::PlayerInfo info;
	info.ParseFromString(iter);
	if (info.plid() == plid && info.status() == cs::kInvite) {
		info.set_status(cs::kRefuse);
		info.SerializeToString(&iter);

		if (!gRedis->hset(makeCorpsPlayersKey(pkey), players)) {
			WARN_LOG("hgetall failed: %s!corpsid=%u", gRedis->last_error_cstr(), cid);
			return;
		}
	}
}


void CorpsData::getNeighborCorps(const cs::GeoPos& pos, db::ListCorpInfoRsp& rsp, uint32_t channel)
{
	uint32_t kGeoNeighborCnt = 5;

	string keyCountry;
	string keyProvince;
	string keyCity;
	
	std::string sch = to_string(channel);
	keyCountry = makeCorpsAllKey(sch);
	keyProvince = makeCorpsProvinceKey(to_string(pos.province_code()), sch);
	keyCity = makeCorpsCityKey(to_string(pos.province_code()), to_string(pos.city_code()), sch);

	vector<string> vals;

	DEBUG_LOG("CorpsData::getNeighborCorps %s %s %s", keyCountry.c_str(), keyProvince.c_str(), keyCity.c_str());

	if (!gRedis->srandmembers(keyCity, kGeoNeighborCnt, vals)) {
		WARN_LOG("srandmembers failed: %s!", gRedis->last_error_cstr());
		return;
	}

	if (vals.size() < kGeoNeighborCnt) {
		if (!gRedis->srandmembers(keyProvince, kGeoNeighborCnt, vals)) {
			WARN_LOG("srandmembers failed: %s!", gRedis->last_error_cstr());
			return;
		}
	}


	if (vals.size() < kGeoNeighborCnt) {
		if (!gRedis->srandmembers(keyCountry, kGeoNeighborCnt, vals)) {
			WARN_LOG("srandmembers failed: %s!", gRedis->last_error_cstr());
			return;
		}
	}

	for (auto& ret : vals) {
		getCorpsInfo(atoi(ret.c_str()), *(rsp.add_infos()), true);
	}
	//DEBUG_LOG("CorpsData::getNeighborCorps %s", rsp.Utf8DebugString().c_str());

	for (int i = 0; i < rsp.infos_size(); i++) {
		if (!rsp.infos(i).info().owner_plid()) {
			delCorpsPos( rsp.infos(i).info().corps_id(), true);
		}
	}
	
}

void CorpsData::checkCorpsPos(const db::CorpsInfo& info)
{
	if (!info.info().step_time() || info.info().corps_status() == 3) {
		return;
	}

	db::CorpsInfo rinfo;
	getCorpsInfo(info.info().corps_id(), rinfo);
	LOCKCORPSPOSINFO;

	string keyCountry;
	string keyCountry1;
	string keyProvince;
	string keyProvince1;
	string keyCity;
	string keyCity1;
	vector<string> ckey { to_string(info.info().corps_id()) };

	auto& pos = rinfo.info().pos();
	std::string channel = std::to_string(rinfo.info().region());
	if (info.info().pos().province_code() != pos.province_code()) {
		if (pos.province_code()) {
			keyProvince = makeCorpsProvinceKey(to_string(pos.province_code()), "0");
			keyProvince1 = makeCorpsProvinceKey(to_string(pos.province_code()), channel);
			if (!gRedis->srem(keyProvince, ckey)) {
				WARN_LOG("srandmembers failed: %s!", gRedis->last_error_cstr());
			}
			if (!gRedis->srem(keyProvince1, ckey)) {
				WARN_LOG("srandmembers failed: %s!", gRedis->last_error_cstr());
			}
		}
	}

	if (info.info().pos().city_code() != pos.city_code()) {
		if (pos.city_code()) {
			keyCity = makeCorpsCityKey(to_string(pos.province_code()), to_string(pos.city_code()), "0");
			keyCity1 = makeCorpsCityKey(to_string(pos.province_code()), to_string(pos.city_code()), channel);
			if (!gRedis->srem(keyCity, ckey)) {
				WARN_LOG("srandmembers failed: %s!", gRedis->last_error_cstr());
			}
			if (!gRedis->srem(keyCity1, ckey)) {
				WARN_LOG("srandmembers failed: %s!", gRedis->last_error_cstr());
			}

		}
	}

	UNLOCKCORPSPOSINFO;

	updateCorpsPos(info);

	// 成立之后
	/*
	DEBUG_LOG("checkCorpsPos %u %u", info.info().corps_id(), IsExistsCorpsPos(info.info().corps_id()));
	if (!IsExistsCorpsPos(info.info().corps_id())) {
		updateCorpsPos(info);
		return;
	}*/

	/*
	auto& pos = rinfo.info().pos();
	if (info.info().pos().province_code() != pos.province_code()
			|| info.info().pos().city_code() != pos.city_code()) {
		delCorpsPos(info.info().corps_id(), false);
		updateCorpsPos(info);
	}*/


}

void CorpsData::delCorpsPos(uint32_t key, bool remove)
{
	LOCKCORPSPOSINFO;

	string keyCountry;
	string keyProvince;
	string keyCity;

	string keyCountry1;
	string keyProvince1;
	string keyCity1;

	vector<string> ckey { to_string(key) };

	db::CorpsInfo info;
	getCorpsInfo(key, info);

	std::string channel = std::to_string(info.info().region());

	if (remove) {
		keyCountry = makeCorpsAllKey("0");
		if (!gRedis->srem(keyCountry, ckey)) {
			WARN_LOG("srandmembers failed: %s!", gRedis->last_error_cstr());
		}

		keyCountry1 = makeCorpsAllKey(channel);
		if (!gRedis->srem(keyCountry1, ckey)) {
			WARN_LOG("srandmembers failed: %s!", gRedis->last_error_cstr());
		}

	}

	const cs::GeoPos& pos = info.info().pos();

	if (pos.province_code()) {
		keyProvince = makeCorpsProvinceKey(to_string(pos.province_code()), "0");
		if (!gRedis->srem(keyProvince, ckey)) {
			WARN_LOG("srandmembers failed: %s!", gRedis->last_error_cstr());
		}
		keyProvince1 = makeCorpsProvinceKey(to_string(pos.province_code()), channel);
		if (!gRedis->srem(keyProvince1, ckey)) {
			WARN_LOG("srandmembers failed: %s!", gRedis->last_error_cstr());
		}

	}

	if (pos.city_code()) {
		keyCity = makeCorpsCityKey(to_string(pos.province_code()), to_string(pos.city_code()), "0");
		if (!gRedis->srem(keyCity, ckey)) {
			WARN_LOG("srandmembers failed: %s!", gRedis->last_error_cstr());
		}
		keyCity1 = makeCorpsCityKey(to_string(pos.province_code()), to_string(pos.city_code()), channel);
		if (!gRedis->srem(keyCity1, ckey)) {
			WARN_LOG("srandmembers failed: %s!", gRedis->last_error_cstr());
		}

	}

	UNLOCKCORPSPOSINFO;
}

void CorpsData::updateCorpsPos(const db::CorpsInfo& info)
{
	LOCKCORPSPOSINFO;

	string keyCountry;
	string keyProvince;
	string keyCity;

	string keyCountry1;
	string keyProvince1;
	string keyCity1;

	string cid = to_string(info.info().corps_id());
	vector<string> ckey { cid };

	std::string channel = std::to_string(info.info().region());
	const cs::GeoPos& pos = info.info().pos();

	if (pos.province_code()) {
		keyProvince = makeCorpsProvinceKey(to_string(pos.province_code()), "0");
		if (!gRedis->sadd(keyProvince, ckey)) {
			WARN_LOG("srandmembers failed: %s!", gRedis->last_error_cstr());
		}

		keyProvince1 = makeCorpsProvinceKey(to_string(pos.province_code()), channel);
		if (!gRedis->sadd(keyProvince1, ckey)) {
			WARN_LOG("srandmembers failed: %s!", gRedis->last_error_cstr());
		}
	}

	if (pos.city_code()) {
		keyCity = makeCorpsCityKey(to_string(pos.province_code()), to_string(pos.city_code()), "0");
		if (!gRedis->sadd(keyCity, ckey)) {
			WARN_LOG("srandmembers failed: %s!", gRedis->last_error_cstr());
		}

		keyCity1 = makeCorpsProvinceKey(to_string(pos.province_code()), channel);
		if (!gRedis->sadd(keyCity1, ckey)) {
			WARN_LOG("srandmembers failed: %s!", gRedis->last_error_cstr());
		}

	}

	keyCountry = makeCorpsAllKey("0");
	if (!gRedis->sadd(keyCountry, ckey)) {
		WARN_LOG("sadd failed: %s!", gRedis->last_error_cstr());
	}

	keyCountry1 = makeCorpsAllKey(channel);
	if (!gRedis->sadd(keyCountry1, ckey)) {
		WARN_LOG("sadd failed: %s!", gRedis->last_error_cstr());
	}

	DEBUG_LOG("========== %s %s %s %u", keyCountry1.c_str(), keyProvince1.c_str(), keyCity1.c_str(), info.info().region());

	UNLOCKCORPSPOSINFO;
}

bool CorpsData::IsExistsCorpsPos(uint32_t key)
{
	bool member = false;	
	if (!gRedis->sismember(makeCorpsAllKey("0"), to_string(key), member)) {
		WARN_LOG("sismember failed: %s!", gRedis->last_error_cstr());
	}
	return member;
}


