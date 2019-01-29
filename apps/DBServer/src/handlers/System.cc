#include "System.h"
#include "../core/dispatcher.h"
#include "../proto/SvrProtoID.pb.h"
#include "../proto/CSCoreMsg.pb.h"
#include "KeyPrefixDef.h"
#include "../core/redis_client.h"
#include "../CSV/CSVManager.h"
#include <serverbench/config.hpp>
#include "PlayerData.h"
#include "Arena.h"
#include "CapRes.h"
#include "BehaviourRank.h"

System gSystem;

System::System()
{
	gMsgDispatcher.RegisterHandler(ConfigProtoPing, *this, &System::Ping);
	gMsgDispatcher.RegisterHandler(ConfigProtoNotifyReload, *this, &System::NotifyReloadCSV);
	gMsgDispatcher.RegisterHandler(DBProtoGetMaintenanceState, *this, &System::GetMaintenanceState, nullptr, new cs::CSBoolReq);
	gMsgDispatcher.RegisterHandler(DBProtoUpdateMaintenanceState, *this, &System::UpdateMaintenanceState, new cs::CSBoolReq, nullptr);
	gMsgDispatcher.RegisterHandler(DBProtoGetActOpenStates, *this, &System::GetActOpenStates, nullptr, new cs::GmActOpenStates);
	gMsgDispatcher.RegisterHandler(DBProtoUpdateActOpenState, *this, &System::UpdateActOpenState, new cs::GmActOpenState, nullptr);
}

ErrCodeType System::Ping(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	return ErrCodeSucc;
}

ErrCodeType System::NotifyReloadCSV(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	//什麽都不用做，但是不能移除,否則返回消息會引發ConfigSvr重複發送導致死循環
	CSVManager::Load(config_get_strval("csv_path"));
	gPlayerData.Init();
	gArena.Init();
	gArena.Test();
	gCapResData.Init();
	gBehaviourRank.Init();
	return ErrCodeSucc;
}

ErrCodeType System::GetMaintenanceState(const SSProtoHead & h, google::protobuf::Message * inMsg, google::protobuf::Message * outMsg)
{
	REAL_PROTOBUF_MSG(outMsg, cs::CSBoolReq, req);
	bool state;
	gRedis->get(kKeyMaintenanceState, state);
	req.set_is_true(state);
	return ErrCodeSucc;
}

ErrCodeType System::UpdateMaintenanceState(const SSProtoHead & h, google::protobuf::Message * inMsg, google::protobuf::Message * outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, cs::CSBoolReq, req);
	gRedis->set(kKeyMaintenanceState, std::to_string(req.is_true()));
	return ErrCodeSucc;
}

ErrCodeType System::GetActOpenStates(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(outMsg, cs::GmActOpenStates, rsp);

	std::unordered_map<std::string, std::string> m;
	if (!gRedis->hgetall(kKeyGmActOpenState, m)) {
		DEBUG_LOG("GetActOpenStates, hgetall failed: %s!", gRedis->last_error_cstr());
		return ErrCodeDB;
	}
	for (auto& it : m) {
		auto p = rsp.add_states();
		p->set_id(atoi(it.first.c_str()));
		p->set_state(atoi(it.second.c_str()));
	}
	return ErrCodeSucc;
}

ErrCodeType System::UpdateActOpenState(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, cs::GmActOpenState, req);

	std::unordered_map<std::string, std::string> m;
	m[std::to_string(req.id())] = std::to_string(req.state());
	if (!gRedis->hset(kKeyGmActOpenState, m)) {
		DEBUG_LOG("UpdateActOpenState, hset failed: %s!", gRedis->last_error_cstr());
		return ErrCodeDB;
	}
	return ErrCodeSucc;
}

