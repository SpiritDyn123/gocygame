#pragma once
#include <string>
#include <Proto/Proto.h>
#include "../proto/CommonMsg.pb.h"
#include "../proto/ErrCode.pb.h"
#include "KeyPrefixDef.h"
#include "MsgHandler.h"

class Arena
	: public MsgHandler
{
public:
	Arena();
	void Init() override;
	void Test();
	bool GetRegionKey(uint32_t index, std::string& key, int gameRegion);
	ErrCodeType	UpdateArenaRank(uint32_t playerID, int32_t score, int gameRegion);
public:
	ErrCodeType GetTarget(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType UpdateInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType GetInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType GetInfoImp(uint32_t tarId, cs::ArenaInfo& info, bool noRank = true, bool noScore = true, int gameRegion = 1);
	ErrCodeType UpdateDefInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType ReplayUpate(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType ReplayAdd(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType GetReplayList(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	//周末区段奖励发放
	ErrCodeType WeekendAward(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);

	//拉取好友积分
	ErrCodeType GetPlayerScore(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);

	ErrCodeType SetArenaScore(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType GetRegionForWorship(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType UpdateYesterdayRegionId(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);

	ErrCodeType GetSimpleInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType UpdateSimpleInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);

	//记录竞技场前30名到mysql
	ErrCodeType RecordTop30(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);

	void TrySendWeekendAwardMail(uint32_t uid);
	void SendWeeklyRankAward(std::set<uint32_t>& vec, int gameRegion);
	uint32_t GetYesterdayRegionId(uint32_t uid);
	int GetPlayerScore(uint32_t uid, int gameRegion);
	//发送测试服竞技场排名top30奖励
	static void addArenaTop30Award(const cs::Account& accInfo, uint32_t uid);
private:
	bool IsLock(const std::string& key, int gameRegion);
	bool TryLock(const std::string& key, int gameRegion);
	void Unlock(const std::string& key, int gameRegion);
	void RemoveRangeOutReplay(uint32_t uid);
	const std::vector<std::string>& GetRegionKeyVec(int gameRegion);
private:
	std::unordered_map<int, std::vector<std::string> > keyByGameRegion_;
	long long					arenaReplayMaxCnt_;
};
extern Arena gArena;