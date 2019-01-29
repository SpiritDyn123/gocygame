/*
 * HatchData.cc
 *
 *  Created on: 2017年3月2日
 *      Author: antigloss
 */

#include <unordered_map>
#include "../proto/SvrProtoID.pb.h"
#include "../proto/db.pb.h"
#include "../core/dispatcher.h"
#include "../core/redis_client.h"

#include "KeyPrefixDef.h"
#include "PlayerData.h"
#include "HatchData.h"

using namespace std;

static HatchData hatchData;
static std::string kKeyRedGuidKey = "kKeyRedGuidKey";

HatchData::HatchData()
{
//	gMsgDispatcher.RegisterHandler(DBProtoCreateHatchData, *this, &HatchData::createHatchData,
//									new db::CreateHatchDataReq, new db::CreateHatchDataRsp);
//	gMsgDispatcher.RegisterHandler(DBProtoSetHatchData, *this, &HatchData::setHatchData,
//									new db::SetHatchDataReq, nullptr);
//	gMsgDispatcher.RegisterHandler(DBProtoGetHatchData, *this, &HatchData::getHatchData,
//									new db::GetHatchDataReq, new db::GetHatchDataRsp);
//	gMsgDispatcher.RegisterHandler(DBProtoDelHatchData, *this, &HatchData::delHatchData,
//									new db::DelHatchDataReq, nullptr);
	// 以上已作废
	gMsgDispatcher.RegisterHandler(DBProtoHatchCreate, *this, &HatchData::createHatchInfo,
									new db::CreateHatchInfoReq, new db::CreateHatchInfoRsp);
	gMsgDispatcher.RegisterHandler(DBProtoHatchSetData, *this, &HatchData::setHatchInfo,
								   new db::StrReq);
	gMsgDispatcher.RegisterHandler(DBProtoHatchLoadData, *this, &HatchData::loadHatchData,
									new db::BoolReq, new db::LoadHatchDataRsp);
	gMsgDispatcher.RegisterHandler(DBProtoHatchDelData, *this, &HatchData::delHatchInfo,
									new db::Uint32Req);
	gMsgDispatcher.RegisterHandler(DBProtoHatchAddMsg, *this, &HatchData::addHatchMsg,
									new db::StrReq);
	gMsgDispatcher.RegisterHandler(DBProtoHatchGetMsg, *this, &HatchData::getHatchMsg,
									nullptr, new db::RepeatedStrRsp);
	gMsgDispatcher.RegisterHandler(DBProtoSetRedPacket, *this, &HatchData::SetRedPacketMsg,
									new db::SetRedPacketInfo, nullptr);
	gMsgDispatcher.RegisterHandler(DBProtoBuildingLog, *this, &HatchData::SetBuildingLogMsg,
									new db::SetBuildingLogInfo, nullptr);

}

//----------------------------------------------------------------------
// Private Methods
//----------------------------------------------------------------------
ErrCodeType HatchData::createHatchData(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::StrReq, dbreq);
	REAL_PROTOBUF_MSG(outMsg, db::CreateHatchDataRsp, rsp);

	db::CreateHatchDataReq req;
	req.ParseFromString(dbreq.str());
	string key = makeHatchKey(req.hatch_guid());
	RedisClient::ExpirationTime expTm(false, 86400 * 180); // 强制180天过期
	if (!gRedis->set(key, req.data(), &expTm)) {
		DEBUG_LOG("set failed! plid=%u origProto=%u err=%s",
					h.PlayerID, h.OrigProtoID, gRedis->last_error_message().c_str());
		return ErrCodeDB;
	}

	rsp.set_hatch_guid(req.hatch_guid());
	rsp.mutable_data()->swap(*req.mutable_data());
	return ErrCodeSucc;
}

ErrCodeType HatchData::setHatchData(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::StrReq, dbreq);

	db::SetHatchDataReq req;
	req.ParseFromString(dbreq.str());
	string key = makeHatchKey(req.hatch_guid());
	RedisClient::ExpirationTime expTm(false, 86400 * 180); // 强制180天过期
	if (!gRedis->set(key, req.data(), &expTm)) {
		DEBUG_LOG("set failed! plid=%u origProto=%u err=%s",
					h.PlayerID, h.OrigProtoID, gRedis->last_error_message().c_str());
		return ErrCodeDB;
	}

	return ErrCodeSucc;
}

ErrCodeType HatchData::getHatchData(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::GetHatchDataReq, req);
	REAL_PROTOBUF_MSG(outMsg, db::GetHatchDataRsp, rsp);

	auto data = rsp.mutable_data();
	for (int i = 0; i != req.hatch_guid_size(); ++i) {
		doGetHatchData(h, req.hatch_guid(i), data);
	}

	return ErrCodeSucc;
}

