#pragma once
#include <Proto/Proto.h>
#include "../proto/ErrCode.pb.h"
#include "../core/sqlconnection.h"
#include "MsgHandler.h"
class BtlReplay
	: public MsgHandler
{
public:
	BtlReplay();
	void Init() override;
public:
	void RemoveReplay(uint32_t uid, const std::string& key);
private:
	ErrCodeType addReplay(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType getReplay(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType getReplayList(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
private:
	std::string GetBtlUserKey(uint32_t uid);
	std::string GetReplayId(uint32_t uid);
	void AddReplay(uint32_t pid, std::string& data, const std::string& ver);
private:
	std::string dbPrefix_;
	std::string tblPrefix_;
	bool ignoreVer_;
};
extern BtlReplay gBtlReplay;
