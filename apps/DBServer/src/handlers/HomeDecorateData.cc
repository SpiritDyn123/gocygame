/*
* HomeDecorateData.cc
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
#include "HomeDecorateData.h"

using namespace std;

HomeDecorateData homeDecorateData;

HomeDecorateData::HomeDecorateData()
{
	gMsgDispatcher.RegisterHandler(DBProtoGetHomeDecorateData, *this, &HomeDecorateData::getHomeDecorateData,
										new db::GetHomeDecorateReq, new db::GetHomeDecorateRsp);
	gMsgDispatcher.RegisterHandler(DBProtoSetHomeDecorateData, *this, &HomeDecorateData::setHomeDecorateData,
										new db::SetHomeDecorateReq, nullptr);
}

ErrCodeType HomeDecorateData::setHomeDecorateData(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::SetHomeDecorateReq, req);

	string key = makeDecorateKey(h.TargetID);
	unordered_map<string, string> fields;

	const db::HomeDecorateData& info = req.data();

	if (info.has_base_info()) {
		if (!info.base_info().SerializeToString(&(fields[kDecorateBaseInfoFiled]))) {
			WARN_LOG("Decorate BaseInfo Failed to serialize! plid=%u", h.TargetID);
			fields.erase(kDecorateBaseInfoFiled);
		}
	}

	vector<std::string> delKeys;
	for (int i = 0; i < info.building_info_size(); i++) {
		const auto& bInfo = info.building_info(i);
		auto bKey = to_string(bInfo.buildings().guid());
		if (bInfo.buildings().id() == 0) {
			delKeys.push_back(bKey);
			continue;
		}
		if (!bInfo.SerializeToString(&(fields[bKey]))) {
			WARN_LOG("Decorate BuidingInfo Failed to serialize! plid=%u guid=%u", h.TargetID, bInfo.buildings().guid());
			fields.erase(bKey);
		}
	}

	// 修改建筑信息
	if (fields.size() > 0 && !gRedis->hset(key, fields)) {
		DEBUG_LOG("setHomeDecorateData failed! plid=%u key=%s", h.TargetID, key.c_str());
		return ErrCodeDB;
	}

	// 删除建筑信息
	if (delKeys.size() > 0 && !gRedis->hdel(key, delKeys)) {
		WARN_LOG("del failed: %s!", gRedis->last_error_cstr());
		return ErrCodeDB;
	}

	gPlayerData.setPlayerDataTouchInfo(h.TargetID, kKeyPrefixHomeDecorateData, true);
	DEBUG_LOG("setHomeDecorateData succ! plid=%u key=%s sets=%lu dels=%lu", h.TargetID, key.c_str(), fields.size(), delKeys.size());
	return ErrCodeSucc;
}

ErrCodeType HomeDecorateData::getHomeDecorateData(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::GetHomeDecorateReq, req);
	REAL_PROTOBUF_MSG(outMsg, db::GetHomeDecorateRsp, rsp);

	// owner info
	rsp.set_player_id(h.TargetID);
	rsp.set_owner_info_flag(req.owner_info_flag());
	cs::PlayerSimpleInfo sInfo;
	if (!req.owner_info_flag()) { // 不从db拉取
		gPlayerData.doGetPlayerHomeInfo(h.TargetID, &sInfo, *rsp.mutable_mon_nick(), rsp.mutable_homebuff_info());
		rsp.mutable_player_name()->swap(*sInfo.mutable_player_name());
		rsp.mutable_picture()->swap(*sInfo.mutable_picture());
		rsp.set_avatar(sInfo.avatar());
		rsp.set_avatar_frame(sInfo.avatar_frame());
		rsp.set_title(sInfo.title());
		rsp.mutable_birthday()->swap(*sInfo.mutable_birthday());
		rsp.set_mon_id(sInfo.primary_monster_id());
		rsp.set_gender(sInfo.gender());
		rsp.mutable_wearing()->Swap(sInfo.mutable_wearing());
		rsp.set_arena_region(sInfo.arena_region());
	} else {
		gPlayerData.doGetPlayerHomeBuffInfo(h.TargetID, rsp.mutable_homebuff_info());
	}

	// decorate info
	string key = makeDecorateKey(h.TargetID);
	unordered_map<string, string> value;
	if (!gRedis->hgetall(key, value)) {
		WARN_LOG("getHomeDecorateData hgetall failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}

	db::HomeDecorateData* pData = rsp.mutable_data();
	for (const auto& v : value) {
		if (v.first.compare(kDecorateBaseInfoFiled) == 0) {
			pData->mutable_base_info()->ParseFromString(v.second);
		} else {
			pData->add_building_info()->ParseFromString(v.second);
		}
	}

	gPlayerData.setPlayerDataTouchInfo(h.TargetID, kKeyPrefixHomeDecorateData, false);
	DEBUG_LOG("getHomeDecorateData succ! plid=%u key=%s", h.TargetID, key.c_str());
	return ErrCodeSucc;
}

int HomeDecorateData::getHomeLevel(uint32_t TargetID)
{
	string key = makeDecorateKey(TargetID);
	unordered_map<string, string> fields;
	string& baseInfoStr = fields[kDecorateBaseInfoFiled];
	if (!gRedis->hget(key, fields)) {
		WARN_LOG("getHomeLevel failed: %s! plid=%u", gRedis->last_error_cstr(), TargetID);
		return ErrCodeDB;
	}
	db::HomeDecorateBaseInfo baseInfo;
	if (!baseInfo.ParseFromString(baseInfoStr)) {
		return 0;
	}

	gPlayerData.setPlayerDataTouchInfo(TargetID, kKeyPrefixHomeDecorateData, false);
	return baseInfo.level();
}