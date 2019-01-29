#include <string>
#include <sstream>
#include "BtlReplay.h"
#include "../core/dispatcher.h"
#include "../core/redis_client.h"
#include "../core/MysqlProxy.h"
#include "../proto/Battle.pb.h"
#include "../proto/SvrProtoID.pb.h"
#include <StringUtils.h>
#include "../tables/tbl_replay.h"
#include "../core/MysqlProxy.h"
#include <serverbench/benchapi.hpp>
#include "../global.h"
#include "../log.h"

#define REPLAY_CNT_MAX		30

BtlReplay gBtlReplay;

BtlReplay::BtlReplay()
{
	gMsgDispatcher.RegisterHandler(DBProtoBtlReplayAdd, *this, &BtlReplay::addReplay, new cs::BtlReplayInfo);
	gMsgDispatcher.RegisterHandler(DBProtoBtlReplayList, *this, &BtlReplay::getReplayList, new cs::BtlReplayGet, new cs::BtlReplayList);
	gMsgDispatcher.RegisterHandler(DBProtoBtlReplayGet, *this, &BtlReplay::getReplay, new cs::BtlReplayGet, new cs::BtlReplayInfo);
	ignoreVer_ = false;
}

void BtlReplay::Init()
{
	dbPrefix_ = MysqlProxy::Instance().GetKv("btlReplayDB");
	tblPrefix_ = "btl_replay";
	ignoreVer_ = config_get_intval("btl_replay_ignore_ver") == 1;
}

void BtlReplay::RemoveReplay(uint32_t uid, const std::string& key)
{
	TblReplay tblReplay(uid, dbPrefix_.c_str(), tblPrefix_.c_str());
	tblReplay.Del(key);
}

ErrCodeType BtlReplay::addReplay(const SSProtoHead & h, google::protobuf::Message * inMsg, google::protobuf::Message * outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, cs::BtlReplayInfo, req);
	
	if (req.type() == cs::BtlTypeArenaPvPImage) {
		//auto& uKey = req.key();
		//std::unordered_map<std::string, std::string> fields;
		//req.replay().SerializeToString(&fields["data"]);
		//std::stringstream ss;
		//ss << req.cnt();
		//fields["cnt"] = ss.str();
		//gRedisBattle->hset(uKey, fields);
		auto data = req.replay().SerializePartialAsString();
		auto arr = Split(req.key(), '_');
		if (arr.size() > 2) {
			auto uid = atoi(arr[2].c_str());
			TblReplay tblReplay(uid, dbPrefix_.c_str(), tblPrefix_.c_str());
			tblReplay.Add(req.key(), req.ver(), data, req.cnt());
		}
		else {
			ERROR_LOG("invlaid replay key formate:%s", req.key().c_str());
		}
	}
	else {
		std::string data;
		req.replay().SerializeToString(&data);
		for (auto it : req.replay().enter()) {
			AddReplay(it.req().my_info().player_id(), data, req.ver());
		}
	}
	return ErrCodeType::ErrCodeSucc;
}

void BtlReplay::AddReplay(uint32_t pid, std::string& data, const std::string& ver)
{
	std::string uKey = GetBtlUserKey(pid);
	std::vector<std::string> list;
	gRedisBattle->smembers(uKey, list);
	std::vector<std::string> rmList;
	std::map<uint32_t, std::string> rMap;
	//移除超过上限的早期记录
	if (list.size() >= REPLAY_CNT_MAX) {
		for (auto& it : list) {
			auto arr = Split(it, '_');
			if (arr.size() >= 4) {
				time_t cTime = atoi(arr[3].c_str());
				rMap.emplace(cTime, it);
			}
			else {
				if (rmList.size() <= list.size() - REPLAY_CNT_MAX) {
					rmList.emplace_back(it);
				}
				else {
					break;
				}
			}
		}
	}
	for (auto it : rMap) {
		if (rmList.size() <= list.size() - REPLAY_CNT_MAX) {
			rmList.emplace_back(it.second);
		}
		else {
			break;
		}
	}
	
	if (rmList.size() > 0) {
		//gRedisBattle->del(rmList);
		gRedisBattle->srem(uKey, rmList);
	}
	std::string	key = GetReplayId(pid);
	TblReplay tblReplay(pid, dbPrefix_.c_str(), tblPrefix_.c_str());
	tblReplay.Add(key, ver, data);
	for (auto& k : rmList) {
		tblReplay.Del(k);
	}

	//std::unordered_map<std::string, std::string> fields;
	//fields["data"] = std::move(data);
	//gRedisBattle->hset(key, fields);
	//data = std::move(fields["data"]);

	std::vector<std::string> vecKey = { key };
	gRedisBattle->sadd(uKey, vecKey);
}

ErrCodeType BtlReplay::getReplay(const SSProtoHead & h, google::protobuf::Message * inMsg, google::protobuf::Message * outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, cs::BtlReplayGet, req);
	REAL_PROTOBUF_MSG(outMsg, cs::BtlReplayInfo, replay);
	//std::unordered_map<std::string, std::string> fields;
	//std::string& data = fields["data"];
	//gRedisBattle->hget(req.key(), fields);
	DEBUG_LOG("getReplay key:%s", req.key().c_str());
	std::string data;
	auto arr = Split(req.key(), '_');
	if (arr.size() > 2) {
		auto uid = atoi(arr[2].c_str());
		TblReplay tblReplay(uid, dbPrefix_.c_str(), tblPrefix_.c_str());
		auto ret = tblReplay.Get(req.key(), req.ver(), data, ignoreVer_);
		if (ret != 0) {
			return ErrCodeType::ErrcodeReplayOutOfData;
		}
	}

	if (data.empty()) {
		return ErrCodeType::ErrcodeInvalidReplay;
	}
	else {
		replay.mutable_replay()->ParseFromString(data);
		return ErrCodeType::ErrCodeSucc;
	}
}

ErrCodeType BtlReplay::getReplayList(const SSProtoHead & h, google::protobuf::Message * inMsg, google::protobuf::Message * outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, cs::BtlReplayGet, req);
	REAL_PROTOBUF_MSG(outMsg, cs::BtlReplayList, replay);
	
	std::string	key = GetBtlUserKey(atoi(req.key().c_str()));
	std::vector<std::string> list;
	gRedisBattle->smembers(key, list);
	for (auto it : list) {
		auto item = replay.add_key();
		*item = it;
	}
	return ErrCodeType::ErrCodeSucc;
}

std::string BtlReplay::GetBtlUserKey(uint32_t uid)
{
	std::stringstream ss;
	ss << "btl_replay_" << uid;
	return ss.str();
}

std::string BtlReplay::GetReplayId(uint32_t uid)
{
	time_t now;
	time(&now);
	std::stringstream ss;
	ss << "btl_replay_" << uid << "_" << now << "_" << (rand() % 100);
	return ss.str();
}


