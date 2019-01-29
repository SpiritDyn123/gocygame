#pragma once
// ����
#include "MsgHandler.h"
#include <Proto/Proto.h>
#include "../proto/ErrCode.pb.h"

class Pavilion
	: public MsgHandler
{
public:
	Pavilion();
	void Init() override;

public:
	uint32_t GetPlayerPavilionScore(uint32_t plid);
private:
	ErrCodeType Challenge(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType GetLog(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType onWriteLog(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType onClearLog(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType UpdateScore(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
};

extern Pavilion gPavilion;