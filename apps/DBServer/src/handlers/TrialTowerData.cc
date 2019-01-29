/*
 * TrialTowerData.cpp
 *
 *  Created on: 2018年8月9日
 *      Author: zhangxuan
 */

#include <vector>
#include <sstream>
#include <unordered_map>
#include "../proto/SvrProtoID.pb.h"
#include "../proto/db.pb.h"
#include "../proto/CommonMsg.pb.h"
#include "../proto/CSCoreMsg.pb.h"
#include "../core/dispatcher.h"
#include "../core/redis_client.h"

#include "KeyPrefixDef.h"
#include "TrialTowerData.h"

using namespace std;
const string TrialTowerData::kTrialTowerDataKeyPrefix("TW");
const string TrialTowerData::kTrialTowerDataKey(kTrialTowerDataKeyPrefix + "Data");

static TrialTowerData trialTowerData;

TrialTowerData::TrialTowerData()
{
	gMsgDispatcher.RegisterHandler(DBProtoTrialTowerPullBestPassRecords, *this, &TrialTowerData::pullBestPassRecords,
			new cs::CSUint32Req , new cs::TrialTowerPassRecords);
	gMsgDispatcher.RegisterHandler(DBProtoUpdateRecords, *this, &TrialTowerData::tryUpdateRecords,
			new db::TrialTowerRecordsUpdateReq, nullptr);
}

ErrCodeType TrialTowerData::pullBestPassRecords(const SSProtoHead& h, google::protobuf::Message* inMsg,
									google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, cs::CSUint32Req, req);
	REAL_PROTOBUF_MSG(outMsg, cs::TrialTowerPassRecords, rsp);
	//RedisClient::ExpirationTime expTm(false, 86400 * 180); // 强制180天过期
	DEBUG_LOG("pull record. plid=%u, gameRegion=%u", h.PlayerID, req.u32());
	std::unordered_map<string, string> fields;
	if(!gRedis->hgetall(makeTrialTowerRecordsDataKey(req.u32()) , fields)){
		DEBUG_LOG("trial data get failed! plid=%u origProto=%u err=%s", h.PlayerID, h.OrigProtoID,
					gRedis->last_error_message().c_str());
		return ErrCodeDB;
	}
	for(auto& it : fields) {
		auto record = rsp.add_records();
		record->ParseFromString(it.second);
	}
	DEBUG_LOG("player pull records data. plid=%u,  gameRegion=%u", h.PlayerID, req.u32());
	return ErrCodeSucc;
}

// 只负责写, 不用关注返回
ErrCodeType TrialTowerData::tryUpdateRecords(const SSProtoHead& h, google::protobuf::Message* inMsg,
									google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::TrialTowerRecordsUpdateReq, req);
	//DEBUG_LOG("update record. plid=%u, floor=%u, round=%u, gameRegion=%u", h.PlayerID, req.floor(), req.round(), req.game_region());
	unordered_map<string, string> fields;
	fields[std::to_string(req.floor())];
	if(!gRedis->hget(makeTrialTowerRecordsDataKey(req.game_region()), fields)){
		DEBUG_LOG("trial data get failed! plid=%u origProto=%u err=%s", h.PlayerID, h.OrigProtoID,
					gRedis->last_error_message().c_str());
		return ErrCodeDB;
	}
	bool needUpdate = false;
	if(!fields[std::to_string(req.floor())].empty()) {
		cs::TrialTowerPassRecords_Record record;
		record.ParseFromString(fields[std::to_string(req.floor())]);
		if(req.round() <(uint32_t) record.pass_round()) {
			needUpdate = true;
		}
	} else {
		needUpdate = true;
	}
	if(needUpdate) {
		cs::TrialTowerPassRecords_Record record;
		record.set_pid(h.PlayerID);
		record.set_floor(req.floor());
		record.set_pass_round(req.round());
		record.SerializeToString(&fields[std::to_string(req.floor())]);
		if (!gRedis->hset(makeTrialTowerRecordsDataKey(req.game_region()), fields)) {
			DEBUG_LOG("Set failed! plid=%u origProto=%u err=%s", h.PlayerID, h.OrigProtoID,
						gRedis->last_error_message().c_str());
			return ErrCodeDB;
		}
		DEBUG_LOG("update trial tower best record. plid=%u, floor=%u, round=%u, region=%u", h.PlayerID, req.floor(), req.round(), req.game_region());
	}

	return ErrCodeSucc;
}