ErrCodeType HatchData::delHatchData(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::DelHatchDataReq, req);

	string key = makeHatchKey(req.hatch_guid());
	if (!req.del_now()) {
		// 为了容错，并不立刻删除，而是设置15天过期
		if (gRedis->expire(key, 15 * 86400)) { // 反正有180天强制超时，故无论是否删除成功都无所谓
			DEBUG_LOG("'%s' will be deleted in 15 days. plid=%u origProto=%u",
						key.c_str(), h.PlayerID, h.OrigProtoID);
		}
	} else {
		vector<string> keys;
		keys.emplace_back(key);
		if (gRedis->del(keys)) { // 反正有180天强制超时，故无论是否删除成功都无所谓
			DEBUG_LOG("Del hatch data %s. plid=%u origProto=%u", key.c_str(), h.PlayerID, h.OrigProtoID);
		}
	}

	return ErrCodeSucc;
}

// 以上已作废
//-------------------

ErrCodeType HatchData::createHatchInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::CreateHatchInfoReq, req);
	REAL_PROTOBUF_MSG(outMsg, db::CreateHatchInfoRsp, rsp);

	string key = makeHatchKey(h.TargetID);
	unordered_map<string, string> fields;
	DEBUG_LOG("set createHatchInfo %s %s %u", key.c_str(), makeHatchFiled(req.egg_guid()).c_str(), req.egg_guid());
	fields[makeHatchFiled(req.egg_guid())] = req.hatch_info(); //TODO:
	if (!gRedis->hset(key, fields)) {
		DEBUG_LOG("set failed! plid=%u origProto=%u err=%s",
					h.PlayerID, h.OrigProtoID, gRedis->last_error_message().c_str());
		return ErrCodeDB;
	}

	rsp.set_egg_guid(req.egg_guid());
	rsp.mutable_hatch_info()->swap(*req.mutable_hatch_info());
	return ErrCodeSucc;
}

ErrCodeType HatchData::setHatchInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::StrReq, dbreq);

	db::HatchInfo req;
	req.ParseFromString(dbreq.str());
	string key = makeHatchKey(h.TargetID);
	unordered_map<string, string> fields;
	//DEBUG_LOG("set setHatchInfo %u",  req.egg_guid());
	fields[makeHatchFiled(req.egg_guid())] = dbreq.str(); //TODO:
	if (!gRedis->hset(key, fields)) {
		DEBUG_LOG("set failed! plid=%u origProto=%u err=%s",
					h.PlayerID, h.OrigProtoID, gRedis->last_error_message().c_str());
		return ErrCodeDB;
	}

	return ErrCodeSucc;
}

ErrCodeType HatchData::loadHatchData(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	//判定放到interSvr处理
	/*if (h.PlayerID != h.TargetID) {
		auto err = gPlayerData.CheckIfInBlacklist(h.TargetID, h.PlayerID);
		if (err != ErrCodeSucc) {
			return err;
		}
	}*/
	REAL_PROTOBUF_MSG(inMsg, db::BoolReq, req);
	REAL_PROTOBUF_MSG(outMsg, db::LoadHatchDataRsp, rsp);
	gPlayerData.GetBlackList(h.TargetID, *rsp.mutable_black_list());
	if (req.flag()) {
		string key = makeHatchKey(h.TargetID);
		//DEBUG_LOG("HatchInfo %s", key.c_str());
		unordered_map<string, string> m;
		if (!gRedis->hgetall(key, m)) {
			WARN_LOG("loadHatchData hgetall failed: %s! plid=%u", gRedis->last_error_cstr(), h.PlayerID);
			return ErrCodeDB;
		}
		for (const auto& v : m) {
			rsp.add_hatch_infos(v.second);
		}

		unordered_map<string, string> m1;
		key = makeBuildingLogKey(h.TargetID);
		if (!gRedis->hgetall(key, m1)) {
			WARN_LOG("loadHatchData hgetall failed: %s! plid=%u", gRedis->last_error_cstr(), h.PlayerID);
			return ErrCodeDB;
		}

		for (const auto& v : m1) {
			rsp.mutable_logs()->add_logs()->ParseFromString(v.second);
		}

		key = makeRedPacketKey(h.TargetID);
		unordered_map<string, string> m2;
		if (!gRedis->hgetall(key, m2)) {
			WARN_LOG("loadHatchData hgetall failed: %s! plid=%u", gRedis->last_error_cstr(), h.PlayerID);
			return ErrCodeDB;
		}
		for (const auto& v : m2) {
			if (v.first == kKeyRedGuidKey) {
				rsp.mutable_info()->set_guid(atoi(v.second.c_str()));
			} else {
				rsp.mutable_info()->add_record()->ParseFromString(v.second);
			}
		}

	}

	return ErrCodeSucc;
}

