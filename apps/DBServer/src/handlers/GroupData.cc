/*
 * GroupData.cpp
 *
 *  Created on: 2017年4月11日
 *      Author: zhangxuan
 */

#include <vector>
#include <sstream>
#include <unordered_map>
#include "../proto/SvrProtoID.pb.h"
#include "../proto/db.pb.h"
#include "../core/dispatcher.h"
#include "../core/redis_client.h"
#include "GroupData.h"

using namespace std;
const string GroupData::kGroupDataKeyPrefix("GD");
const string GroupData::kGroupDataKey(kGroupDataKeyPrefix + "Data");

static GroupData groupData;

GroupData::GroupData()
{
	gMsgDispatcher.RegisterHandler(DBProtoGetGroupData, *this, &GroupData::getGroupData,
			new db::GetGroupDataReq, new db::GetGroupDataRsp);
	gMsgDispatcher.RegisterHandler(DBProtoSetGroupData, *this, &GroupData::setGroupData,
			new db::SetGroupDataReq, nullptr);
}

ErrCodeType GroupData::getGroupData(const SSProtoHead& h, google::protobuf::Message* inMsg,
									google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::GetGroupDataReq, req);
	REAL_PROTOBUF_MSG(outMsg, db::GetGroupDataRsp, rsp);
	string key = makeGroupKey(req.group_id());
	//RedisClient::ExpirationTime expTm(false, 86400 * 180); // 强制180天过期
	unordered_map<string, string> m;
	string& dataStr = m[key];
	if (!gRedis->hget(kGroupDataKey, m)) {
		DEBUG_LOG("group data get failed! plid=%u origProto=%u err=%s", h.PlayerID, h.OrigProtoID,
					gRedis->last_error_message().c_str());
		return ErrCodeDB;
	}
	if (!dataStr.empty()) {
		rsp.set_group_id(req.group_id());
		rsp.mutable_group_data()->ParseFromString(dataStr);
	} else {
		rsp.set_group_id(req.group_id());
	}
	return ErrCodeSucc;
}

// 只负责写, 不用关注返回
ErrCodeType GroupData::setGroupData(const SSProtoHead& h, google::protobuf::Message* inMsg,
									google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::SetGroupDataReq, req);
	if (req.group_id() == 0)
		return ErrCodeGame;

	string key = makeGroupKey(req.group_id());

	if (req.has_group_data()) {
		unordered_map<string, string> m;

		string& dataStr = m[key];
		req.group_data().SerializeToString(&dataStr);
		if (!gRedis->hset(kGroupDataKey, m)) {
			DEBUG_LOG("Set failed! plid=%u origProto=%u err=%s", h.PlayerID, h.OrigProtoID,
						gRedis->last_error_message().c_str());
			return ErrCodeDB;
		}
	} else {
		vector<string> fields;
		fields.push_back(key);
		if (!gRedis->hdel(kGroupDataKey, fields)) {
			DEBUG_LOG("del failed! plid=%u origProto=%u group_key=%s, err=%s", h.PlayerID, h.OrigProtoID, key.c_str(),
						gRedis->last_error_message().c_str());
			return ErrCodeDB;
		}
	}
	return ErrCodeSucc;
}

std::__cxx11::string GroupData::makeGroupKey(uint64_t guid)
{
	std::ostringstream oss;
	oss << guid;
	return oss.str();
}
