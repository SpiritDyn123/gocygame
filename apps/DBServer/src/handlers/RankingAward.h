#pragma once
#include <string>
#include <Proto/Proto.h>
#include "../proto/ErrCode.pb.h"

class RankingAward
{
public:
	RankingAward();
	void Init();
public:
	ErrCodeType TrySendAward(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
private:

};

extern RankingAward gRankingAward;
