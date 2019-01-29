#pragma once
#include <Proto/Proto.h>
#include "../proto/ErrCode.pb.h"
#include "../proto/CommonMsg.pb.h"
#include <stdint.h>
#include <vector>
#include <string>
#include <unordered_map>
#include "MsgHandler.h"

class BehaviourRank
	: public MsgHandler
{
public:
	BehaviourRank();
	void Init() override;
public:
	void TrySendAwardMail(uint32_t uid);
	void DelRankInfo(uint32_t uid, int gameRegion);
private:
	ErrCodeType onUpdate(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType onTrySendAward(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType onGetList(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	std::string GetRankKey(uint32_t bid, int gameRegion);
	std::string GetFlagKey(int gameRegion);
private:
	struct Award {
		struct Item {
			Item(const std::vector<int>& info);
			void AttachToMail(cs::MailBase* mail) const;
			uint32_t type_;
			uint32_t id_;
			uint32_t cnt_;
			std::vector<int> exData_;
		};
		uint32_t begin_;
		uint32_t end_;
		std::vector<Item> award_;
	};
	struct AwardInfo {
		AwardInfo() :showMax_(0) {}
		std::vector<Award> award_;
		uint32_t mailId_;
		uint32_t showMax_;
	};
	//key: behavior id; val:award by rank
	std::unordered_map<uint32_t, AwardInfo > awards_;
};
extern BehaviourRank gBehaviourRank;