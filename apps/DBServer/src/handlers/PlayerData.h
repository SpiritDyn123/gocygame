/*
 * player_data.h
 *
 *  Created on: 2017年2月13日
 *      Author: antigloss
 */

#ifndef DIGIMON_SRC_HANDLERS_PLAYER_DATA_H_
#define DIGIMON_SRC_HANDLERS_PLAYER_DATA_H_

#include "../proto/ErrCode.pb.h"
#include "../proto/Core.pb.h"
#include "../proto/db.pb.h"
#include "../CSV/CSVNpcRes.h"
#include "../CSV/CSVResInfo.h"
#include <list>
#include "MsgHandler.h"
struct int_pair_hash {
    std::size_t operator () (const std::pair<int,int> &p) const {
        return std::hash<int>{}(p.first) ^ std::hash<int>{}(p.second);  
    }
};

class PlayerData
	: public MsgHandler
{
public:
	PlayerData();
	void Init() override;

	// 检查plid1的黑名单里面是否有plid2
	ErrCodeType CheckIfInBlacklist(uint32_t plid1, uint32_t plid2);
	// < 0 DB错误，0不在孵化中，>0孵化中
	int CheckIfIsHatching(uint32_t plid);
	void GetBlackList(uint32_t uid, ::google::protobuf::RepeatedField<uint32_t>& list);

	//static void doGetPlayerHomeInfo(uint32_t plid, cs::PlayerSimpleInfo* playerInfo, google::protobuf::RepeatedField<::google::protobuf::int32>* monIds, db::PlayerHomeBuffInfo* homeBuffs);
	static void doGetPlayerHomeInfo(uint32_t plid, cs::PlayerSimpleInfo* playerInfo, std::string& mon_nick, db::PlayerHomeBuffInfo* homeBuffs);
	static void doGetPlayerHomeBuffInfo(uint32_t plid, db::PlayerHomeBuffInfo* homeBuffs);

private:
	ErrCodeType getRealPlayerID(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType delRealPlayerID(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType getAccountInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType getPlayerRegionInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);

	ErrCodeType getGMPlayerID(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType getPlayersByMomoID(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType getPlayersLvByMomoID(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType getSnapShotBeg(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType getSnapShot(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType getCorpSnapShot(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType getMaxPlayerID(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	// 为打点返回玩家信息
	ErrCodeType getPlayerInfoForStat(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType onSetInteractInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType onViewPlayerInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType onViewInteractInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);

	ErrCodeType createPlayer(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType setPlayerData(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType getPlayerData(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType getHomeInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType getPlayerBasicInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType getPlayerAndMonBasicInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType getFriendsBasicInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType getPlayerExtraData(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType getTodayRandPlayers(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType getPlayerGrpInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);

	ErrCodeType addOfflineMsg(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType getOfflineMsgs(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);

	ErrCodeType allocNick(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType releaseNick(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType getPlayerIDByNick(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType getPlayerFormation(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType	getGMPlayerData(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType	addLove(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType	addPopular(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType	addWealth(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType	setMomoPopular(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType	setMomoLove(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType	setMomoFriendShip(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType	modifyNameBirthPos(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType	completeNewbieGuide(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);

	ErrCodeType	getRecruitInviteCode(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType	finishBeRecruited(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType	addRecruiteScore(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);

	ErrCodeType	addWorship(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType	delWorship(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType	getWorship(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	
	ErrCodeType	giveGift(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType getPlayerGiftInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType getPlayerGiftRecordByPlid(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType getPlayerGiftRecvRecord(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType getPlayerGiftRecvRank(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType GiftGemExchange(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	
	ErrCodeType onAddFailedSyncMomoItem(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType onGetFailedSyncMomoItem(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType onDelFailedSyncMomoItem(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType banPlayer(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType banDeviceOrIP(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType getMultiPlayerDevice(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType banChannel(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType gsMember(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType gsMemberList(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType gsRecRoomUpdate(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType gsRecRoomList(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType onChangePlayerNameForcible(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType onChannelSwitchUpdate(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType onChannelSwitchList(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType gmSelfTest(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType getShopParam(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType getPlayerSocialInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType getPlayerMiscInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType addHomeItemPopular(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);

	ErrCodeType followPlayer(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType removeFollowings(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType removeFollowers(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType addFavFollowing(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType removeFavFollowing(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType addFriendship(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType addToBlacklist(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType delFromBlacklist(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType chkIfInBlacklist(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType updatePlayerActivityData(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);

	ErrCodeType getPlayersAdventureRecord(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);

	ErrCodeType	getPlayerTaskGuid(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType	setPlayerRedPointInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);

	ErrCodeType updatePlayerAccInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);

	ErrCodeType offLine(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType getMoreFans(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	void online(uint32_t playerId);
	ErrCodeType updateOnlineInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);

	ErrCodeType checkArFaceId(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType getArFaceInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType checkArFaceDailyTask(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType setARFaceShowInfofbd(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType setARFaceIncrBy(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);

	ErrCodeType updateGarment(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	
	ErrCodeType checkBetaSubscribe(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType onBetaPhoneSave(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType updatePlayerBeta(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);

	ErrCodeType onUpdatePlayerGarWearing(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType onUpdateAcTaskInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType onUpdateHutInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType onUpdatePlayerCollectCacheInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);

	ErrCodeType getPlayerMonsterInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType getActGlobalInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType updateActGlobalInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	
	//各渠道的礼包
	ErrCodeType giftGivenChannel(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType getPlayerExtraInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);

	ErrCodeType setOperatorVer(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType onChangePlayerData(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType onGetLoginRecords(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType onGetBindInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType onAddLotteryRecord(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType onGetLotteryRecords(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType onAddSpamRecord(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType onGetSpamRecords(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType onUpdateSpamState(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
private:
	static ErrCodeType doSetPlayerData(const SSProtoHead& h, const std::string& key, db::PlayerData& data, bool isCreatePlayer = false);
	static void SetPlayerLoginedToday(std::string& strPlayerID, bool newbieFinished, int gameRegion);
	static bool toPlayerData(std::unordered_map<std::string, std::string>& m, db::PlayerData& data);
	static ErrCodeType doGetPlayerAndMonBasicInfo(uint32_t plid, uint32_t targetID, cs::PlayerSimpleInfo* playerInfo, cs::MonsterSimpleInfo* monInfo, bool lastRegion = false);
	static void doGetFriendBasicInfo(uint32_t plid, uint32_t targetID, db::GetFriendsBasicInfoRsp& rsp);
	static void toFriendsData(std::unordered_map<std::string, std::string>& m, std::unordered_map<std::string, std::string>& momo, google::protobuf::RepeatedPtrField<cs::FriendsData_OneFriend>* friends);
	static void doGetPlayerAdventureTowerDatas(uint32_t plid, uint32_t targetID, google::protobuf::RepeatedPtrField<cs::AdventureData_AdventureTowerData>* towerDatas);
	static ErrCodeType doAddDailyPopular(uint32_t plid, uint32_t addValue);
	static ErrCodeType checkDailyPopular(uint32_t plid, int* yPopular = nullptr);
	static ErrCodeType checkWeeklySocial(uint32_t plid);
	static ErrCodeType checkDailySocial(uint32_t plid);
	static ErrCodeType calMaxDigiCnt(uint32_t playerId, uint32_t &cnt, uint32_t &skin);
	static ErrCodeType calTotalGiftCnt(uint32_t playerID, uint32_t &cnt);
	//static int calAvatar(cs::GarmentWearing* info, int avatar, int gender);
	static int getPlayerSetting(cs::KeyVals* settings, cs::PlayerSettingType type);
	static bool checkDoArMonDailyTask(const std::string& monid, uint32_t plid);
	static bool checkPlayerIsBan(uint32_t pid);

public:
	static ErrCodeType GetPopAndLove(std::string& strPlayerID, long long* totalPopular, long long* totalLove, long long* weekPopular, long long* weekLove);
	static ErrCodeType GetSocialSimpleInfo(std::string& strPlayerID, db::SimpleSocialInfo& info);
	static int GetPlayerTotalPopluar(uint32_t plid);
	static int GetPlayerTotalWealth(uint32_t plid);
	static int GetPlayerTrialTowerRankScore(uint32_t plid);
	static int GetGameRegion(uint32_t plid);
	static void setPlayerDataTouchInfo(uint32_t plid, const std::string& tKey, bool setFlag);
	static bool IsGsMember(uint32_t plid, bool ever = true);
	static void updatePlayerLoginRecord(uint32_t pid, std::string ip);

private:
	//std::unordered_map<uint32_t, uint32_t> avatarMap_;
	uint32_t maxPlayerID_;
};

extern PlayerData gPlayerData;

#endif /* DIGIMON_SRC_HANDLERS_PLAYER_DATA_H_ */
