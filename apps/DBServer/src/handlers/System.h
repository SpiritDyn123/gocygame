#pragma once
#include <Proto/Proto.h>
#include "../proto/CommonMsg.pb.h"
#include "../proto/ErrCode.pb.h"
class System {
public:
	System();
private:
	ErrCodeType Ping(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType NotifyReloadCSV(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType GetMaintenanceState(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType UpdateMaintenanceState(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType GetActOpenStates(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType UpdateActOpenState(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
};
