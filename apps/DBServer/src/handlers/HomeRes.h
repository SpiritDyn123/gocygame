#pragma once
#include <Proto/Proto.h>
#include "../proto/ErrCode.pb.h"
class HomeRes {
public:
	HomeRes();
	ErrCodeType UpdateResFactory(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType GetResFactory(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType UpdateHierInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);

private:
	std::string makeKey(uint32_t playerId);
	std::string makeHireKey(uint32_t playerId);
};