ErrCodeType HatchData::delHatchInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::Uint32Req, req);

	string key = makeHatchKey(h.TargetID);
	vector<string> delHatch;
	delHatch.push_back(makeHatchFiled(req.u32()));
	if (!gRedis->hdel(key, delHatch)) {
		DEBUG_LOG("delHatchInfo failed! plid=%u origProto=%u err=%s",
					h.PlayerID, h.OrigProtoID, gRedis->last_error_message().c_str());
		return ErrCodeDB;
	}
	//DEBUG_LOG("delHatchInfo succ! %s plid=%u egg=%u", key.c_str(), h.TargetID, req.u32());

	return ErrCodeSucc;
}

ErrCodeType HatchData::addHatchMsg(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::StrReq, req);

	string key(kKeyPrefixHatchMsg + to_string(h.TargetID));
	long long cnt = 0;
	vector<string> vals(1);
	vals[0].swap(*req.mutable_str());
	if (!gRedis->lpush(key, vals, &cnt)) {
		WARN_LOG("lpush failed: %s! plid=%u", gRedis->last_error_message().c_str(), h.TargetID);
		return ErrCodeDB;
	}
	if (cnt > 100) { // 最多保存100条
		gRedis->ltrim(key, 0, 99);
	}

	return ErrCodeSucc;
}

ErrCodeType HatchData::getHatchMsg(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	string key(kKeyPrefixHatchMsg + to_string(h.TargetID));
	vector<string> vals;
	if (!gRedis->lrange(key, 0, 99, vals)) {
		WARN_LOG("lrange failed: %s! plid=%u", gRedis->last_error_message().c_str(), h.TargetID);
		return ErrCodeSucc; // 拉取离线消息失败不当成错误
	}

	REAL_PROTOBUF_MSG(outMsg, db::RepeatedStrRsp, rsp);
	for (auto& val : vals) {
		rsp.add_strs()->swap(val);
	}

	return ErrCodeSucc;
}


ErrCodeType HatchData::SetRedPacketMsg(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::SetRedPacketInfo, req);
	string key(makeRedPacketKey(h.TargetID));
	unordered_map<string, string> fields;
	fields[kKeyRedGuidKey] = to_string(req.info().guid());
	for (int i = 0; i < req.info().record_size(); ++i) {
		auto& iter = fields[to_string(req.info().record(i).guid())];
		req.info().record(i).SerializeToString(&iter);
	}

	if (!gRedis->hset(key, fields)) {
		DEBUG_LOG("set failed! plid=%u origProto=%u err=%s",
				h.PlayerID, h.OrigProtoID, gRedis->last_error_message().c_str());
		return ErrCodeDB;
	}

	return ErrCodeSucc;
}


ErrCodeType HatchData::SetBuildingLogMsg(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::SetBuildingLogInfo, req);
	string key(makeBuildingLogKey(h.TargetID));
	unordered_map<string, string> fields;
	for (int i = 0; i < req.logs().logs_size(); ++i) {
		auto& iter = fields[to_string(req.logs().logs(i).bid())];
		req.logs().logs(i).SerializeToString(&iter);
	}

	if (!gRedis->hset(key, fields)) {
		DEBUG_LOG("set failed! plid=%u origProto=%u err=%s",
				h.PlayerID, h.OrigProtoID, gRedis->last_error_message().c_str());
		return ErrCodeDB;
	}

	return ErrCodeSucc;
}

//=============================
// Helpers
//
void HatchData::doGetHatchData(const SSProtoHead& h, uint64_t guid, google::protobuf::RepeatedPtrField<string>* data)
{
	// TODO 数据不存在怎么处理？？

	string key = makeHatchKey(guid);
	string val;
	bool keyExists;
	if (!gRedis->get(key, val, &keyExists)) {
		DEBUG_LOG("get failed! plid=%u origProto=%u err=%s",
					h.PlayerID, h.OrigProtoID, gRedis->last_error_message().c_str());
		return;
	}

	if (!keyExists) {
		DEBUG_LOG("Cant find %s! plid=%u origProto=%u", key.c_str(), h.PlayerID, h.OrigProtoID);
		return;
	}

	data->Add()->swap(val);
}
