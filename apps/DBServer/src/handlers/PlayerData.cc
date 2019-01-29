#include <vector>
#include <sstream>
#include <iomanip>
#include <libant/hash/hash_algo.h>
#include <libant/utils/StringUtils.h>
#include <google/protobuf/util/json_util.h>

#include "../core/dispatcher.h"
#include "../core/redis_client.h"
#include "../core/ProxyConnector.h"
#include "../core/ScriptMgr.h"
#include "../proto/SvrProtoID.pb.h"
#include "../proto/CenterServer.pb.h"
#include "../proto/InteractServer.pb.h"
#include "../proto/GmMsg.pb.h"

#include "KeyPrefixDef.h"
#include "PlayerData.h"
#include "MailData.h"
#include "RankData.h"
#include "GeoData.h"
#include "PayData.h"
#include "WishData.h"
#include "CorpsData.h"
#include "Arena.h"
#include "HomeDecorateData.h"
#include "BehaviourRank.h"
#include "../TimeUtils.h"
#include "../core/MysqlProxy.h"
#include "../tables/tbl_social_interact.h"
#include "../tables/tbl_social_stranger.h"
#include "../tables/tbl_player_nick.h"
#include "../tables/tbl_player_region.h"
#include "../tables/tbl_login_record.h"
#include "../tables/tbl_lottery.h"
#include "../tables/tbl_spam.h"
#include "../CSV/CSVMonster.h"
#include "../CSV/CSVMonsterChipBasic.h"
#include "../CSV/CSVFriendLvExp.h"
#include "../CSV/CSVHomeItem.h"
#include "../CSV/CSVAchivement.h"
#include "../CSV/CSVPveAdventureDungeon.h"
#include "../CSV/CSVAvatar.h"
#include "../CSV/CSVDailyTasksBasic.h"
#include "../CSV/CSVChannelGift.h"
#include "../global.h"


using namespace std;

PlayerData gPlayerData;

enum {
	kMaleFlag			= 1,  // 男性
	kMaxFavFollowingCnt	= 10, // 最大特别关注数
	kMaxFollowingCnt	= 50, // 最大关注数
	kMaxFollowerCnt		= 500, // 最大粉丝数
	kPlayerIDOffset		= 100000, // 玩家id偏移量
};

#define SOCIAL_USE_REDIS 0

PlayerData::PlayerData()
{
	// 登录相关
	gMsgDispatcher.RegisterHandler(DBProtoGetRealPlayerID, *this, &PlayerData::getRealPlayerID,
									new db::GetRealPlayerIDReq, new db::GetRealPlayerIDRsp);
	gMsgDispatcher.RegisterHandler(DBProtoDelPlayerByID, *this, &PlayerData::delRealPlayerID,
									new db::Uint32Req, new db::Uint32Rsp);
	gMsgDispatcher.RegisterHandler(DBProtoGetAccountInfo, *this, &PlayerData::getAccountInfo, nullptr, new cs::Account);
	gMsgDispatcher.RegisterHandler(DBProtoGetPlayerReginInfo, *this, &PlayerData::getPlayerRegionInfo, new db::GetPlayerRegionInfoReq, new db::GetPlayerRegionInfoRsp);

	gMsgDispatcher.RegisterHandler(DBProtoGMGetPlayerID, *this, &PlayerData::getGMPlayerID, new db::QueryPlayerIDReq, new db::QueryPlayerIDRsp);
	gMsgDispatcher.RegisterHandler(DBProtoGetPlayersByMomoID, *this, &PlayerData::getPlayersByMomoID, new db::RepeatedStrReq, new cs::RepeatedPlayerWithName);
	gMsgDispatcher.RegisterHandler(DBProtoGetPlayersLvByMomo, *this, &PlayerData::getPlayersLvByMomoID, new db::RepeatedStrReq, new db::QueryPlayerIDRsp);
	gMsgDispatcher.RegisterHandler(DBProtoGetSnapBeg, *this, &PlayerData::getSnapShotBeg, new db::SnapShotBegReq, new db::SnapShotBegRsp);
	gMsgDispatcher.RegisterHandler(DBProtoGetSnapShot, *this, &PlayerData::getSnapShot, new db::SnapShotReq, new db::SnapShotRsp);
	gMsgDispatcher.RegisterHandler(DBProtoGetCorpSnapShot, *this, &PlayerData::getCorpSnapShot, new db::RepeatedUint32Req, new db::CorpSnapShotRsp);
	gMsgDispatcher.RegisterHandler(DBProtoGetMaxPlayerID, *this, &PlayerData::getMaxPlayerID, nullptr, new db::Uint32Rsp);
	gMsgDispatcher.RegisterHandler(DBProtoCreatePlayer, *this, &PlayerData::createPlayer,
									new db::PlayerData, nullptr);
	gMsgDispatcher.RegisterHandler(DBProtoSetPlayerData, *this, &PlayerData::setPlayerData,
									new db::PlayerData, nullptr);
	gMsgDispatcher.RegisterHandler(DBProtoGetPlayerData, *this, &PlayerData::getPlayerData,
									new db::Uint32Req, new db::PlayerData);
	gMsgDispatcher.RegisterHandler(DBProtoGetHomeInfo, *this, &PlayerData::getHomeInfo,
									nullptr, new db::GetHomeInfoRsp);
	gMsgDispatcher.RegisterHandler(DBProtoGetPlayerBasicInfo, *this, &PlayerData::getPlayerBasicInfo,
									nullptr, new cs::SCGetPlayerBasicInfo);
	gMsgDispatcher.RegisterHandler(DBProtoGetPlayerAndMonBasicInfo, *this, &PlayerData::getPlayerAndMonBasicInfo,
								   new cs::CSGetPlayersBasicInfoReq, new cs::SCGetPlayersBasicInfo);
	gMsgDispatcher.RegisterHandler(DBProtoGetFriendsBasicInfo, *this, &PlayerData::getFriendsBasicInfo,
									new db::RepeatedUint32Req, new db::GetFriendsBasicInfoRsp);
	gMsgDispatcher.RegisterHandler(DBProtoGetPlayerExtraData, *this, &PlayerData::getPlayerExtraData,
									new db::Uint32Req, new db::PlayerExtraData);
	gMsgDispatcher.RegisterHandler(DBProtoGetTodayRandPlayers, *this, &PlayerData::getTodayRandPlayers,
									new db::GetGetTodayRandPlayersReq, new db::RepeatedUint32Rsp);
	gMsgDispatcher.RegisterHandler(DBProtoGetPlayerGrpInfo, *this, &PlayerData::getPlayerGrpInfo,
									new db::RepeatedUint32Req, new db::GetPlayerGrpInfoRsp);
	gMsgDispatcher.RegisterHandler(DBProtoGetPlayerSocialInfo, *this, &PlayerData::getPlayerSocialInfo, nullptr, new cs::PlayerSocialProfile);
	gMsgDispatcher.RegisterHandler(DBProtoGetPlayerMiscInfo, *this, &PlayerData::getPlayerMiscInfo, nullptr, new db::PlayerMiscInfo);
	gMsgDispatcher.RegisterHandler(DBProtoGetMisc, *this, &PlayerData::getPlayerMiscInfo, nullptr, new db::PlayerMiscInfo);

	gMsgDispatcher.RegisterHandler(DBProtoAddOfflineMsg, *this, &PlayerData::addOfflineMsg,
									new db::AddOfflineMsgReq, new db::AddOfflineMsgRsp);
	gMsgDispatcher.RegisterHandler(DBProtoGetOfflineMsg, *this, &PlayerData::getOfflineMsgs,
									nullptr, new db::GetOfflineMsgsRsp);

	gMsgDispatcher.RegisterHandler(DBProtoAllocNick, *this, &PlayerData::allocNick, 
									new db::AllocNickReq, new db::AllocNickRsp);
	gMsgDispatcher.RegisterHandler(DBProtoReleaseNick, *this, &PlayerData::releaseNick, 
									new db::ReleaseNickReq, nullptr);
	gMsgDispatcher.RegisterHandler(DBProtoGetPlayerIDByNick, *this, &PlayerData::getPlayerIDByNick,
									new db::StrReq, new db::Uint32Rsp);
	gMsgDispatcher.RegisterHandler(DBProtoBanPlayer, *this, &PlayerData::banPlayer,
									new db::BanReq, new db::Uint32Rsp);
	gMsgDispatcher.RegisterHandler(DBProtoGetPlayerFormation, *this, &PlayerData::getPlayerFormation,
									new db::GetPlayerFormationReq, new db::DBMonsterFormation);
	gMsgDispatcher.RegisterHandler(DBProtoBanDeviceOrIP, *this, &PlayerData::banDeviceOrIP, new db::BanDeviceOrIP, new db::RepeatedUint32Rsp);
	// 拉取有刷号嫌疑的设备和IP
	gMsgDispatcher.RegisterHandler(DBProtoGetMultiPlayerDevice, *this, &PlayerData::getMultiPlayerDevice, nullptr, new db::MultiPlayerDeviceRsp);
	gMsgDispatcher.RegisterHandler(DBProtoBanPayChannel, *this, &PlayerData::banChannel, new db::BanPayReq, nullptr);
	// GS人员操作 用来控制房间自动置顶
	gMsgDispatcher.RegisterHandler(DBProtoGsMember, *this, &PlayerData::gsMember, new db::GsMemberInfo, new db::GsMemberListInfo);
	gMsgDispatcher.RegisterHandler(DBProtoGsMemberList, *this, &PlayerData::gsMemberList, nullptr, new db::GsMemberListInfo);
	gMsgDispatcher.RegisterHandler(DBProtoGsRecRoomUpdate, *this, &PlayerData::gsRecRoomUpdate, new cs::SCRepeatedUint32Rsp, nullptr);
	gMsgDispatcher.RegisterHandler(DBProtoGsRecRoomList, *this, &PlayerData::gsRecRoomList, nullptr, new cs::SCRepeatedUint32Rsp);
	gMsgDispatcher.RegisterHandler(DBProtoGMSelfTest, *this, &PlayerData::gmSelfTest, new db::GmSelfTestInfo, nullptr);
	// gm强制改名
	gMsgDispatcher.RegisterHandler(DBProtoChangePlayerNameForcible, *this, &PlayerData::onChangePlayerNameForcible, nullptr, new cs::SCStringRsp);
	// gm渠道开关
	gMsgDispatcher.RegisterHandler(DBProtoChannelSwitchUpdate, *this, &PlayerData::onChannelSwitchUpdate, new GmMsg::GmChannelSwitchUnit, nullptr);
	gMsgDispatcher.RegisterHandler(DBProtoChannelSwitchList, *this, &PlayerData::onChannelSwitchList, nullptr, new GmMsg::GmChannelSwitchInfo);
	// 内嵌数据同步失败的请求
	gMsgDispatcher.RegisterHandler(DBProtoGetFailedSyncMomoItem, *this, &PlayerData::onGetFailedSyncMomoItem, nullptr, new db::RepeatedStrRsp);
	gMsgDispatcher.RegisterHandler(DBProtoAddFailedSyncMomoItem, *this, &PlayerData::onAddFailedSyncMomoItem, new db::SyncMomoItemData, new db::SyncMomoItemData);
	gMsgDispatcher.RegisterHandler(DBProtoDelFailedSyncMomoItem, *this, &PlayerData::onDelFailedSyncMomoItem, new db::StrReq, nullptr);
	// 关注功能
	gMsgDispatcher.RegisterHandler(DBProtoFollowPlayer, *this, &PlayerData::followPlayer,
									new db::FollowPlayerReq, new db::FollowPlayerRsp);
	gMsgDispatcher.RegisterHandler(DBProtoRemoveFollowings, *this, &PlayerData::removeFollowings,
									new db::RepeatedUint32Req, nullptr);
	gMsgDispatcher.RegisterHandler(DBProtoRemoveFollowers, *this, &PlayerData::removeFollowers,
									new db::RepeatedUint32Req, nullptr);
	gMsgDispatcher.RegisterHandler(DBProtoAddFavFollowing, *this, &PlayerData::addFavFollowing,
									new db::Uint32Req, new db::Uint32Rsp);
	gMsgDispatcher.RegisterHandler(DBProtoRemoveFavFollowing, *this, &PlayerData::removeFavFollowing,
									new db::Uint32Req, nullptr);
	gMsgDispatcher.RegisterHandler(DBProtoAddFriendship, *this, &PlayerData::addFriendship,
									new db::RepeatedUint32Req, new db::ForMomoAddFriendShip);
	gMsgDispatcher.RegisterHandler(DBProtoAddToBlacklist, *this, &PlayerData::addToBlacklist,
								   new db::Uint32Req, new db::AddToBlackListRsp);
	gMsgDispatcher.RegisterHandler(DBProtoDelFromBlacklist, *this, &PlayerData::delFromBlacklist,
								   new db::Uint32Req, new db::Uint32Rsp);
	gMsgDispatcher.RegisterHandler(DBProtoCheckIfInBlacklist, *this, &PlayerData::chkIfInBlacklist, nullptr, new db::BoolRsp);

	// 活动功能
	gMsgDispatcher.RegisterHandler(DBProtoUpdateAcInfo, *this, &PlayerData::updatePlayerAccInfo, new db::UpdateAcInfoAttrReq, nullptr);
	gMsgDispatcher.RegisterHandler(DBProtoUpdateActivityInfo, *this, &PlayerData::updatePlayerActivityData, new db::UpdateActivityReq, nullptr);
	gMsgDispatcher.RegisterHandler(DBProtoGetPlayersAdventureRecord, *this, &PlayerData::getPlayersAdventureRecord, new cs::CSRepeatedUint32Req, new cs::PlayersAdventureRecord);

	// 任务相关
	gMsgDispatcher.RegisterHandler(DBProtoGetTaskGuid, *this, &PlayerData::getPlayerTaskGuid, new db::Uint32Req, new db::Uint32Rsp);
	gMsgDispatcher.RegisterHandler(DBProtoGetOldDriver, *this, &PlayerData::getPlayerTaskGuid, new db::Uint32Req, new db::Uint32Rsp);

	// 红点提醒
	gMsgDispatcher.RegisterHandler(DBProtoSetRedPointInfo, *this, &PlayerData::setPlayerRedPointInfo, new cs::SCRedPointInfo, new cs::SCRedPointInfo);

	// GM
	gMsgDispatcher.RegisterHandler(DBProtoGetGMPlayerData, *this, &PlayerData::getGMPlayerData, nullptr, new db::AllPlayerData);

	// 人气值、爱心值
	gMsgDispatcher.RegisterHandler(DBProtoAddLove, *this, &PlayerData::addLove, new db::AddLoveReq, new db::ForMomoAddLovePopular);
	gMsgDispatcher.RegisterHandler(DBProtoAddPopular, *this, &PlayerData::addPopular, new db::AddPopularReq, new db::ForMomoAddLovePopular);
	gMsgDispatcher.RegisterHandler(DBProtoSetMomoPopular, *this, &PlayerData::setMomoPopular, new db::Uint32Req, nullptr);
	gMsgDispatcher.RegisterHandler(DBProtoSetMomoLove, *this, &PlayerData::setMomoLove, new db::Uint32Req, nullptr);
	gMsgDispatcher.RegisterHandler(DBProtoSetMomoFriendShip, *this, &PlayerData::setMomoFriendShip, new center::MomoFriend, nullptr);
	gMsgDispatcher.RegisterHandler(DBProtoAddHomeItemPopular, *this, &PlayerData::addHomeItemPopular, new db::Uint32Req);
	gMsgDispatcher.RegisterHandler(DBProtoAddWealth, *this, &PlayerData::addWealth, new db::AddWealthReq, nullptr);
	// 膜拜
	gMsgDispatcher.RegisterHandler(DBProtoAddWorship, *this, &PlayerData::addWorship, new db::WorshipData, new db::Uint32Rsp);
	gMsgDispatcher.RegisterHandler(DBProtoDelWorship, *this, &PlayerData::delWorship, new db::RepeatedStrReq, nullptr);
	gMsgDispatcher.RegisterHandler(DBProtoGetWorship, *this, &PlayerData::getWorship, nullptr, new db::WorshipInfo);
	// 修改姓名、生日、位置
	gMsgDispatcher.RegisterHandler(DBProtoModifyNameBirthPos, *this, &PlayerData::modifyNameBirthPos, new cs::CSModifyNameBirthPos, new cs::CSModifyNameBirthPos);
	// 通知完成新手引导，加入今日登录玩家列表
	gMsgDispatcher.RegisterHandler(DBProtoCompleteNewbieGuide, *this, &PlayerData::completeNewbieGuide, new db::Uint32Req, nullptr);

	// 招募
	gMsgDispatcher.RegisterHandler(DBProtoGetRecruitInviteCode, *this, &PlayerData::getRecruitInviteCode, new db::Uint32Req, new cs::SCStringRsp);
	gMsgDispatcher.RegisterHandler(DBProtoFinishBeRecruited, *this, &PlayerData::finishBeRecruited, new db::StrReq, new db::PlayerRecruitInfo);
	gMsgDispatcher.RegisterHandler(DBProtoAddRecruiteScore, *this, &PlayerData::addRecruiteScore, new db::AddRecruiteScoreInfo, new db::AddRecruiteScoreInfo);

	// 送礼相关
	gMsgDispatcher.RegisterHandler(DBProtoGiveGift, *this, &PlayerData::giveGift, new db::GiveGiftInfo, new db::GiveGiftInfo);
	gMsgDispatcher.RegisterHandler(DBProtoGetGiftRecvRecord, *this, &PlayerData::getPlayerGiftRecvRecord, nullptr, new db::GiftRecvRecordInfo);
	gMsgDispatcher.RegisterHandler(DBProtoGetGiftRecvRank, *this, &PlayerData::getPlayerGiftRecvRank, nullptr, new db::GiftRecvRankInfo);
	gMsgDispatcher.RegisterHandler(DBProtoGetGiftRecordByPlid, *this, &PlayerData::getPlayerGiftRecordByPlid, new db::RepeatedUint32Req, new db::GiftRecvRankInfo);
	gMsgDispatcher.RegisterHandler(DBProtoGiftGemExchange, *this, &PlayerData::GiftGemExchange, new db::Uint32Req, new db::GiftGemExchangeRsp);

	// 拉取各System参数
	gMsgDispatcher.RegisterHandler(DBProtoGetShopParam, *this, &PlayerData::getShopParam, nullptr, new db::ShopParam);
	// 离线
	gMsgDispatcher.RegisterHandler(DBProtoPlayerOffline, *this, &PlayerData::offLine);
	// 拉取粉丝
	gMsgDispatcher.RegisterHandler(DBProtoGetMoreFans, *this, &PlayerData::getMoreFans, new cs::MoreFriendReq, new db::MultiPlayers);
	gMsgDispatcher.RegisterHandler(DBProtoGetPlayerInfoForStat, *this, &PlayerData::getPlayerInfoForStat, new db::StatReq, new db::PlayerDataForStat);
	gMsgDispatcher.RegisterHandler(DBProtoSetInteractInfo, *this, &PlayerData::onSetInteractInfo, new db::SetInteractInfo, new is::SyncInteractCntReq);
	gMsgDispatcher.RegisterHandler(DBProtoViewPlayerInfo, *this, &PlayerData::onViewPlayerInfo, nullptr, new cs::PlayerResume);
	gMsgDispatcher.RegisterHandler(DBProtoViewInteractInfo, *this, &PlayerData::onViewInteractInfo, new cs::InteractInfoReq, new cs::InteractInfoRsp);
	gMsgDispatcher.RegisterHandler(DBProtoUpdateOnlineInfo, *this, &PlayerData::updateOnlineInfo, new db::OnlineInfo);
	gMsgDispatcher.RegisterHandler(DBProtoArFaceIDCheck, *this, &PlayerData::checkArFaceId, new db::ARFaceIDReq, new db::ARFaceIDRsp);
	gMsgDispatcher.RegisterHandler(DBProtoArFaceQueryByPid, *this, &PlayerData::getArFaceInfo, new db::StrReq, new db::ARFaceIDCheckRsp);
	gMsgDispatcher.RegisterHandler(DBProtoArFaceTaskCheck, *this, &PlayerData::checkArFaceDailyTask, new db::ARFaceIDReq, new db::Uint32Rsp);
	gMsgDispatcher.RegisterHandler(DBProtoARFaceShowInfofbd, *this, &PlayerData::setARFaceShowInfofbd, new db::ARFaceShowInfofbdReq, nullptr);
	gMsgDispatcher.RegisterHandler(DBProtoARFaceIncrBy, *this, &PlayerData::setARFaceIncrBy, new db::ARFaceIDReq, nullptr);

	// 更新玩家时装信息
	gMsgDispatcher.RegisterHandler(DBProtoUpdateGarment, *this, &PlayerData::updateGarment, new db::GarmentData, nullptr);

	// 检查玩家是否预约
	gMsgDispatcher.RegisterHandler(DBProtoCheckBetaSubscribe, *this, &PlayerData::checkBetaSubscribe, new db::BetaAssistCheckReq, new db::BetaAssistCheckRsp);
	gMsgDispatcher.RegisterHandler(DBProtoSaveBetaPhone, *this, &PlayerData::onBetaPhoneSave, new cs::BetaPhone, nullptr);
	gMsgDispatcher.RegisterHandler(DBProtoUpdatePlayerBeta, *this, &PlayerData::updatePlayerBeta, new cs::BetaAssist, nullptr);

	gMsgDispatcher.RegisterHandler(DBProtoUpdateGarWearing, *this, &PlayerData::onUpdatePlayerGarWearing, new cs::GarmentWearing, nullptr);
	gMsgDispatcher.RegisterHandler(DBProtoUpdateActTaskInfo, *this, &PlayerData::onUpdateAcTaskInfo, new db::DBActTaskInfo, nullptr);
	gMsgDispatcher.RegisterHandler(DBProtoUpdateHutInfo, *this, &PlayerData::onUpdateHutInfo, new cs::HutInfo, nullptr);

	gMsgDispatcher.RegisterHandler(DBProtoUpdatePlayerCollectCache, *this, &PlayerData::onUpdatePlayerCollectCacheInfo, new db::PlayerCollectCacheInfo, nullptr);

	gMsgDispatcher.RegisterHandler(DBProtoGetPlayerMonsterInfo, *this, &PlayerData::getPlayerMonsterInfo, new db::Uint32Req, new db::MonsterInfo);

	gMsgDispatcher.RegisterHandler(DBProtoGetActGlobalInfo, *this, &PlayerData::getActGlobalInfo, nullptr, new cs::ActivityGlobalInfo);
	gMsgDispatcher.RegisterHandler(DBProtoUpdateActGlobalInfo, *this, &PlayerData::updateActGlobalInfo, new cs::ActivityGlobalInfo, nullptr);

	gMsgDispatcher.RegisterHandler(DBProtoGiftGive, *this, &PlayerData::giftGivenChannel, new db::StrReq, new db::Uint32Rsp);
	gMsgDispatcher.RegisterHandler(DBProtoGetPlayerExtraInfo, *this, &PlayerData::getPlayerExtraInfo, nullptr, new db::PlayerExtraInfo);
	gMsgDispatcher.RegisterHandler(DBProtoSetOperatorVer, *this, &PlayerData::setOperatorVer, new db::Uint32Req, nullptr);
	// 清理玩家签名
	gMsgDispatcher.RegisterHandler(DBProtoChangePlayerData, *this, &PlayerData::onChangePlayerData, new GmMsg::GmChangePlayerDataReq, nullptr);
	// 拉取玩家登陆记录
	gMsgDispatcher.RegisterHandler(DBProtoGetLoginRecords, *this, &PlayerData::onGetLoginRecords, new cs::GmLoginRecordReq, new cs::GmLoginRecordRsp);
	// 拉取玩家账号绑定信息
	gMsgDispatcher.RegisterHandler(DBProtoGetBindInfo, *this, &PlayerData::onGetBindInfo, nullptr, new cs::GmPlayerBindInfo);
	// 增加玩家抽卡记录
	gMsgDispatcher.RegisterHandler(DBProtoAddLotteryRecord, *this, &PlayerData::onAddLotteryRecord, new cs::GmLotteryRecord, nullptr);
	// 拉取玩家抽卡记录
	gMsgDispatcher.RegisterHandler(DBProtoGetLotteryRecords, *this, &PlayerData::onGetLotteryRecords, new cs::GmLotteryRecordReq, new cs::GmLotteryRecordRsp);
	// 增加spam命中记录
	gMsgDispatcher.RegisterHandler(DBProtoAddSpamRecord, *this, &PlayerData::onAddSpamRecord, new cs::GmSpamRecord, nullptr);
	// 拉取spam命中记录
	gMsgDispatcher.RegisterHandler(DBProtoGetSpamRecords, *this, &PlayerData::onGetSpamRecords, new cs::GmSpamRecordReq, new cs::GmSpamRecordRsp);
	// 更新spam记录状态
	gMsgDispatcher.RegisterHandler(DBProtoUpdateSpamState, *this, &PlayerData::onUpdateSpamState, new cs::GmSpamUpdateReq, new cs::GmSpamRecordRsp);


	maxPlayerID_ = config_get_intval("max_player_id", 0);
}

void PlayerData::Init()
{
	/*avatarMap_.clear();
	for (auto avIt : gCSVAvatar.AllItems) {
		uint32_t key = avIt.second->Head;
		if (key == 0) {
			key = 100000000 + avIt.second->Gender;
		}
		avatarMap_.emplace(key, avIt.first);
	}*/
}

ErrCodeType PlayerData::CheckIfInBlacklist(uint32_t plid1, uint32_t plid2)
{
	bool inBlacklist;
	// if (!gRedis->sismember(makeBlacklistKey(plid1), to_string(plid2), inBlacklist)) {
	if (!gRedis->hexists(makeBlacklistKey(plid1), to_string(plid2), inBlacklist)) {
		DEBUG_LOG("Check if in blacklist failed! plid=%u target=%u err=%s",
				  plid2, plid1, gRedis->last_error_cstr());
		return ErrCodeDB;
	}
	if (inBlacklist) {
		return ErrCodeYouInBlacklist;
	}
	return ErrCodeSucc;
}

int PlayerData::CheckIfIsHatching(uint32_t plid)
{
	string key = kKeyPrefixPlayerData + to_string(plid);
	unordered_map<string, string> fields;
	const auto& hatchStr = fields[kHatchInfo];
	if (!gRedis->hget(key, fields)) {
		WARN_LOG("hget failed!");
		return -1;
	}

	if (hatchStr.empty()) {
		return 0;
	}

	db::HatchingData hInfo;
	hInfo.ParseFromString(hatchStr);
	return hInfo.hatching();
}


void PlayerData::GetBlackList(uint32_t uid, ::google::protobuf::RepeatedField<uint32_t>& list)
{
	unordered_map<string, string> bmap;
	gRedis->hgetall(makeBlacklistKey(uid), bmap);
	for (auto& it : bmap) {
		list.Add(atoi(it.first.c_str()));
	}
}

//----------------------------------------------------------------------
// Private Methods
//----------------------------------------------------------------------
static const string kScriptSavePlayerIdDevice =
		"redis.call('SADD', KEYS[1], ARGV[1])\n"
		"local r = redis.call('SCARD', KEYS[1])\n"
		"if r >= 1 then\n"
		"  redis.call('HSET', KEYS[2], ARGV[2], r)\n"
		"end\n"
		"return 0";

ErrCodeType PlayerData::getRealPlayerID(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::GetRealPlayerIDReq, req);
	REAL_PROTOBUF_MSG(outMsg, db::GetRealPlayerIDRsp, rsp);
	//uint32_t channelID = req.u32(0);
	//uint32_t userID = req.u32(1);

	//一个玩家登录时使用的唯一标识符是LG+渠道号+'#'+accountid
	// string key(kKeyPrefixLogin + req.reg_info().account().account_type() + kLoginSplit + req.reg_info().account().account_id());
	string key = makeLoginKey(req.reg_info().account().account_type(), req.reg_info().account().account_id());
	bool exist = false;
	string value;
	if (!gRedis->get(key, value, &exist)) {
		WARN_LOG("exist failed: %s!", gRedis->last_error_cstr());
		return ErrCodeDB;
	}
	DEBUG_LOG("key %s %s", key.c_str(), value.c_str());

	if (exist) {
		rsp.set_player_id(atoi(value.c_str()));
		unordered_map<string, string> m;

#if 0		// 将来可能有用
		if (req.reg_info().account().account_type() != "momo") {
			string key(kKeyPrefixPlayerData + value);
			db::PlayerData  data;
			unordered_map<string, string> tempm;
			tempm[kRegInfo];
			if (!gRedis->hget(key, tempm)) {
				WARN_LOG("hgetall failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
				return ErrCodeDB;
			}
			data.mutable_reg_info()->ParseFromString(tempm[kRegInfo]);
			if (data.reg_info().third_userid() == ""
					&& req.reg_info().third_userid() != "") {
				data.mutable_reg_info()->set_third_userid(req.reg_info().third_userid());
				data.mutable_reg_info()->SerializeToString(&(m[kRegInfo]));
				if (!gRedis->hset(key, tempm)) {
					WARN_LOG("hgetall failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
					return ErrCodeDB;
				}
			}
		}
#endif



#if 0
		db::PlayerData	data;
		data.mutable_reg_info()->CopyFrom(req.reg_info());
		data.mutable_monster_bag()->set_next_guid(0);
		data.mutable_monster_bag()->SerializeToString(&(m[kMonsterBag]));
		data.mutable_reg_info()->SerializeToString(&(m[kRegInfo]));
		string playerkey(kKeyPrefixPlayerData + value);
		DEBUG_LOG("playerkey %s", playerkey.c_str());
		if (!gRedis->hset(playerkey, m)) {
			WARN_LOG("player data init failed: %s!", gRedis->last_error_cstr());
			return ErrCodeDB;
		}
#endif

		if (!gRedis->hexists(makeBasicExtraDataKey(to_string(rsp.player_id())), 
					kBEDataFieldBanLogin, exist)) {
			WARN_LOG("exit Ban failed! plid=%u err=%s",
						rsp.player_id(), gRedis->last_error_cstr());
			return ErrCodeDB;
		}

		uint32_t banflag = 0;
		if (exist) {
			m[kBEDataFieldBanLogin];
			if (!gRedis->hget(makeBasicExtraDataKey(to_string(rsp.player_id())), m)) {
				WARN_LOG("Get player Ban failed! plid=%u err=%s",
						rsp.player_id(), gRedis->last_error_cstr());
				return ErrCodeDB;
			}
		}
		auto banLoginTime = atoi(m[kBEDataFieldBanLogin].c_str());
		banflag = 1;
		// 判断设备是否被封
		if (req.reg_info().device().did() != "") {
			m.clear();
			auto& banDeviceLoginTimeStr = m[req.reg_info().device().did()];
			if (!gRedis->hget(kMiscFieldBanLoginDevice, m)) {
				WARN_LOG("hget failed: %s! plid=%u", gRedis->last_error_message().c_str(), rsp.player_id());
				return ErrCodeDB;
			}
			auto banDeviceLoginTime = atoi(banDeviceLoginTimeStr.c_str());
			if (banDeviceLoginTime > banLoginTime) {
				banLoginTime = banDeviceLoginTime;
				banflag = 1;
			}
		}
		// 判断ip是否被封
		if (req.reg_info().device().ip()) {
			m.clear();
			auto& banIPLoginTimeStr = m[TransIP(req.reg_info().device().ip())];
			if (!gRedis->hget(kMiscFieldBanLoginIP, m)) {
				WARN_LOG("hget failed: %s! plid=%u", gRedis->last_error_message().c_str(), rsp.player_id());
				return ErrCodeDB;
			}
			auto banIPLoginTime = atoi(banIPLoginTimeStr.c_str());
			if (banIPLoginTime > banLoginTime) {
				banLoginTime = banIPLoginTime;
				banflag = 1;
			}
		}
		// 判断渠道是否被封
		if (req.reg_info().channel() != "") {
			m.clear();
			auto& banChannelLoginTimeStr = m[req.reg_info().channel()];
			if (!gRedis->hget(kMiscFieldBanChannelLogin, m)) {
				WARN_LOG("hget failed: %s! plid=%u", gRedis->last_error_message().c_str(), rsp.player_id());
				return ErrCodeDB;
			}
			auto banChannelLoginTime = atoi(banChannelLoginTimeStr.c_str());
			if (banChannelLoginTime > banLoginTime) {
				banLoginTime = banChannelLoginTime;
				banflag = 2;
			}
		}
		rsp.set_ban_flag(time(0) < banLoginTime ? banflag : 0);
		rsp.set_ban_time(banLoginTime);
	} else {
		long long uID = 0;
		gRedis->incrby(kKeyPLayerGUID, 1, &uID);
		uID += kPlayerIDOffset;
		uint32_t realUID = uID;
		if (realUID < uID) {
			WARN_LOG("ERROR EXCEED PLAYER UINT32_T LENGTH");
			return ErrCodeDB;
		}
		if (maxPlayerID_ > 0 && realUID >= maxPlayerID_) {
			WARN_LOG("ERROR EXCEED PLAYER ID %u %u", realUID, maxPlayerID_);
			return ErrCodePlayerIDHasFull;
		}

		// const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg
		
		unordered_map<string, string> m;
		db::PlayerData	data;
		data.mutable_reg_info()->CopyFrom(req.reg_info());
		data.mutable_monster_bag()->set_next_guid(0);
		data.mutable_monster_bag()->SerializeToString(&(m[kMonsterBag]));
		data.mutable_reg_info()->SerializeToString(&(m[kRegInfo]));
		string playerkey(kKeyPrefixPlayerData + to_string(realUID));
		if (!gRedis->hset(playerkey, m)) {
			WARN_LOG("player data init failed: %s!", gRedis->last_error_cstr());
			return ErrCodeDB;
		}
		if (req.reg_info().device().did() != "") {
			vector<string> keys = { makePlayerOfDeviceKey(req.reg_info().device().did()), kMiscFieldDubiousDevice };
			vector<string> args = { to_string(realUID), req.reg_info().device().did() };
			ScopedReplyPointer reply = gRedis->eval(kScriptSavePlayerIdDevice, &keys, &args);
			CHECK_REPLY_EC(reply, REDIS_REPLY_INTEGER, realUID);
		}
		if (req.reg_info().device().ip()) {
			vector<string> keys = { makePlayerOfIPKey(TransIP(req.reg_info().device().ip())), kMiscFieldDubiousIP };
			vector<string> args = { to_string(realUID), TransIP(req.reg_info().device().ip()) };
			ScopedReplyPointer reply = gRedis->eval(kScriptSavePlayerIdDevice, &keys, &args);
			CHECK_REPLY_EC(reply, REDIS_REPLY_INTEGER, realUID);
		}
#if 0
		SSProtoHead fh;
		fh.PlayerID = realUID;
		fh.TargetID = realUID;
		fh.OrigProtoID = h.OrigProtoID;
		fh.ProtoID = h.ProtoID;
		ErrCodeType ret = setPlayerData(fh, &data, nullptr);
		if (ret != ErrCodeSucc) {
			return ret;
		}
#endif

		if (!gRedis->set(key, to_string(realUID))) {
			WARN_LOG("exists failed: %s!", gRedis->last_error_cstr());
			return ErrCodeDB;
		}
		rsp.set_player_id(realUID);
		DEBUG_LOG("getRealPlayerID key=%s uid=%u", key.c_str(), realUID);
		//新号是不可能被禁的
	}

	std::string thirdkey = req.reg_info().account().account_type() + "#" + req.reg_info().third_userid();
	if (!gRedis->hexists(kKeyChannelUserIdKey, thirdkey, exist)) {
		WARN_LOG("hexists failed! plid=%u err=%s",h.TargetID, gRedis->last_error_cstr());
		return ErrCodeDB;
	}
	if (!exist) {
		std::unordered_map<string, string> m;
		m[thirdkey] = to_string(rsp.player_id());
		if (!gRedis->hset(kKeyChannelUserIdKey, m)) {
			WARN_LOG("hexists failed! plid=%u err=%s",h.TargetID, gRedis->last_error_cstr());
			return ErrCodeDB;
		}
	}

	if (req.has_info()) {
		string queryKey(kKeyPrefixExtrPhotos + to_string(rsp.player_id()));
		string value;
		req.info().SerializeToString(&value);
		if (!gRedis->set(queryKey, value)) {
			WARN_LOG("exists failed: %s!", gRedis->last_error_cstr());
			return ErrCodeDB;
		}

	}

	return ErrCodeSucc;
}

ErrCodeType PlayerData::delRealPlayerID(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	// kRegInfo
	REAL_PROTOBUF_MSG(inMsg, db::Uint32Req, req);
	REAL_PROTOBUF_MSG(outMsg, db::Uint32Rsp, rsp);
	string queryKey(kKeyPrefixPlayerData + to_string(req.u32()));
	unordered_map<string, string> m;
	m[kRegInfo];
	DEBUG_LOG("delRealPlayerID %s", queryKey.c_str());
	if (!gRedis->hget(queryKey, m)) {
		WARN_LOG("hget failed: %s! plid=%u", gRedis->last_error_cstr(), req.u32());
		return ErrCodeDB;
	}
	db::RegInfo reginfo;
	reginfo.ParseFromString(m[kRegInfo]);
	//一个玩家登录时使用的唯一标识符是LG+渠道号+'#'+accountid
	// string key(kKeyPrefixLogin + reginfo.account().account_type() + kLoginSplit + reginfo.account().account_id());
	string key = makeLoginKey(reginfo.account().account_type(), reginfo.account().account_id());
	long long cnt;
	std::vector<std::string> keys{key};
	DEBUG_LOG("delRealPlayerID %s", key.c_str());
	if (!gRedis->del(keys, &cnt)) {
		WARN_LOG("del failed: %s!", gRedis->last_error_cstr());
		return ErrCodeDB;
	}
	if (cnt) {
		unordered_map<string, string> m;
		m[queryKey] = queryKey;
		if (!gRedis->hset(kKeyDelPlayers, m)) {
			WARN_LOG("hset failed: %s!", gRedis->last_error_cstr());
			return ErrCodeDB;
		}

	}

	rsp.set_u32(0);
	return ErrCodeSucc;	
}

ErrCodeType PlayerData::getAccountInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(outMsg, cs::Account, rsp);
	string regKey(kKeyPrefixPlayerData + to_string(h.PlayerID));
	unordered_map<string, string> mReg;
	auto& regInfoStr = mReg[kRegInfo];
	if (!gRedis->hget(regKey, mReg)) {
		WARN_LOG("hget failed: %s! plid=%u", gRedis->last_error_message().c_str(), h.PlayerID);
		return ErrCodeDB;
	}
	db::RegInfo regInfo;
	regInfo.ParseFromString(regInfoStr);
	rsp.Swap(regInfo.mutable_account());
	return ErrCodeSucc;
}

ErrCodeType PlayerData::getPlayerRegionInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::GetPlayerRegionInfoReq, req);
	REAL_PROTOBUF_MSG(outMsg, db::GetPlayerRegionInfoRsp, rsp);

	TblPlayerRegion tblPlayerRegion(0);
	int ret = tblPlayerRegion.GetPlayerRegionInfo(req.channel(), req.channel_id(), rsp.mutable_infos());
	if (ret != ErrCodeSucc) {
		return ErrCodeDB;
	}

	rsp.set_channel(req.channel());
	rsp.set_channel_id(req.channel_id());
	return ErrCodeSucc;
}

ErrCodeType PlayerData::getGMPlayerID(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::QueryPlayerIDReq, req);
	REAL_PROTOBUF_MSG(outMsg, db::QueryPlayerIDRsp, rsp);
	string playerID;
	bool keyExist;
	for (int i = 0; i < req.query_strings_size(); ++i) {
		switch (req.query_type()) {
		case db::QueryPlayerID: {
			string queryKey(kKeyPrefixPlayerData + req.query_strings(i));
			if (!gRedis->exists(queryKey, keyExist)) {
				WARN_LOG("exists failed: %s!", gRedis->last_error_message().c_str());
				return ErrCodeDB;
			}
			playerID = keyExist ? req.query_strings(i) : "0";
			break;
		}
		case db::QueryNickName: {
			/*playerID = "0";
			string queryKey(kKeyPrefixNickname + req.query_strings(i));
			if (!gRedis->get(queryKey, playerID, &keyExist)) {
				WARN_LOG("get failed: %s!", gRedis->last_error_message().c_str());
				return ErrCodeDB;
			}*/
			uint32_t orgPlid = 0;
			string queryKey(req.query_strings(i));
			TblPlayerNick tblNick(0);
			if (tblNick.CheckNick(queryKey, orgPlid, keyExist) != ErrCodeSucc) {
				return ErrCodeDB;
			}
			playerID = to_string(orgPlid);
			break;
		}
		case db::QueryMomoID: {
			playerID = "0";
			string queryKey = makeLoginKey(kChannelNameMomo, req.query_strings(i));
			// string queryKey(kKeyPrefixLogin + "momo" + kLoginSplit + req.query_strings(i));
			if (!gRedis->get(queryKey, playerID, &keyExist)) {
				WARN_LOG("get failed: %s!", gRedis->last_error_message().c_str());
				return ErrCodeDB;
			}
			break;
		}
		case db::QueryBanState: {
			playerID = checkPlayerIsBan(atoi(req.query_strings(i).c_str())) ? "1" : "2";
			break;
		}
		case db::QueryPIDByOpenID: {
			playerID = "0";

			string querykey = req.query_channel() + "#" + req.query_strings(i);
			unordered_map<string, string> m;
			auto& pid = m[querykey];
			bool exist;
			if (!gRedis->hexists(kKeyChannelUserIdKey, querykey, exist)) {
				WARN_LOG("get failed: %s!", gRedis->last_error_message().c_str());
				return ErrCodeDB;
			}
			if (exist) {
				if (!gRedis->hget(kKeyChannelUserIdKey, m)) {
					WARN_LOG("get failed: %s!", gRedis->last_error_message().c_str());
					return ErrCodeDB;
				}
				playerID = pid;
			}
			break;
		}
		default:
			break;
		}

		auto queryRet = rsp.add_player_ids();
		queryRet->set_query_string(req.query_strings(i));
		queryRet->set_player_id(atoi(playerID.c_str()));
	}
	DEBUG_LOG("getGMPlayerID, rsp=%s", rsp.Utf8DebugString().c_str());
	return ErrCodeSucc;
}

ErrCodeType PlayerData::getPlayersByMomoID(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::RepeatedStrReq, req);
	REAL_PROTOBUF_MSG(outMsg, cs::RepeatedPlayerWithName, rsp);
	string playerID;
	bool keyExist;
	for (int i = 0; i < req.strs_size(); ++i) {
		playerID = "0";
		string queryKey = makeLoginKey(kChannelNameMomo, req.strs(i));
		if (!gRedis->get(queryKey, playerID, &keyExist)) {
			WARN_LOG("get failed: %s!", gRedis->last_error_message().c_str());
			return ErrCodeDB;
		}

		auto queryRet = rsp.add_datas();
		queryRet->set_momo_id(req.strs(i));
		queryRet->set_player_id(atoi(playerID.c_str()));
		if (keyExist) {
			string key(kKeyPrefixPlayerData + playerID);
			unordered_map<string, string> m;
			auto& accInfoStr = m[kAccountInfo];
			if (!gRedis->hget(key, m)) {
				WARN_LOG("hget failed: %s! plid=%u target=%s", gRedis->last_error_cstr(), h.PlayerID, playerID.c_str());
				return ErrCodeDB;
			}

			if (accInfoStr.empty()) {
				DEBUG_LOG("Player not found. plid=%u target=%s", h.PlayerID, playerID.c_str());
				continue;
			}

			db::AccountInfo accInfo;
			accInfo.ParseFromString(accInfoStr);
			queryRet->set_player_name(accInfo.player_name());

			setPlayerDataTouchInfo(atol(playerID.c_str()), kKeyPrefixPlayerData, false);
		}
	}
	return ErrCodeSucc;
}

ErrCodeType PlayerData::getPlayersLvByMomoID(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::RepeatedStrReq, req);
	REAL_PROTOBUF_MSG(outMsg, db::QueryPlayerIDRsp, rsp);
	string playerID;
	bool keyExist;
	for (int i = 0; i < req.strs_size(); ++i) {
		playerID = "0";
		string queryKey = makeLoginKey(kChannelNameMomo, req.strs(i));
		if (!gRedis->get(queryKey, playerID, &keyExist)) {
			WARN_LOG("get failed: %s!", gRedis->last_error_message().c_str());
			return ErrCodeDB;
		}

		auto queryRet = rsp.add_player_ids();
		queryRet->set_query_string(req.strs(i));
		if (keyExist) {
			string key(kKeyPrefixPlayerData + playerID);
			unordered_map<string, string> m;
			auto& attrInfoStr = m[kPlayerAttr];
			if (!gRedis->hget(key, m)) {
				WARN_LOG("hget failed: %s! plid=%u target=%s", gRedis->last_error_cstr(), h.PlayerID, playerID.c_str());
				return ErrCodeDB;
			}

			if (attrInfoStr.empty()) {
				DEBUG_LOG("Player not found. plid=%u target=%s", h.PlayerID, playerID.c_str());
				continue;
			}

			cs::PlayerAttr attrInfo;
			attrInfo.ParseFromString(attrInfoStr);
			queryRet->set_player_id(attrInfo.lv()); // 用这个message，该字段实际为lv
			
			setPlayerDataTouchInfo(atol(playerID.c_str()), kKeyPrefixPlayerData, false);
		}
	}
	return ErrCodeSucc;
}

ErrCodeType PlayerData::getCorpSnapShot(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::RepeatedUint32Req, req);
	db::ListCorpInfoRsp tmpRsp;
	for (auto i : req.u32()) {
		gCorpsData.getCorpsInfo(i, *(tmpRsp.add_infos()));
	}

	REAL_PROTOBUF_MSG(outMsg, db::CorpSnapShotRsp, rsp);
	for (auto info : tmpRsp.infos()) {
		if (!info.info().lv()) {
			continue;
		}
		auto snapCorp = rsp.add_snap_infos();
		snapCorp->set_appid("mm_smbb_fQnrTwYj");
		snapCorp->set_type("corps");
		unordered_map<string, string> mReg;
		auto& regInfoStr = mReg[kRegInfo];
		string regKey(kKeyPrefixPlayerData + to_string(info.info().create_plid()));
		if (!gRedis->hget(regKey, mReg)) {
			WARN_LOG("hget failed: %s! plid=%u", gRedis->last_error_message().c_str(), info.info().create_plid());
			continue;
		}
		if (!regInfoStr.empty()) {
			db::RegInfo regInfo;
			regInfo.ParseFromString(regInfoStr);
			snapCorp->set_channel(regInfo.channel());
			snapCorp->set_os(regInfo.device().os());
		} else {
			snapCorp->set_channel("");
			snapCorp->set_os("");
		}
		snapCorp->set_family_id(to_string(info.info().corps_id()));
		char fmtTime[20];
		time_t ts(info.info().create_time());
		strftime(fmtTime,sizeof(fmtTime),"%Y-%m-%d %H:%M:%S",localtime(&ts));
		snapCorp->set_time(fmtTime);
		snapCorp->set_level(to_string(info.info().lv()));
		snapCorp->set_server("0");
		snapCorp->set_status("1");
		snapCorp->set_owner(to_string(info.info().owner_plid()));
		snapCorp->set_number(to_string(info.players_size()));
		snapCorp->mutable_ext();
	}
	return ErrCodeSucc;
}

ErrCodeType PlayerData::getSnapShot(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::SnapShotReq, req);
	REAL_PROTOBUF_MSG(outMsg, db::SnapShotRsp, rsp);
	for (uint32_t i = req.start_player_id(); i <= req.end_player_id(); i++) {
		SSProtoHead& hh = const_cast<SSProtoHead&>(h);

		uint32_t playerID = i;
		if (req.today_logined()) {
			unordered_map<string, string> m;
			auto& str_pid = m[to_string(i)];
			if (!gRedis->hget(kKeyPrefixAllTodayLoginedPlayersSnapshot, m)) {
				WARN_LOG("getGMPlayerData, today_logined, hget failed: %s! plid=%u", gRedis->last_error_cstr(), playerID);
				continue;
			}
			playerID = atoi(str_pid.c_str());
		}

		hh.TargetID = playerID;
		db::AllPlayerData allData;
		ErrCodeType errCode = getGMPlayerData(hh, nullptr, &allData);
		if (errCode) {
			WARN_LOG("getGMPlayerData failed: %s! plid=%u", gRedis->last_error_cstr(), playerID);
			continue;
		}
		auto snapInfo = rsp.add_snap_infos();
		snapInfo->set_appid("mm_smbb_fQnrTwYj");
		snapInfo->set_type("user");
		snapInfo->set_channel(allData.player_data().reg_info().channel());
		snapInfo->set_server(string("3-") + to_string(req.region()));
		// snapInfo->set_os(allData.player_data().online_stat_info().last_device().os());
		snapInfo->set_os(allData.player_data().reg_info().device().os());
		snapInfo->set_game_id(allData.player_data().reg_info().account().account_id());
		snapInfo->set_uid("");
		snapInfo->set_role_id(to_string(playerID));
		snapInfo->set_status("1");
		// snapInfo->mutable_device_info()->set_did(allData.player_data().online_stat_info().last_device().did());
		// snapInfo->mutable_device_info()->set_mac(allData.player_data().online_stat_info().last_device().mac());
		// snapInfo->mutable_device_info()->set_ip(TransIP(allData.player_data().online_stat_info().last_device().ip()));
		// snapInfo->mutable_device_info()->set_model(allData.player_data().online_stat_info().last_device().model());
		// snapInfo->mutable_device_info()->set_rom(allData.player_data().online_stat_info().last_device().rom());
		snapInfo->mutable_device_info()->set_did(allData.player_data().reg_info().device().did());
		snapInfo->mutable_device_info()->set_mac(allData.player_data().reg_info().device().mac());
		snapInfo->mutable_device_info()->set_ip(TransIP(allData.player_data().reg_info().device().ip()));
		snapInfo->mutable_device_info()->set_model(allData.player_data().reg_info().device().model());
		snapInfo->mutable_device_info()->set_rom(allData.player_data().reg_info().device().rom());
		snapInfo->mutable_user_info()->set_role_name(allData.player_data().account_info().player_name());
		snapInfo->mutable_user_info()->set_role_ct(to_string(allData.player_data().reg_info().reg_time()));
		snapInfo->mutable_user_info()->set_pay_type(allData.pay_order().finished_orders_size() > 0 ? "1" : "0");
		snapInfo->mutable_user_info()->set_sex(to_string(allData.player_data().account_info().gender()));
		//拉去陌陌玩家性别
		bool exist = false;
		string key1(kKeyPrefixExtrPhotos + to_string(playerID));
		string value;
		if (!gRedis->get(key1, value, &exist)) {
			WARN_LOG("exists failed: %s! plid=%u", gRedis->last_error_cstr(), playerID);
			continue;
		}

		if (exist) {
			cs::ExtraInfo extra;
			extra.ParseFromString(value);
			snapInfo->mutable_user_info()->set_user_type(extra.sexual() ? "F" : "M");
		} else {
			snapInfo->mutable_user_info()->set_user_type("");
		}
		snapInfo->mutable_user_info()->set_user_level(to_string(allData.player_data().attr_info().lv()));
		char fmtTime[20];
		time_t ts(allData.player_data().online_stat_info().last_online_time());
		strftime(fmtTime,sizeof(fmtTime),"%Y-%m-%d %H:%M:%S",localtime(&ts));
		snapInfo->mutable_user_info()->set_last_login(fmtTime);
		snapInfo->mutable_user_info()->set_last_login_ip(TransIP(allData.player_data().online_stat_info().last_device().ip()));
		snapInfo->mutable_resource_info()->set_diamond(to_string(allData.player_data().asset().cur_asset().diamonds()));
		snapInfo->mutable_resource_info()->set_total_diamond(to_string(allData.player_data().asset().hist_asset().diamonds()));
		snapInfo->mutable_resource_info()->set_gold(to_string(allData.player_data().asset().cur_asset().coins()));
		snapInfo->mutable_resource_info()->set_total_gold(to_string(allData.player_data().asset().hist_asset().coins()));
		snapInfo->mutable_resource_info()->set_color_brick(to_string(allData.player_data().asset().cur_asset().gems()));
		snapInfo->mutable_resource_info()->set_total_brick(to_string(allData.player_data().asset().hist_asset().gems()));
		snapInfo->mutable_resource_info()->set_ticket(to_string(allData.player_data().asset().cur_asset().lottery_tickets()));
		snapInfo->mutable_resource_info()->set_total_ticket(to_string(allData.player_data().asset().hist_asset().lottery_tickets()));
		snapInfo->mutable_resource_info()->set_digital_egg(to_string(allData.player_data().monster_bag().monster_eggs_size()));
		float totalMoney = 0;
		for (auto order : allData.pay_order().finished_orders()) {
			totalMoney += order.total_fee();
		}
		stringstream stream;
		stream << fixed << setprecision(2) << totalMoney;
		snapInfo->mutable_resource_info()->set_total_money(stream.str());
		for (int j = 0; j < allData.player_data().monster_bag().monsters_size(); ++j) {
			auto mon = allData.player_data().monster_bag().monsters(j);
			auto pet = snapInfo->add_pet_info();
			pet->set_id(to_string(mon.id()));
			pet->set_uid(to_string(mon.guid()));
			pet->set_level(to_string(mon.lv()));
			pet->set_star(to_string(mon.star()));
			auto itemCfg = gCSVMonster.GetItem(mon.id());
			if (itemCfg) {
				pet->set_elem_type(to_string(itemCfg->ElemType));
				pet->set_rarity(to_string(itemCfg->Rarity));
				pet->set_stage(to_string(itemCfg->Stage));
			} else {
				pet->set_elem_type("0");
				pet->set_rarity("0");
				pet->set_stage("0");
			}
		}
		for (int j = 0; j < allData.player_data().common_item_bag().items_size(); ++j) {
			auto item = allData.player_data().common_item_bag().items(j);
			auto tool = snapInfo->add_tools();
			tool->set_id(to_string(cs::ResourceTypeCommonItem) + "-" + to_string(item.id()));
			tool->set_number(to_string(item.cnt()));
			tool->set_type(to_string(cs::ResourceTypeCommonItem));
		}
		// 核心水晶
		unordered_map<string, string> mapGarment;
		if (!gRedis->hgetall(makeGarmentKey(to_string(playerID)), mapGarment)) {
			WARN_LOG("hgetall failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
			continue;
		}
		for (const auto& it : mapGarment) {
			if (it.first.find(kKeyGar_Garments) != string::npos) {
				cs::GarmentUnit garment;
				garment.ParseFromString(it.second); 
				auto tool = snapInfo->add_tools();
				tool->set_id(to_string(cs::ResourceTypeEquip) + "-" + to_string(garment.gar_id()));
				tool->set_number("1");
				tool->set_type(to_string(cs::ResourceTypeEquip));
			}
		}

		// 特别关注
		long long favFollowingsCnt = 0;
		if (!gRedis->scard(makeFavFollowingKey(playerID), favFollowingsCnt)) {
			WARN_LOG("scard failed: %s! plid=%u", gRedis->last_error_cstr(), playerID);
			continue;
		}
		// 关注
		unordered_map<string, string> followings;
		if (!gRedis->hgetall(makeFollowingKey(playerID), followings)) {
			WARN_LOG("hgetall failed: %s! plid=%u", gRedis->last_error_cstr(), playerID);
			continue;
		}
		// 互相关注
		vector<string> intersection;
		if (!gRedis->smembers(makeIntersectionKey(playerID), intersection)) {
			WARN_LOG("smembers failed: %s! plid=%u", gRedis->last_error_cstr(), playerID);
			continue;
		}
		cs::ArenaInfo arenaInfo;
		gArena.GetInfoImp(h.TargetID, arenaInfo);
		int maxFirendship = 0;
		unordered_map<int, int> freindLvCnt;
		for (auto friendID : intersection) {
			auto it = followings.find(friendID);
			if (it != followings.end()) {
				int friendship = atoi(it->second.c_str());
				if (maxFirendship < friendship) {
					maxFirendship = friendship;
				}
				for (int lv = 1; lv <= (int)gCSVFriendLvExp.AllItems.size(); lv++) {
					auto lvItm = gCSVFriendLvExp.GetItem(lv);
					if (!lvItm) {
						break;
					}
					friendship -= lvItm->Exp;
					if (lvItm->Exp <= 0 || friendship <= 0) {
						auto lvIt = freindLvCnt.find(lv);
						if (lvIt != freindLvCnt.end()) {
							freindLvCnt[lv] += 1;
						} else {
							freindLvCnt[lv] = 1;
						}
						break;
					}
				}
			}
		}
		snapInfo->mutable_user_info()->set_friendliness(to_string(maxFirendship));
		snapInfo->mutable_user_info()->set_special_focus(to_string(favFollowingsCnt));
		snapInfo->mutable_user_info()->set_space_id(to_string(arenaInfo.region()));
		/*string decorateKey = homeDecorateData.makeDecorateKey(playerID);
		unordered_map<string, string> mDecorate;
		string& homeLvStr = mDecorate[kDecorateLevelFiled];
		if (!gRedis->hget(decorateKey, mDecorate)) {
			WARN_LOG("hgetall failed: %s! plid=%u", gRedis->last_error_cstr(), playerID);
			continue;
		}*/
		string homeLvStr = to_string(homeDecorateData.getHomeLevel(playerID));
		snapInfo->mutable_user_info()->set_family_level(homeLvStr);

		uint32_t totalBuildingCnt = allData.player_data().home_buildings().buildings_size() + allData.player_data().home_buildings().in_buildings_size();
		snapInfo->mutable_resource_info()->set_build_number(to_string(totalBuildingCnt));
		uint32_t totalOrnamentCnt = 0;
		for (auto home_item : allData.player_data().home_items().items()) {
			totalOrnamentCnt += home_item.total_cnt();
		}
		snapInfo->mutable_resource_info()->set_ornament_number(to_string(totalOrnamentCnt));
		uint32_t totalAchivementPoint = 0;
		uint32_t collectAchivementPoint = 0;
		uint32_t cultivateAchivementPoint = 0;
		for (auto achive : allData.player_data().achivement_info().achivements()) {
			if(achive.state() == 1) {
				auto achiveCfg = gCSVAchivement.GetItem(achive.id());
				if (!achiveCfg) {
					continue;
				}
				switch (achiveCfg->Category) {
				case 1:
					collectAchivementPoint += achiveCfg->Point;
					break;
				case 2:
					cultivateAchivementPoint += achiveCfg->Point;
					break;
				default:
					totalAchivementPoint += achiveCfg->Point;
				}
			}
		}
		snapInfo->mutable_resource_info()->set_achieve_number(to_string(totalAchivementPoint));
		snapInfo->mutable_resource_info()->set_collect_achieve_number(to_string(collectAchivementPoint));
		snapInfo->mutable_resource_info()->set_cultivate_achieve_number(to_string(cultivateAchivementPoint));

		for (auto it : freindLvCnt) {
			auto snapFriend = snapInfo->add_friend_level();
			snapFriend->set_level(to_string(it.first));
			snapFriend->set_dist_cnt(to_string(it.second));
		}

		string wishKey(kKeyPrefixWishData_V1 + to_string(playerID));
		unordered_map<string, string> wishFields;
		string& wishCntStr = wishFields[wishData.kFieldWishCnt];
		if (!gRedis->hget(wishKey, wishFields)) {
			WARN_LOG("hgetall failed: %s! plid=%u", gRedis->last_error_cstr(), playerID);
			continue;
		}
		// cs::WishDataWithRank wishInfo;
		// for (auto it : fields) {
		// 	if (it.first != wishData.kFieldWishCnt && it.first != wishData.kFieldPlayedWishGrid 
		// 		&& it.first != wishData.kFieldLastWishTime && it.first != wishData.kFieldRecordFlag) {
		// 		auto wish = wishInfo.mutable_wish_data()->add_wishes();
		// 		wish->ParseFromString(it.second);
		// 	}
		// }
		int wishCnt = atoi(wishCntStr.c_str());
		for (int i=0; i<6; i++) {
			auto snapWish = snapInfo->add_vow_info();
			snapWish->set_id(to_string(i));
			snapWish->set_type(i < wishCnt ? "1" : "0");
		}

		unordered_map<pair<int, int>, int, int_pair_hash> chipNumber;
		for (auto chip : allData.player_data().chip_bag().chips()) {
			pair<int, int> chipKey = make_pair(chip.chip_id(), chip.chip_lv());
			chipNumber[chipKey] += 1;
		}
		for (int j = 0; j < allData.player_data().chip_bag().boxs_size(); ++j) {
			for (int k = 0; k < allData.player_data().chip_bag().boxs(j).chips_size(); ++k) {
				auto chip = allData.player_data().chip_bag().boxs(j).chips(k);
				pair<int, int> chipKey = make_pair(chip.chip_id(), chip.chip_lv());
				chipNumber[chipKey] += 1;
			}
		}
		for (auto chipInfo : chipNumber) {
			auto snapChip = snapInfo->add_rune_info();
			snapChip->set_id(to_string(chipInfo.first.first));
			snapChip->set_level(to_string(chipInfo.first.second));
			snapChip->set_number(to_string(chipInfo.second));
		}

		map<int, int> eggCnts;
		for (auto monEgg : allData.player_data().monster_bag().monster_eggs()) {
			eggCnts[monEgg.id()] += 1;
		}
		for (auto it : eggCnts) {
			auto snapEgg = snapInfo->add_digital_egg_info();
			snapEgg->set_id(to_string(it.first));
			snapEgg->set_number(to_string(it.second));
		}

		map<int, pair<int, int>> buildingInfo; // id,<仓库中的,摆出来的>
		for (auto building : allData.player_data().home_buildings().buildings()) {
			auto& it = buildingInfo[building.id()];
			it.first += 1;
		}
		for (auto building : allData.player_data().home_buildings().in_buildings()) {
			auto& it = buildingInfo[building.id()];
			it.second += 1;
		}
		for (auto it : buildingInfo) {
			auto snapBuilding = snapInfo->add_building_info();
			snapBuilding->set_id(to_string(it.first));
			auto itemCfg = gCSVHomeItem.GetItem(it.first);
			if (itemCfg) {
				snapBuilding->set_type(to_string(itemCfg->HomeItemType));
			} else {
				snapBuilding->set_type("0");
			}
			snapBuilding->set_warehouse_number(to_string(it.second.first));
			snapBuilding->set_home_number(to_string(it.second.second));
		}
		
		for (auto item : allData.player_data().home_items().items()) {
			auto snapOrnament = snapInfo->add_ornament_info();
			snapOrnament->set_id(to_string(item.id()));
			snapOrnament->set_warehouse_number(to_string(item.in_home_cnt()));
			snapOrnament->set_home_number(to_string(item.total_cnt() - item.in_home_cnt()));
		}

		int storyline_star = 0;
		if (allData.player_data().online_stat_info().last_online_time() > time(0) - 86400) {
			for (auto dungeon : allData.player_data().new_adventure_info().tower_datas()) {
				for (auto info : dungeon.dungeons_star()) {
					storyline_star += info.attr_val();
					auto snapStory = snapInfo->add_story_info();
					snapStory->set_section(to_string(dungeon.tower_id()));
					snapStory->set_id(to_string(info.attr_id()));
					snapStory->set_star(to_string(info.attr_val()));
					auto itemCfg = gCSVPveAdventureDungeon.GetItem(info.attr_id());
					if (itemCfg) {
						snapStory->set_type(to_string(itemCfg->Difficulty));
					} else {
						snapStory->set_type("0");
					}
				}
			}
		}

		// 各种资源
		auto snapResDiamond = snapInfo->add_resource();
		snapResDiamond->set_type(to_string(cs::ResourceTypeDiamond));
		snapResDiamond->set_number(to_string(allData.player_data().asset().cur_asset().diamonds()));
		auto snapResCoin = snapInfo->add_resource();
		snapResCoin->set_type(to_string(cs::ResourceTypeCoin));
		snapResCoin->set_number(to_string(allData.player_data().asset().cur_asset().coins()));
		auto snapResLowLottery = snapInfo->add_resource();
		snapResLowLottery->set_type(to_string(cs::ResourceTypeLowLotteryTicket));
		snapResLowLottery->set_number(to_string(allData.player_data().asset().cur_asset().low_lottery_tickets()));
		auto snapResLottery = snapInfo->add_resource();
		snapResLottery->set_type(to_string(cs::ResourceTypeLotteryTicket));
		snapResLottery->set_number(to_string(allData.player_data().asset().cur_asset().lottery_tickets()));
		auto snapResGem = snapInfo->add_resource();
		snapResGem->set_type(to_string(cs::ResourceTypeGem));
		snapResGem->set_number(to_string(allData.player_data().asset().cur_asset().gems()));
		auto snapResWishToken = snapInfo->add_resource();
		snapResWishToken->set_type(to_string(cs::ResourceTypeWishToken));
		snapResWishToken->set_number(to_string(allData.player_data().asset().cur_asset().wish_token()));
		auto snapResBroadcast = snapInfo->add_resource();
		snapResBroadcast->set_type(to_string(cs::ResourceTypeBroadcastTicket));
		snapResBroadcast->set_number(to_string(allData.player_data().asset().cur_asset().broadcast_tickets()));

		snapInfo->mutable_ext()->set_storyline_star(to_string(storyline_star));

		// cd状态下礼包
		string goodsStr;
		for (auto activeGood : allData.player_data().shop_info().active_good_infos()) {
			if (activeGood.has_last_purchase_time() && activeGood.last_purchase_time() > 0) {
				if (goodsStr.empty()) {
					goodsStr = to_string(activeGood.good_id());
				} else{
					goodsStr += ",";
					goodsStr += to_string(activeGood.good_id());
				}
			}
		}
		snapInfo->add_gift_bag()->set_gifts(goodsStr.c_str());

		// 选召者训练营最大层数
		std::map<int, int> maxCampFloor;
		for(int passId : allData.player_data().chosen_camp_info().passed_dungeons()){
			int type = passId / 100;
			int floor = passId % 100;
			if(maxCampFloor.find(type) == maxCampFloor.end() || floor > maxCampFloor[type] ){
				maxCampFloor[type] = floor;
			}
		}	
		for(auto it : maxCampFloor)	{
			auto pCampInfo = snapInfo->add_train_camp();
			pCampInfo->set_type(it.first);
			pCampInfo->set_layer(it.second);
		}
	}

	return ErrCodeSucc;
}

ErrCodeType PlayerData::getSnapShotBeg(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::SnapShotBegReq, req);
	REAL_PROTOBUF_MSG(outMsg, db::SnapShotBegRsp, rsp);

	long long total_num = 0;
	string key = makeAllYesterdayLoginedPlayersKey(req.game_region());
	if (!gRedis->scard(key, total_num)) {
		DEBUG_LOG("getSnapShotBeg, scard failed:%s", gRedis->last_error_cstr());
		return ErrCodeDB;
	}

	vector<string> v;
	if (!gRedis->smembers(key, v)) {
		DEBUG_LOG("getSnapShotBeg, smembers failed:%s", gRedis->last_error_cstr());
		return ErrCodeDB;
	}

	unordered_map<string, string> m;
	for (uint32_t i = 0; i < v.size(); i++) {
		m[std::to_string(i + 1)] = v[i];
	}

	if (!m.empty()) {
		if (!gRedis->hset(kKeyPrefixAllTodayLoginedPlayersSnapshot, m)) {
			DEBUG_LOG("getSnapShotBeg, hset failed:%s", gRedis->last_error_cstr());
			return ErrCodeDB;
		}
	}

	rsp.set_total_num(total_num);
	DEBUG_LOG("getSnapShotBeg, game_region:%d, total_num:%lld, key:%s", req.game_region(), total_num, key.c_str());
	return ErrCodeSucc;
}

ErrCodeType PlayerData::getMaxPlayerID(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(outMsg, db::Uint32Rsp, rsp);
	string uID;
	gRedis->get(kKeyPLayerGUID, uID);
	uint32_t maxID = atoi(uID.c_str())  + kPlayerIDOffset;
	rsp.set_u32(maxID);
	return ErrCodeSucc;
}

static const string kScriptSetStrangerVisit =
		"if redis.call('ZRANK', KEYS[1], ARGV[1]) then\n"
		"  return 0\n"
		"end\n"
		"redis.call('ZADD', KEYS[1], ARGV[2], ARGV[1])\n"
		"return 1";

ErrCodeType PlayerData::onSetInteractInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	if (SOCIAL_USE_REDIS) {
		// redis版本
		REAL_PROTOBUF_MSG(inMsg, db::SetInteractInfo, req);
		string strangerKey = makeStrangerKey(h.TargetID);
		string strTargetPlayerID = to_string(h.TargetID);
		string strPlayerID = to_string(h.PlayerID);
		vector<string> keys = { strangerKey };
		vector<string> args = { strPlayerID, to_string(time(nullptr)) };
		ScopedReplyPointer reply = gRedis->eval(kScriptSetStrangerVisit, &keys, &args);
		CHECK_REPLY_EC(reply, REDIS_REPLY_INTEGER, h.PlayerID);

		if (!gRedis->zadd(makeInteractTimeKey(req.type(), h.TargetID), time(nullptr), strPlayerID)) {
			WARN_LOG("zadd failed: %s! plid=%u", gRedis->last_error_cstr(), h.PlayerID);
			return ErrCodeDB;
		}
		unordered_map<string, string> mInteract;
		mInteract[strPlayerID] = to_string(req.interact_type());
		if (!gRedis->hset(makeInteractInfoKey(req.type(), h.TargetID), mInteract)) {
			WARN_LOG("hset failed: %s! plid=%u", gRedis->last_error_cstr(), h.PlayerID);
			return ErrCodeDB;
		}
		REAL_PROTOBUF_MSG(outMsg, is::SyncInteractCntReq, rsp);
		string socialKey(kKeyPrefixSocialData + to_string(h.TargetID));
		unordered_map<string, string> mSocial;
		string& strCheckInteractTime = mSocial[kSocialFieldCheckInteractTime];

		if (!gRedis->hget(socialKey, mSocial)) {
			WARN_LOG("hget failed: %s! plid=%u", gRedis->last_error_cstr(), h.PlayerID);
			return ErrCodeDB;
		}
		uint32_t strangerCnt = 0;
		if (!gRedis->zcount(strangerKey, atoi(strCheckInteractTime.c_str()), time(nullptr), strangerCnt)) {
			WARN_LOG("zcount failed: %s! plid=%u", gRedis->last_error_cstr(), h.PlayerID);
			return ErrCodeDB;
		}
		long long lookedMeCnt = 0;
		if (!gRedis->zcard(makeInteractTimeKey(cs::InteractCateResume, h.TargetID), &lookedMeCnt)) {
			WARN_LOG("zcard failed: %s! plid=%u", gRedis->last_error_cstr(), h.PlayerID);
			return ErrCodeDB;
		}
		long long visitedMeCnt = 0;
		if (!gRedis->zcard(makeInteractTimeKey(cs::InteractCateHome, h.TargetID), &visitedMeCnt)) {
			WARN_LOG("zcard failed: %s! plid=%u", gRedis->last_error_cstr(), h.PlayerID);
			return ErrCodeDB;
		}
		rsp.set_stranger_cnt(strangerCnt);
		rsp.set_looked_me_cnt(lookedMeCnt);
		rsp.set_visited_me_cnt(visitedMeCnt);
		return ErrCodeSucc;
	}
	else {
		// mysql版本
		REAL_PROTOBUF_MSG(inMsg, db::SetInteractInfo, req);
		REAL_PROTOBUF_MSG(outMsg, is::SyncInteractCntReq, rsp);

		TblSocialStranger mysqlStranger(h.TargetID, "digimon", "social_stranger");
		TblSocialInteract mysqlInteract(h.TargetID, "digimon", "social_interact");

		mysqlStranger.AddStranger(h.PlayerID);
		mysqlInteract.AddInteract(h.PlayerID, req.type(), req.interact_type());
		rsp.set_stranger_cnt(mysqlStranger.GetStrangerCnt());
		rsp.set_looked_me_cnt(mysqlInteract.GetCntByInteractType(cs::InteractCateResume));
		rsp.set_visited_me_cnt(mysqlInteract.GetCntByInteractType(cs::InteractCateHome));
		unordered_map<string, string> m;
		m[kSocialFieldStrangerCnt] = to_string(rsp.stranger_cnt());
		m[kSocialFieldLookedCnt] = to_string(rsp.looked_me_cnt());
		m[kSocialFieldVisitedCnt] = to_string(rsp.visited_me_cnt());
		if (!gRedis->hset(kKeyPrefixSocialData + to_string(h.TargetID), m)) {
			WARN_LOG("hset failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
			return ErrCodeDB;
		}

		setPlayerDataTouchInfo(h.TargetID, kKeyPrefixSocialData, true);
		return ErrCodeSucc;
	}
	return ErrCodeSucc;
}

ErrCodeType PlayerData::onViewPlayerInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(outMsg, cs::PlayerResume, rsp);
	ErrCodeType errCode = doGetPlayerAndMonBasicInfo(h.PlayerID, h.TargetID, rsp.mutable_basic_info()->mutable_player_info(), rsp.mutable_basic_info()->mutable_monster_info());
	if (errCode) {
		return errCode;
	}
	return getPlayerSocialInfo(h, nullptr, rsp.mutable_social_info());
}

ErrCodeType PlayerData::onViewInteractInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, cs::InteractInfoReq, req);
	REAL_PROTOBUF_MSG(outMsg, cs::InteractInfoRsp, rsp);

	if (SOCIAL_USE_REDIS) {
		vector<string> result;
		string interTimeKey = makeInteractTimeKey(req.type(), h.PlayerID);
		if (!gRedis->zrevrange(interTimeKey, req.from_pos(), req.end_pos(), result, 1)) {
			WARN_LOG("zrevrange failed: %s! plid=%u", gRedis->last_error_cstr(), h.PlayerID);
			return ErrCodeDB;
		}
		unordered_map<string, string> mInteractInfo;
		for (size_t i = 0; i < result.size(); i += 2) {
			mInteractInfo[result[i]];
		}
		if (result.size() > 0) {
			string interKey = makeInteractInfoKey(req.type(), h.PlayerID);
			if (!gRedis->hget(interKey, mInteractInfo)) {
				WARN_LOG("hget failed: %s! plid=%u", gRedis->last_error_cstr(), h.PlayerID);
				return ErrCodeDB;
			}
		}

		for (size_t i = 0; i < result.size(); i += 2) {
			auto data = rsp.add_infos();
			data->set_player_id(atoi(result[i].c_str()));
			auto it = mInteractInfo.find(result[i]);
			if (it != mInteractInfo.end()) {
				data->set_interact_type(atoi(it->second.c_str()));
			}
			else {
				data->set_interact_type(0);
			}
			data->set_time(atoi(result[i + 1].c_str()));
		}
		rsp.set_next_pos(req.from_pos() + result.size() / 2);
		if (!req.from_pos()) {
			unordered_map<string, string> mSocial;
			mSocial[kSocialFieldCheckInteractTime] = to_string(time(nullptr));

			string socialKey(kKeyPrefixSocialData + to_string(h.PlayerID));
			if (!gRedis->hset(socialKey, mSocial)) {
				WARN_LOG("hset failed: %s! plid=%u", gRedis->last_error_cstr(), h.PlayerID);
				return ErrCodeDB;
			}
		}
	}
	else {
		TblSocialInteract mysqlInteract(h.PlayerID, "digimon", "social_interact");
		mysqlInteract.GetInteractList(req.type(), req.from_pos(), req.end_pos(),
			[&rsp](uint32_t interact_pid, uint32_t sub_type, uint32_t time_stamp) {
			auto data = rsp.add_infos();
			data->set_player_id(interact_pid);
			data->set_interact_type(sub_type);
			data->set_time(time_stamp);
		});
		rsp.set_next_pos(req.from_pos() + rsp.infos_size());
		if (!req.from_pos()) {
			TblSocialStranger mysqlStranger(h.PlayerID, "digimon", "social_stranger");
			mysqlStranger.UpdateCheckTime();
		}
	}
	
	return ErrCodeSucc;
}

ErrCodeType PlayerData::getPlayerInfoForStat(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::StatReq, req);

	string strPlayerID = to_string(h.TargetID);
	string key(kKeyPrefixPlayerData + strPlayerID);
	unordered_map<string, string> m;
	string& accountInfoStr = m[kAccountInfo];
	string& regInfoStr = m[kRegInfo];
	string& onlineStatInfoStr = m[kOnlineStatInfo];
	string& attrInfoStr = m[kPlayerAttr];
	if (!gRedis->hget(key, m)) {
		WARN_LOG("hgetall failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}
	if (accountInfoStr.empty()) {
		return ErrCodeSucc;
	}
	long long finishedOrderCnt = 0;
	if (!gRedis->hlen(makeFinishedOrderKey(h.TargetID), finishedOrderCnt)) {
		WARN_LOG("hlen failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}
	bool existExtra = false;
	string extraKey(kKeyPrefixExtrPhotos + to_string(h.TargetID));
	string value;
	if (!gRedis->get(extraKey, value, &existExtra)) {
		WARN_LOG("exists failed: %s!", gRedis->last_error_cstr());
		return ErrCodeDB;
	}
	/*string decorateKey = homeDecorateData.makeDecorateKey(h.TargetID);
	unordered_map<string, string> mDecorate;
	string& homeLvStr = mDecorate[kDecorateLevelFiled];
	if (!gRedis->hget(decorateKey, mDecorate)) {
		WARN_LOG("hgetall failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}*/
	int homeLv = homeDecorateData.getHomeLevel(h.TargetID);
	cs::ArenaInfo arenaInfo;
	gArena.GetInfoImp(h.TargetID, arenaInfo);

	REAL_PROTOBUF_MSG(outMsg, db::PlayerDataForStat, rsp);
	rsp.mutable_req()->Swap(&req);

	if (existExtra) {
		rsp.mutable_extra_info()->ParseFromString(value);
	}
	rsp.mutable_account_info()->ParseFromString(accountInfoStr);
	rsp.mutable_reg_info()->ParseFromString(regInfoStr);
	rsp.mutable_online_stat_info()->ParseFromString(onlineStatInfoStr);
	cs::PlayerAttr attr;
	attr.ParseFromString(attrInfoStr);
	rsp.set_arena_region(arenaInfo.region());
	rsp.set_home_lv(homeLv);
	rsp.set_user_lv(attr.lv());
	if (finishedOrderCnt) {
		rsp.set_is_paid("1");
	} else {
		rsp.set_is_paid("0");
	}
	return ErrCodeSucc;
}

ErrCodeType PlayerData::createPlayer(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	string key(kKeyPrefixPlayerData + to_string(h.TargetID));

	unordered_map<string, string> fields;
	const auto& accInfoStr = fields[kAccountInfo];
	const auto& regInfoStr = fields[kRegInfo];
	if (!gRedis->hget(key, fields)) {
		WARN_LOG("hget() failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}
	if (accInfoStr.size()) {
		DEBUG_LOG("Player had already been created! plid=%u", h.TargetID);
		return ErrCodeAlreadyCreatedPlayer;
	}

	string strPlayerID = to_string(h.TargetID);

	REAL_PROTOBUF_MSG(inMsg, db::PlayerData, req);
	SetPlayerLoginedToday(strPlayerID, false, req.game_region().u32());

	db::RegInfo regInfo;
	regInfo.ParseFromString(regInfoStr);
	TblPlayerRegion tblPlayerRegion(0);
	tblPlayerRegion.PlayerCreate(regInfo.account().account_type(), regInfo.account().account_id(), h.TargetID, req.game_region().u32());

	// 新建角色
	return doSetPlayerData(h, key, req, true);
}

ErrCodeType PlayerData::setPlayerData(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::PlayerData, req);

	string key(kKeyPrefixPlayerData + to_string(h.TargetID));
	return doSetPlayerData(h, key, req);
}

ErrCodeType PlayerData::getPlayerData(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	//DEBUG_LOG("getPlayerData start. plid=%d", h.TargetID);
	REAL_PROTOBUF_MSG(inMsg, db::Uint32Req, req);

	int gameRegion = req.u32();
	string strPlayerID = to_string(h.TargetID);
	string key(kKeyPrefixPlayerData + strPlayerID);
	unordered_map<string, string> m;
	if (!gRedis->hgetall(key, m)) {
		WARN_LOG("hgetall failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}

	REAL_PROTOBUF_MSG(outMsg, db::PlayerData, rsp);
	if (m.empty() || !toPlayerData(m, rsp)) {
		DEBUG_LOG("getPlayerData not found pid. plid=%d", h.TargetID);
		return ErrCodeEntryNotFound;
	}

	// 判断渠道是否被封
	if (rsp.reg_info().channel() != "") {
		unordered_map<string, string> m;
		auto& banChannelCreateStr = m[rsp.reg_info().channel()];
		if (!gRedis->hget(kMiscFieldBanChannelCreate, m)) {
			WARN_LOG("hget failed: %s! plid=%u", gRedis->last_error_message().c_str(), h.TargetID);
			return ErrCodeDB;
		}
		auto banChannelCreateTime = atoi(banChannelCreateStr.c_str());
		rsp.mutable_reg_info()->set_can_create(time(0) < banChannelCreateTime ? false : true);
	}



	// 拉取昨日竞技场段位
	rsp.mutable_arena_info()->set_region_yesterday(gArena.GetYesterdayRegionId(h.TargetID));

	SetPlayerLoginedToday(strPlayerID, (rsp.newbie_info().complete_time() > 0), gameRegion);

	TblPlayerRegion tblPlayerRegion(0);
	tblPlayerRegion.PlayerLogin(rsp.reg_info().account().account_type(), rsp.reg_info().account().account_id(), h.TargetID, gameRegion);

	if (rsp.account_info().gender() == kMaleFlag) {
		key = kGameMGuide;
	} else {
		key = kGameFGuide;
	}
	gRedis->sadd(key, { strPlayerID });

	DEBUG_LOG("getPlayerData. plid=%d rg=%d", h.TargetID, gameRegion);

	online(h.TargetID);
	setPlayerDataTouchInfo(h.TargetID, kKeyPrefixPlayerData, false);

	// 存储玩家登陆记录
	updatePlayerLoginRecord(h.TargetID, TransIP(rsp.online_stat_info().last_device().ip()));

	return ErrCodeSucc;
}

ErrCodeType PlayerData::completeNewbieGuide(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::Uint32Req, req);

	string strPlayerID = to_string(h.TargetID);
	// 过了新手引导，把玩家加到当日登录列表
	string todayPlayersKey = makeTodayLoginedPlayersKey(req.u32());
	gRedis->sadd(todayPlayersKey, { strPlayerID });
	// 设置当日登录列表的过期时间
	long long int ttl = -1;
	gRedis->ttl(todayPlayersKey, ttl);
	if (ttl == -1) {
		gRedis->expire(todayPlayersKey, 86700); // 多留300秒
	}
	return ErrCodeSucc;
}

ErrCodeType	PlayerData::getRecruitInviteCode(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::Uint32Req, req);
	REAL_PROTOBUF_MSG(outMsg, cs::SCStringRsp, rsp);

	//char code[10] = {'\0'};
	//code[0] = '0' + req.u32();
	string code;
	

	uint32_t tryTimes = 0;
	while (tryTimes < 100) {
		code.clear();
		code.append(to_string(req.u32() % 10 + ((tryTimes / 20) * 2)));
		for (int i = 0; i < 8; i++) {
			code.append(to_string(rand() % 10));
		}
		bool exist;
		if (!gRedis->hexists(kKeyRecruitInviteCode, code, exist)) {
			WARN_LOG("hexists failed! plid=%u err=%s",
				h.TargetID, gRedis->last_error_cstr());
			return ErrCodeDB;
		}
		if (!exist) {
			// 设置全局的code
			unordered_map<string, string> m;
			m[code] = to_string(h.PlayerID);
			if (!gRedis->hset(kKeyRecruitInviteCode, m)) {
				WARN_LOG("hset failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
				return ErrCodeDB;
			}
			// 设置个人code
			string rkey = makeRecruitDataKey(h.TargetID);
			unordered_map<string, string> pm;
			pm[kRecruitFieldCode] = code; 
			if (!gRedis->hset(rkey, pm)) {
				WARN_LOG("hset failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
				return ErrCodeDB;
			}
			break;
		}
		tryTimes++;
	}

	rsp.set_str(code);
	return ErrCodeSucc;
}

ErrCodeType	PlayerData::finishBeRecruited(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::StrReq, req);
	REAL_PROTOBUF_MSG(outMsg, db::PlayerRecruitInfo, rsp);
	
	// get招募者plid
	string code = req.str();
	unordered_map<string, string> m;
	auto& plidStr = m[code];
	if (!gRedis->hget(kKeyRecruitInviteCode, m)) {
		WARN_LOG("hget failed: %s!", gRedis->last_error_message().c_str());
		return ErrCodeDB;
	}
	if (plidStr.empty()) {
		return ErrCodeRecruitCodeNotExists;
	}
	auto plid = atoi(plidStr.c_str());

	// 获取招募者信息
	string pKey(kKeyPrefixPlayerData + plidStr);
	unordered_map<string, string> pm;
	auto& accInfoStr = pm[kAccountInfo];
	if (!gRedis->hget(pKey, m)) {
		WARN_LOG("hget failed: %s!", gRedis->last_error_message().c_str());
		return ErrCodeDB;
	}
	db::AccountInfo accInfo;
	accInfo.ParseFromString(accInfoStr);

	// 招募成功,设置我自己的信息
	string myReKey = makeRecruitDataKey(h.TargetID);
	unordered_map<string, string> myinfo;
	myinfo[kRecruitFieldMyRecruitPlid] = plidStr;
	myinfo[kRecruitFieldMyRecruitNick] = accInfo.player_name();
	if (!gRedis->hset(myReKey, myinfo)) {
		WARN_LOG("hset failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}

	// 设置招募者招募信息
	string rKey = makeRecruitDataKey(plid);
	unordered_map<string, string> pinfo;
	string pField = to_string(h.TargetID);
	pinfo[pField] = to_string(time(0));
	if (!gRedis->hset(rKey, pinfo)) {
		WARN_LOG("hset failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}

	rsp.set_my_recruit(plid);
	rsp.set_my_recruit_nick(accInfo.player_name());

	return ErrCodeSucc;
}

ErrCodeType	PlayerData::addRecruiteScore(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::AddRecruiteScoreInfo, req);
	REAL_PROTOBUF_MSG(outMsg, db::AddRecruiteScoreInfo, rsp);

	string tKey = makeRecruitDataKey(req.plid());

	unordered_map<string, string> tinfo;
	auto& lgScoreStr = tinfo[kRecruitFieldLoginScore];
	auto& adScoreStr = tinfo[kRecruitFieldAdventureScore];
	auto& btlScoreStr = tinfo[kRecruitFieldBtlGrpScore];
	auto& lvScoreStr = tinfo[kRecruitFieldLevelScore];
	if (!gRedis->hget(tKey, tinfo)) {
		WARN_LOG("hset failed: %s! plid=%u", gRedis->last_error_cstr(), req.plid());
		return ErrCodeDB;
	}

	long long lgScore = atoi(lgScoreStr.c_str());
	long long adScore = atoi(adScoreStr.c_str());
	long long btlScore = atoi(btlScoreStr.c_str());
	long long lvScore = atoi(lvScoreStr.c_str());

	string field;
	long long curScore = 0;
	if (req.type() == 1) {
		field = kRecruitFieldLoginScore;
		curScore = lgScore;
	} else if (req.type() == 2) {
		field = kRecruitFieldAdventureScore;
		curScore = adScore;
	} else if (req.type() == 3) {
		field = kRecruitFieldBtlGrpScore;
		curScore = btlScore;
	} else if (req.type() == 4) {
		field = kRecruitFieldLevelScore;
		curScore = lvScore;
	} else {
		WARN_LOG("addRecruiteScore type error! plid=%u type=%d", req.plid(), req.type());
		return ErrCodeDB;
	}

	long long totalScore = req.limit_value();
	if (curScore < req.limit_value()) {
		if (!gRedis->hincrby(tKey, field, req.add_value(), &totalScore)) {
			WARN_LOG("hincrby failed: %s! plid=%u", gRedis->last_error_cstr(), req.plid());
			return ErrCodeDB;
		}
	} else {
		rsp.set_limit_flag(true);
		return ErrCodeSucc;
	}

	if (totalScore > req.limit_value()) {
		totalScore = req.limit_value();
		unordered_map<string, string> sinfo;
		sinfo[field] = to_string(totalScore);
		gRedis->hset(tKey, sinfo);
	}

	rsp.set_plid(req.plid());
	rsp.set_type(req.type());
	rsp.set_add_value(req.add_value());
	rsp.set_limit_value(req.limit_value());
	rsp.set_total_value(totalScore);
	rsp.set_limit_flag(false);

	return ErrCodeSucc;
}

ErrCodeType PlayerData::getHomeInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	string key(kKeyPrefixPlayerData + to_string(h.TargetID));
	unordered_map<string, string> m;
	auto& accInfoStr = m[kAccountInfo];
	auto& monBagStr = m[kMonsterBag];
	auto& garmentInfoStr = m[kGarWearing];
	if (!gRedis->hget(key, m)) {
		WARN_LOG("hget failed: %s!", gRedis->last_error_message().c_str());
		return ErrCodeDB;
	}

	// 判断该玩家是否存在
	bool playerFound = false;
	for (const auto& v : m) {
		if (v.second.size()) {
			playerFound = true;
			break;
		}
	}
	if (!playerFound) {
		DEBUG_LOG("No such player! plid=%u target=%u proto=%u origProto=%u",
					h.PlayerID, h.TargetID, h.ProtoID, h.OrigProtoID);
		return ErrCodeEntryNotFound;
	}

	db::PlayerData playerData;
	auto& accInfo = *playerData.mutable_account_info();
	accInfo.ParseFromString(accInfoStr);
	auto& monBag = *playerData.mutable_monster_bag();
	monBag.ParseFromString(monBagStr);

	cs::GarmentWearing garmentWearing;
	garmentWearing.ParseFromString(garmentInfoStr);

	REAL_PROTOBUF_MSG(outMsg, db::GetHomeInfoRsp, rsp);
	rsp.mutable_player_name()->swap(*accInfo.mutable_player_name());
	rsp.mutable_picture()->swap(*accInfo.mutable_picture());
	rsp.mutable_birthday()->swap(*accInfo.mutable_birthday());
	rsp.set_avatar_frame(accInfo.avatar_frame());
	rsp.set_title(accInfo.title());
	rsp.set_avatar(accInfo.avatar());
	rsp.set_mon_id(monBag.primary_id());

	if (monBag.in_home_mons_size() > 0) {
		unordered_map<string, string> inHomeMons;
		for (int i = 0; i < monBag.in_home_mons_size(); ++i) {
			inHomeMons[kMonHashKeyPrefix + to_string(monBag.in_home_mons(i))];
		}
		if (gRedis->hget(key, inHomeMons)) {
			db::MonsterInfo monInfo;
			for (const auto& v : inHomeMons) {
				if (monInfo.ParseFromString(v.second)) {
					rsp.add_owner_mon_ids(monInfo.id());
				}
			}
		}
	}

	setPlayerDataTouchInfo(h.TargetID, kKeyPrefixPlayerData, false);
	return ErrCodeSucc;
}

ErrCodeType PlayerData::getPlayerBasicInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(outMsg, cs::SCGetPlayerBasicInfo, rsp);
	return doGetPlayerAndMonBasicInfo(h.PlayerID, h.TargetID, rsp.mutable_player_info(), rsp.mutable_monster_info());
}

ErrCodeType PlayerData::getPlayerAndMonBasicInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, cs::CSGetPlayersBasicInfoReq, req);
	REAL_PROTOBUF_MSG(outMsg, cs::SCGetPlayersBasicInfo, rsp);

	cs::PlayerAndMonBasicInfo* info = nullptr;
	for (int i = 0; i < req.u32_size(); ++i) {
		if (!info) {
			info = rsp.add_info();
		}
		if (doGetPlayerAndMonBasicInfo(h.PlayerID, req.u32(i), info->mutable_player_info(), info->mutable_mon_info(), req.last_region()) == ErrCodeSucc) {
			info = nullptr;
		}
	}
	if (info) {
		rsp.mutable_info()->RemoveLast();
	}

	return ErrCodeSucc;
}

ErrCodeType PlayerData::getFriendsBasicInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::RepeatedUint32Req, req);
	REAL_PROTOBUF_MSG(outMsg, db::GetFriendsBasicInfoRsp, rsp);

	// DEBUG_LOG("Get friends basic info. plid=%u cnt=%d", h.PlayerID, req.u32_size());
	for (int i = 0; i < req.u32_size(); ++i) {
		doGetFriendBasicInfo(h.PlayerID, req.u32(i), rsp);
	}

	return ErrCodeSucc;
}

ErrCodeType PlayerData::getPlayerExtraData(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::Uint32Req, req);
	REAL_PROTOBUF_MSG(outMsg, db::PlayerExtraData, rsp);
	DEBUG_LOG("Get extra data. plid=%u", h.PlayerID);

	// 特别关注
	vector<string> favFollowings;
	if (!gRedis->smembers(makeFavFollowingKey(h.TargetID), favFollowings)) {
		WARN_LOG("smembers failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}
	// 关注
	unordered_map<string, string> followings;
	if (!gRedis->hgetall(makeFollowingKey(h.TargetID), followings)) {
		WARN_LOG("hgetall failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}
	// 粉丝
	// unordered_map<string, string> followers;
	// if (!gRedis->hgetall(makeFollowerKey(h.TargetID), followers)) {
	// 	WARN_LOG("hgetall failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
	// 	return ErrCodeDB;
	// }
	// Momo关注
	unordered_map<string, string> momoFollowings;
	if (!gRedis->hgetall(kKeyPrefixMomoFollowing + to_string(h.TargetID), momoFollowings)) {
		WARN_LOG("hgetall failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}
	// Momo粉丝
	unordered_map<string, string> momoFollowers;
	if (!gRedis->hgetall(kKeyPrefixMomoFollower + to_string(h.TargetID), momoFollowers)) {
		WARN_LOG("hgetall failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}

	// 黑名单
	// vector<string> blacklist;
	unordered_map<string, string> blacklist;
	// if (!gRedis->smembers(makeBlacklistKey(h.TargetID), blacklist)) {
	if (!gRedis->hgetall(makeBlacklistKey(h.TargetID), blacklist)) {
		WARN_LOG("smembers failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}
	// 互相关注
	vector<string> intersection;
	if (!gRedis->smembers(makeIntersectionKey(h.TargetID), intersection)) {
		WARN_LOG("smembers failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}

	auto friends = rsp.mutable_friends();
	for (const auto& v : favFollowings) {
		friends->add_fav_followings(atoi(v.c_str()));
	}
	toFriendsData(followings, momoFollowings, friends->mutable_followings());
	// toFriendsData(followers, momoFollowers, friends->mutable_followers());
	for (const auto& v : blacklist) {
		auto blackinfo = friends->add_blacklist();
		blackinfo->set_player_id(atoi(v.first.c_str()));
		blackinfo->set_create_time(atoi(v.second.c_str()));
		// friends->add_blacklist(atoi(v.c_str()));
	}
	for (const auto& v : intersection) {
		friends->add_intersection(atoi(v.c_str()));
	}
	long long followerCnt = 0;
	if (!gRedisRank->zcard(makeFansKey(h.TargetID), &followerCnt)) {
		WARN_LOG("zcard failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}
	friends->set_follower_cnt(followerCnt);

	// 粉丝
	vector<string> result;
	if (!gRedisRank->zrange(makeFansKey(h.TargetID), 0, 119, result)) {
		WARN_LOG("zrange failed: %s! plid=%u", gRedisRank->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}
	for (auto plid : result) {
		auto follower = friends->add_followers();
		follower->set_player_id(atoi(plid.c_str()));
		auto momoFollowingIt = momoFollowings.find(plid);
		if (momoFollowingIt != momoFollowings.end()) {
			// 对方是momo用户的话要设momoid
			string regKey(kKeyPrefixPlayerData + plid);
			unordered_map<string, string> mReg;
			auto& regInfoStr = mReg[kRegInfo];
			if (!gRedis->hget(regKey, mReg)) {
				WARN_LOG("hget failed: %s! plid=%s", gRedis->last_error_message().c_str(), plid.c_str());
				continue;
			}
			db::RegInfo regInfo;
			regInfo.ParseFromString(regInfoStr);
			if (regInfo.account().account_type() == kChannelNameMomo) {
				follower->set_momo_id(regInfo.account().account_id());
			}
		}
	}

	// 邮件
	ErrCodeType type = gMailData.PackMail(h, req.u32(), rsp.mutable_mails());

	if(type != ErrCodeSucc) {
		return type;
	}

	// 活动
	auto acs = rsp.mutable_activitys();
	unordered_map<string, string> acsMap;
	if (!gRedis->hgetall(kKeyPrefixActivityData + to_string(h.TargetID), acsMap)) {
		WARN_LOG("hgetall failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}

	for (const auto& v : acsMap) {
		if(v.first != kKeyActivtiyDailyDeadTMData) {
			acs->add_activitys()->ParseFromString(v.second);
		} else {
			acs->set_daily_deadtime(atoi(v.second.c_str()));
		}
	}

	auto acinfo = rsp.mutable_ac_infos();
	unordered_map<string, string> acinfoMap;
	if (!gRedis->hgetall(kKeyPrefixPlayerAcInfo + to_string(h.TargetID), acinfoMap)) {
		WARN_LOG("hgetall failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}

	for (const auto& v : acinfoMap) {
		acinfo->add_infos()->ParseFromString(v.second);
	}

	//拉去玩家额外照片
	bool exist = false;
	string key1(kKeyPrefixExtrPhotos + to_string(h.TargetID));
	string value;
	if (!gRedis->get(key1, value, &exist)) {
		WARN_LOG("exists failed: %s!", gRedis->last_error_cstr());
		return ErrCodeDB;
	}
	//DEBUG_LOG(" kKeyPrefixExtrPhotos%s", value.c_str());

	if (exist) {
		rsp.mutable_extra_info()->ParseFromString(value);
	}


	//战队信息
	exist = false;
	key1 = makePlayersKey(to_string(h.TargetID));

	if (!gRedis->get(key1, value, &exist)) {
		WARN_LOG("exists failed: %s!", gRedis->last_error_cstr());
		return ErrCodeDB;
	}

	if (exist) {
		// uint32_t val = strtol(value.c_str(), nullptr, 10);
		uint32_t val = atoi(value.c_str());
		rsp.mutable_corps_info()->set_corps_id(val);
		//rsp.mutable_corps_info()->mutable_corps_id()->set_corps_hid(val>>32);
		//rsp.mutable_corps_info()->mutable_corps_id()->set_corps_lid(val&0xffffffff);
	}

	/*
	key1 = makePlayersJoinKey(to_string(h.TargetID));
	unordered_map<string, string> jmap;
	if (!gRedis->hgetall(key1, jmap)) {
		WARN_LOG("hgetall failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}

	for (const auto& v : jmap) {
		cinfo->add_jcoprs_id(strtol(v.second.c_str(), nullptr, 10));
	}
	*/

	key1 = makePlayersInvKey(to_string(h.TargetID));
	unordered_map<string, string> imap;
	if (!gRedis->hgetall(key1, imap)) {
		WARN_LOG("hgetall failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}

	auto* cinfo = rsp.mutable_corps_info();
	for (const auto& v : imap) {
		auto* invites = cinfo->add_invites();
		invites->ParseFromString(v.second);
	}


	key1 = makePlayerCorpsTask(to_string(h.TargetID));
	unordered_map<string, string> tmap;
	if (!gRedis->hgetall(key1, tmap)) {
		WARN_LOG("hgetall failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}

	for (const auto& v : tmap) {
		auto* task = cinfo->add_infos();
		task->ParseFromString(v.second);
	}



	// 红点提醒
	/*auto redpt = rsp.mutable_redpoint_info();
	vector<string> redptlist;
	if (!gRedis->smembers(makeRedptKey(h.TargetID), redptlist)) {
		WARN_LOG("smembers failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}
	for (const auto& v : redptlist) {
		redpt->add_types(atoi(v.c_str()));
	}*/

	getPlayerMiscInfo(h, nullptr, rsp.mutable_misc_info());

	// 膜拜信息
	// unordered_map<string, string> worship;
	// if (!gRedis->hgetall(makeWorshipKey(h.TargetID), worship)) {
	// 	WARN_LOG("hgetall failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
	// 	return ErrCodeDB;
	// }
	// for (auto v : worship) {
	// 	auto data = rsp.mutable_worship_info()->add_worship_data();
	// 	data->set_day(atoi(v.first.c_str()));
	// 	data->set_cnt(atoi(v.second.c_str()));
	// }

	// 拉取礼物信息
	getPlayerGiftInfo(h, nullptr, rsp.mutable_gift_items());

	// 拉取历练成就
	std::string key = makePlayerExAcTask(to_string(h.TargetID));
	std::unordered_map<std::string, std::string> task;
	if (!gRedis->hgetall(key, task)) {
		WARN_LOG("hgetall failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}

	std::string fkey = makePlayerFExAcTask(to_string(h.TargetID));
	std::unordered_map<std::string, std::string> ftask;
	if (!gRedis->hgetall(fkey, ftask)) {
		WARN_LOG("hgetall failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}

	for (const auto& v : task) {
		auto* temp = rsp.mutable_exactask_info()->add_infos();
		temp->ParseFromString(v.second);
	}

	for (const auto& v : ftask) {
		rsp.mutable_exactask_info()->add_fin_task_ids(atoi(v.first.c_str()));
	}

	// 时装数据
	std::string garmentKey = makeGarmentKey(to_string(h.TargetID));
	unordered_map<string, string> mapGarment;
	if (!gRedis->hgetall(garmentKey, mapGarment)) {
		WARN_LOG("hgetall failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}
	for (const auto& it : mapGarment) {
		if (it.first == kKeyGar_Uid) {
			rsp.mutable_gar_data()->set_uid(atoi(it.second.c_str()));
		}
		else if (it.first == kKeyGar_SuitUse) {
			rsp.mutable_gar_data()->mutable_data()->set_suit_use(atoi(it.second.c_str()));
		}
		else if (it.first.find(kKeyGar_Garments) != string::npos) {
			auto tmp = rsp.mutable_gar_data()->mutable_data()->add_garments();
			tmp->ParseFromString(it.second);
		}
		else if (it.first.find(kKeyGar_GarSuits) != string::npos) {
			auto tmp = rsp.mutable_gar_data()->mutable_data()->add_suits();
			tmp->ParseFromString(it.second);
		}
		else if (it.first.find(kKeyGar_GarPics) != string::npos) {
			auto tmp = rsp.mutable_gar_data()->mutable_data()->add_pics();
			tmp->ParseFromString(it.second);
		}
	}

	//CapRes
	std::string resKey = makeCapResKey(to_string(h.TargetID));
	std::unordered_map<std::string, std::string> capRes;
	if (!gRedis->hgetall(resKey, capRes)) {
		WARN_LOG("hgetall failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}

	for (const auto& v : capRes) {
		if (v.first == kKeyCapResBasicInfo) {
			rsp.mutable_res_info()->add_binfo()->ParseFromString(v.second);
			continue;
		} else if (v.first == kKeyCapResLastStoreInfo) {
			rsp.mutable_res_info()->mutable_last()->ParseFromString(v.second);
		} else {
			auto* temp = rsp.mutable_res_info()->add_res();
			temp->ParseFromString(v.second);
		}
	}
	//DEBUG_LOG("=============== %s", rsp.mutable_res_info()->Utf8DebugString().c_str());
	
	// 公测预约数据
	std::string betaKey = makePlayerBetaKey(to_string(h.TargetID));
	std::string val;
	if (!gRedis->get(betaKey, val)) {
		WARN_LOG("get failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}
	rsp.mutable_bata_assist()->ParseFromString(val);
	if (!rsp.mutable_bata_assist()->phone().empty()) { // 这里特殊处理下，momo账号 ios、android同服的情况
		unordered_map<string, string> tmp;
		const string& val = tmp[rsp.mutable_bata_assist()->phone()];
		if (!gRedis->hget("DigimonBetaSubscribeTmp", tmp)) {
			WARN_LOG("checkBetaSubscribe hget call failed, phone=%s, plid=%u", rsp.mutable_bata_assist()->phone().c_str(), h.TargetID);
			return ErrCodeDB;
		}
		if (!val.empty() && !rsp.mutable_bata_assist()->stage_two_state()) {
			rsp.mutable_bata_assist()->set_stage_two_state(1);
		}
	}
	//DEBUG_LOG("[BetaAssist] %s", rsp.mutable_bata_assist()->Utf8DebugString().c_str());

	// 拉取玩家小屋数据
	string hutKey = makePlayerHutKey(to_string(h.TargetID));
	string hutVal;
	if (!gRedis->get(hutKey, hutVal)) {
		WARN_LOG("get HutInfo failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}
	rsp.mutable_hut()->ParseFromString(hutVal);

	// 拉取招募信息
	string reKey = makeRecruitDataKey(h.TargetID);
	std::unordered_map<std::string, std::string> reInfo;
	if (!gRedis->hgetall(reKey, reInfo)) {
		WARN_LOG("hgetall failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}

	// 道馆防守数据
	std::string pavilionExtraStr;
	if (!gRedis->get(makePavilionExtraDataKey(h.TargetID), pavilionExtraStr)) {
		WARN_LOG("get pavilionExtraData failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}
	rsp.mutable_pavilion_extra_data()->ParseFromString(pavilionExtraStr);

	for (auto& kv : reInfo) {
		if (kv.first == kRecruitFieldCode) {
			rsp.mutable_recruit_info()->set_invite_code(kv.second);
		} else if (kv.first == kRecruitFieldMyRecruitPlid) {
			rsp.mutable_recruit_info()->set_my_recruit(atoi(kv.second.c_str()));
		} else if (kv.first == kRecruitFieldMyRecruitNick) {
			rsp.mutable_recruit_info()->set_my_recruit_nick(kv.second);
		} else if (kv.first == kRecruitFieldLoginScore) {
			rsp.mutable_recruit_info()->set_login_score(atoi(kv.second.c_str()));
		} else if (kv.first == kRecruitFieldAdventureScore) {
			rsp.mutable_recruit_info()->set_group_score(atoi(kv.second.c_str()));
		} else if (kv.first == kRecruitFieldBtlGrpScore) {
			rsp.mutable_recruit_info()->set_btlgrp_score(atoi(kv.second.c_str()));
		} else if (kv.first == kRecruitFieldLevelScore) {
			rsp.mutable_recruit_info()->set_lv_score(atoi(kv.second.c_str()));
		} else {
			auto recruitePlid = atoi(kv.first.c_str());
			if (recruitePlid > 0) {
				rsp.mutable_recruit_info()->add_recruits(recruitePlid);
			}
		}
	}


	return type;
}

ErrCodeType PlayerData::getPlayerSocialInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	string strPlayerID = to_string(h.TargetID);
	string key(kKeyPrefixPlayerData + strPlayerID);
	unordered_map<string, string> m;
	auto& activeInfo = m[kActiveSocialInfo];
	auto& lazyInfo = m[kLazySocialInfo];
	if (!gRedis->hget(key, m)) {
		WARN_LOG("hgetall failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}

	db::AllPlayerData dbRsp;
	if (m.empty()) {
		return ErrCodeEntryNotFound;
	}

	dbRsp.mutable_player_data()->mutable_active_social_info()->ParseFromString(activeInfo);
	dbRsp.mutable_player_data()->mutable_lazy_social_info()->ParseFromString(lazyInfo);

	getPlayerMiscInfo(h, nullptr, dbRsp.mutable_extra_data()->mutable_misc_info());

	wishData.GetWishData(h, nullptr, dbRsp.mutable_wish_data());
	if (dbRsp.player_data().lazy_social_info().show_formation_size()) {
		unordered_map<string, string> mons;
		for (auto monGUID : dbRsp.player_data().lazy_social_info().show_formation()) {
			mons[kMonHashKeyPrefix + to_string(monGUID)];
		}
		if (!gRedis->hget(key, mons)) {
			WARN_LOG("hgetall failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
			return ErrCodeDB;
		}
		for (auto v : mons) {
			db::MonsterInfo monInfo;
			monInfo.ParseFromString(v.second);
			auto mon = dbRsp.mutable_extra_data()->mutable_show_formation()->add_mons();
			mon->set_id(monInfo.id());
			mon->set_lv(monInfo.lv());
			mon->set_star(monInfo.star());
			mon->set_max_stage(monInfo.max_stage());
		}
	}

	REAL_PROTOBUF_MSG(outMsg, cs::PlayerSocialProfile, rsp);
	rsp.mutable_social_attr()->set_total_popular(dbRsp.extra_data().misc_info().popular() + dbRsp.extra_data().misc_info().momo_popular());
	rsp.mutable_social_attr()->set_total_wealth(dbRsp.extra_data().misc_info().wealth());
	rsp.mutable_social_attr()->set_week_popular(dbRsp.extra_data().misc_info().week_popular() + dbRsp.extra_data().misc_info().week_momo_popular());
	rsp.mutable_social_attr()->set_week_wealth(dbRsp.extra_data().misc_info().week_wealth());
	rsp.mutable_social_attr()->set_popular_lv(dbRsp.extra_data().misc_info().popular_lv());
	rsp.mutable_social_attr()->set_wealth_lv(dbRsp.extra_data().misc_info().wealth_lv());
	rsp.mutable_social_attr()->mutable_pic_addr()->CopyFrom(dbRsp.player_data().lazy_social_info().pic_addr());
	rsp.mutable_social_attr()->mutable_show_formation()->CopyFrom(dbRsp.extra_data().show_formation());
	rsp.mutable_social_attr()->mutable_geo_info()->CopyFrom(dbRsp.player_data().lazy_social_info().geo_info());
	rsp.mutable_social_attr()->set_liked_cnt(dbRsp.extra_data().misc_info().liked_cnt());
	rsp.mutable_social_attr()->set_following_cnt(dbRsp.extra_data().misc_info().following_cnt());
	rsp.mutable_social_attr()->set_follower_cnt(dbRsp.extra_data().misc_info().follower_cnt());
	rsp.mutable_wish_data()->Swap(dbRsp.mutable_wish_data()->mutable_wish_data());

	setPlayerDataTouchInfo(h.TargetID, kKeyPrefixPlayerData, false);
	return ErrCodeSucc;
}

ErrCodeType PlayerData::getPlayerMiscInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(outMsg, db::PlayerMiscInfo, rsp);

	string strPlayerID = to_string(h.TargetID);
	// 聊天封禁时间
	unordered_map<string, string> ban;
	auto& banLogin = ban[kBEDataFieldBanLogin];
	auto& banChat = ban[kBEDataFieldBanChat];
	if (!gRedis->hgetall(makeBasicExtraDataKey(strPlayerID), ban)) {
		WARN_LOG("hgetall failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}
	unordered_map<string, string> m;
	auto& regInfoStr = m[kRegInfo];
	if (!gRedis->hget(kKeyPrefixPlayerData + strPlayerID, m)) {
		WARN_LOG("hget failed: %s! plid=%u", gRedis->last_error_message().c_str(), h.TargetID);
		return ErrCodeDB;
	}
	setPlayerDataTouchInfo(h.TargetID, kKeyPrefixPlayerData, false);

	db::RegInfo regInfo;
	regInfo.ParseFromString(regInfoStr);
	m.clear();
	auto& banDeviceChatTimeStr = m[regInfo.device().did()];
	if (!gRedis->hget(kMiscFieldBanChatDevice, m)) {
		WARN_LOG("hget failed: %s! plid=%u", gRedis->last_error_message().c_str(), h.TargetID);
		return ErrCodeDB;
	}
	auto banDeviceChatTime = atoi(banDeviceChatTimeStr.c_str());
	m.clear();
	auto& banIPChatTimeStr = m[TransIP(regInfo.device().ip())];
	if (!gRedis->hget(kMiscFieldBanChatIP, m)) {
		WARN_LOG("hget failed: %s! plid=%u", gRedis->last_error_message().c_str(), h.TargetID);
		return ErrCodeDB;
	}
	auto banIPChatTime = atoi(banIPChatTimeStr.c_str());
	auto banChatTime = atoi(banChat.c_str());
	if (banDeviceChatTime > banChatTime) {
		banChatTime = banDeviceChatTime;
	}
	if (banIPChatTime > banChatTime) {
		banChatTime = banIPChatTime;
	}
	rsp.set_ban_chat_time(banChatTime);
	rsp.set_ban_login_time(atoi(banLogin.c_str()));

	ErrCodeType errCode = checkWeeklySocial(h.TargetID);
	if (errCode) {
		return errCode;
	}
	errCode = checkDailySocial(h.TargetID);
	if (errCode) {
		return errCode;
	}
	unordered_map<string, string> social;
	auto& popular = social[kSocialFieldPopular];
	auto& momoPopular = social[kSocialFieldMomoPopular];
	auto& love = social[kSocialFieldLove];
	auto& momoLove = social[kSocialFieldMomoLove];
	auto& wealth = social[kSocialFieldWealth];
	auto& likedCnt = social[kSocialFieldLikedCnt];
	auto& weekPopular = social[kSocialFieldWeekPopular];
	auto& weekMomoPopular = social[kSocialFieldWeekMomoPopular];
	auto& weekLove = social[kSocialFieldWeekLove];
	auto& weekMomoLove = social[kSocialFieldWeekMomoLove];
	auto& weekWealth = social[kSocialFieldWeekWealth];
	auto& dailyPopular = social[kSocialFieldDailyPopular];
	auto& dailyWealth = social[kSocialFieldDailyWealth];
	auto& popularLv = social[kSocialFieldPopularLv];
	auto& WealthLv = social[kSocialFieldWealthLv];
	auto& gsMem = social[kSocialFieldGsMember];
	auto& strStrangerCnt = social[kSocialFieldStrangerCnt];
	auto& strLookedCnt = social[kSocialFieldLookedCnt];
	auto& strVisitedCnt = social[kSocialFieldVisitedCnt];
	auto& opVer = social[kMiscFieldOperationVer];

	if (!gRedis->hget(kKeyPrefixSocialData + strPlayerID, social)) {
		WARN_LOG("hgetall failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}
	setPlayerDataTouchInfo(h.TargetID, kKeyPrefixSocialData, false);

	rsp.set_popular(atoi(popular.c_str()));
	rsp.set_momo_popular(atoi(momoPopular.c_str()));
	rsp.set_love(atoi(love.c_str()));
	rsp.set_momo_love(atoi(momoLove.c_str()));
	rsp.set_wealth(atoi(wealth.c_str()));
	rsp.set_week_popular(atoi(weekPopular.c_str()));
	rsp.set_week_momo_popular(atoi(weekMomoPopular.c_str()));
	rsp.set_week_love(atoi(weekLove.c_str()));
	rsp.set_week_momo_love(atoi(weekMomoLove.c_str()));
	rsp.set_week_wealth(atoi(weekWealth.c_str()));
	rsp.set_liked_cnt(atoi(likedCnt.c_str()));
	rsp.set_daily_popular(atoi(dailyPopular.c_str()));
	rsp.set_daily_wealth(atoi(dailyWealth.c_str()));
	rsp.set_popular_lv(atoi(popularLv.c_str()));
	rsp.set_wealth_lv(atoi(WealthLv.c_str()));
	rsp.set_operation_ver(atoi(opVer.c_str()));

	db::GsMemberInfo gsm;
	gsm.ParseFromString(gsMem);
	rsp.set_gsmem_time_beg(gsm.time_beg());
	rsp.set_gsmem_time_end(gsm.time_end());
	rsp.set_is_rec(gsm.is_rec());

	// 每日人气累积（计算加成）
	// (这个功能突然说不要了，何必当初呢。。。2018.09.06 记上)
	/*int yPoplular = 0;
	checkDailyPopular(h.TargetID, &yPoplular);
	rsp.set_yesterday_popular(yPoplular);*/

	// 该状态控制充值返利活动是否显示 [ 0不显示 ( 此次测试之前从未充值过 或者 奖励已领取 ) 1显示 ]
	string account = makeLoginKey(regInfo.account().account_type(), regInfo.account().account_id());
	m.clear();
	m[account];
	if (!gRedis->hget(kRebateBoughtGems, m)) {
		DEBUG_LOG("hget bought gems failed! err=%s plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}
	if (atoi(m[account].c_str())) {
		m.clear();
		m[account];
		if (!gRedis->hget(kKeyOrderGotBoughtGems, m)) {
			DEBUG_LOG("hget has got rebate gems failed! err=%s plid=%u", gRedis->last_error_cstr(), h.TargetID);
			return ErrCodeDB;
		}
		if (atoi(m[account].c_str()) == 0) {
			rsp.set_rebate_state(1);
		}
	}

	// 关注数和粉丝数
	string followingKey = makeFollowingKey(h.TargetID);
	string followerKey = makeFollowerKey(h.TargetID);
	long long followingCnt, followerCnt;
	if (!gRedis->hlen(followingKey, followingCnt)) {
		DEBUG_LOG("hlen following failed! err=%s plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}
	// if (!gRedis->hlen(followerKey, followerCnt)) {
	// 	DEBUG_LOG("hlen follower failed! err=%s plid=%u", gRedis->last_error_cstr(), h.TargetID);
	// 	return ErrCodeDB;
	// }
	if (!gRedisRank->zcard(makeFansKey(h.TargetID), &followerCnt)) {
		WARN_LOG("zcard failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}
	rsp.set_following_cnt(followingCnt);
	rsp.set_follower_cnt(followerCnt);

	// TODO:
	if (SOCIAL_USE_REDIS) {
		// redis版本
		string socialKey(kKeyPrefixSocialData + to_string(h.TargetID));
		unordered_map<string, string> mSocial;
		string& strCheckInteractTime = mSocial[kSocialFieldCheckInteractTime];

		if (!gRedis->hget(socialKey, mSocial)) {
			WARN_LOG("hget failed: %s! plid=%u", gRedis->last_error_cstr(), h.PlayerID);
			return ErrCodeDB;
		}
		uint32_t strangerCnt = 0;
		if (!gRedis->zcount(makeStrangerKey(h.TargetID), atoi(strCheckInteractTime.c_str()), time(nullptr), strangerCnt)) {
			WARN_LOG("zcount failed: %s! plid=%u", gRedis->last_error_cstr(), h.PlayerID);
			return ErrCodeDB;
		}
		long long lookedMeCnt = 0;
		if (!gRedis->zcard(makeInteractTimeKey(cs::InteractCateResume, h.TargetID), &lookedMeCnt)) {
			WARN_LOG("zcard failed: %s! plid=%u", gRedis->last_error_cstr(), h.PlayerID);
			return ErrCodeDB;
		}
		long long visitedMeCnt = 0;
		if (!gRedis->zcard(makeInteractTimeKey(cs::InteractCateHome, h.TargetID), &visitedMeCnt)) {
			WARN_LOG("zcard failed: %s! plid=%u", gRedis->last_error_cstr(), h.PlayerID);
			return ErrCodeDB;
		}
		rsp.set_stranger_cnt(strangerCnt);
		rsp.set_looked_me_cnt(lookedMeCnt);
		rsp.set_visited_me_cnt(visitedMeCnt);
	}
	else {
		// mysql版本
		rsp.set_stranger_cnt(atoi(strStrangerCnt.c_str()));
		rsp.set_looked_me_cnt(atoi(strLookedCnt.c_str()));
		rsp.set_visited_me_cnt(atoi(strVisitedCnt.c_str()));
	}
	
	return ErrCodeSucc;
}

ErrCodeType PlayerData::addHomeItemPopular(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::Uint32Req, req);

	string key(kKeyPrefixSocialData + to_string(h.TargetID));
	unordered_map<string, string> social;
	auto& yesterdayPopular = social[kSocialFieldYesterdayPopular];
	if (!gRedis->hgetall(key, social)) {
		WARN_LOG("hgetall failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}

	uint32_t yPopluar = atoi(yesterdayPopular.c_str());
	social[kSocialFieldYesterdayPopular] = to_string(0);
	if (!gRedis->hset(key, social)) {
		WARN_LOG("hset failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}

	uint32_t addValue = yPopluar * req.u32() / 100;
	db::AddPopularReq addReq;
	addReq.set_popular(addValue);
	addReq.set_src(cs::PopularSrc::PopularSrcHomeItem);
	addReq.set_adddailyflag(false);
	return addPopular(h, &addReq, nullptr);
}

ErrCodeType PlayerData::getGMPlayerData(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	string strPlayerID = to_string(h.TargetID);
	string key(kKeyPrefixPlayerData + strPlayerID);
	unordered_map<string, string> m;
	if (!gRedis->hgetall(key, m)) {
		WARN_LOG("hgetall failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}

	REAL_PROTOBUF_MSG(outMsg, db::AllPlayerData, rsp);
	if (m.empty() || !toPlayerData(m, *rsp.mutable_player_data())) {
		return ErrCodeEntryNotFound;
	}

	db::Uint32Req req;
	req.set_u32(h.TargetID);
	getPlayerExtraData(h, &req, rsp.mutable_extra_data());
	// 设置公会名称
	uint32_t corps_id_ = rsp.extra_data().corps_info().corps_id();
	if (corps_id_) {
		bool exists = false;
		string basic;
		if (!gRedis->get(makeCorpsBasicKey(to_string(corps_id_)), basic, &exists)) {
			WARN_LOG("hgetall failed: %s!corpsid=%u", gRedis->last_error_cstr(), corps_id_);
			return ErrCodeDB;
		}
		if (exists) {
			cs::CorpsBasicInfo tmp;
			tmp.ParseFromString(basic);
			rsp.mutable_extra_data()->mutable_corps_info()->set_cname(tmp.corps_name());
		}
	}

	gPayData.GetPayOrder(h, nullptr, rsp.mutable_pay_order());

	cs::ArenaInfo arena_;
	gArena.GetInfoImp(h.TargetID, arena_, false, false);
	rsp.set_arena_region(arena_.region());

	std::string loginkey = kKeyPrefixPlayerAcInfo + to_string(h.TargetID);
	m.clear();
	auto& strTime = m[to_string(cs::kLastLoginTimestamp)];
	auto& strDays = m[to_string(cs::kContLoginDays)];
	if (!gRedis->hget(loginkey, m)) {
		WARN_LOG("hget failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}
	cs::PlayerAcInfo time_;
	cs::PlayerAcInfo days_;
	time_.ParseFromString(strTime);
	days_.ParseFromString(strDays);
	rsp.set_last_login_time(time_.val());
	rsp.set_cont_login_days(days_.val());

	string wealthkey(kKeyPrefixSocialData + to_string(h.TargetID));
	m.clear();
	auto& wealthval = m[kSocialFieldWealth];
	if (!gRedis->hget(wealthkey, m)) {
		WARN_LOG("hget failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}
	rsp.set_wealth(atoi(wealthval.c_str()));

	return ErrCodeSucc;
}

ErrCodeType PlayerData::setMomoPopular(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	// 检查是否完成建号,防止没建号完的上榜
	string accountKey(kKeyPrefixPlayerData + to_string(h.TargetID));
	unordered_map<string, string> mAccount;
	auto& accInfoStr = mAccount[kAccountInfo];
	if (!gRedis->hget(accountKey, mAccount)) {
		WARN_LOG("hget failed: %s! plid=%u", gRedis->last_error_message().c_str(), h.TargetID);
		return ErrCodeDB;
	}

	if (accInfoStr.empty()) {
		DEBUG_LOG("No such player! plid=%u target=%u", h.PlayerID, h.TargetID);
		return ErrCodeDB;
	}

	db::AccountInfo accInfo;
	accInfo.ParseFromString(accInfoStr);
	if (!accInfo.has_player_name()) {
		return ErrCodeSucc;
	}

	REAL_PROTOBUF_MSG(inMsg, db::Uint32Req, req);
	// Todo,改req为uint32_pair,内含总人气和周人气
	uint32_t momoPopular = req.u32();
	uint32_t weekMomoPopular = 0;
	string strPlayerID = to_string(h.TargetID);
	string key(kKeyPrefixSocialData + strPlayerID);
	double timeTail = GetTimeTail();
	ErrCodeType errCode = checkWeeklySocial(h.TargetID);
	if (errCode) {
		return errCode;
	}
	errCode = checkDailySocial(h.TargetID);
	if (errCode) {
		return errCode;
	}
	unordered_map<string, string> m;
	m[kSocialFieldMomoPopular] = to_string(momoPopular);
	m[kSocialFieldWeekMomoPopular] = to_string(weekMomoPopular);
	// stringstream ss;
	// ss << kKeyPrefixSocialData << to_string(NextMonday()) << '_' << strPlayerID;
	// string weekKey = ss.str();
	// 设陌陌总人气
	if (!gRedis->hset(key, m)) {
		WARN_LOG("hset failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}
	setPlayerDataTouchInfo(h.TargetID, kKeyPrefixSocialData, true);
	// 设陌陌周人气
	// m[kSocialFieldMomoPopular] = to_string(weekMomoPopular);
	// if (!gRedis->hset(weekKey, m)) {
	// 	WARN_LOG("hset failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
	// 	return ErrCodeDB;
	// }
	// 取游戏总人气
	m.clear();
	string& gamePopular = m[kSocialFieldPopular];
	string& weekGamePopular = m[kSocialFieldWeekPopular];
	string& dailyGamePopular = m[kSocialFieldDailyPopular];
	if (!gRedis->hget(key, m)) {
		WARN_LOG("hget failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}
	double totalScore = (double)momoPopular + atoi(gamePopular.c_str()) + timeTail; // 总人气值
	// 取游戏周人气
	// m.clear();
	// string& weekGamePopular = m[kSocialFieldPopular];
	// if (!gRedis->hget(weekKey, m)) {
	// 	WARN_LOG("hget failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
	// 	return ErrCodeDB;
	// }
	double weekScore = (double)weekMomoPopular + atoi(weekGamePopular.c_str()) + timeTail; // 周人气值
	double dailyScore = atoi(dailyGamePopular.c_str()) + timeTail; // 周人气值
	// 取玩家地理位置
	cs::GeoPos geo;
	errCode = GeoData::GetGeoData(h.TargetID, geo);
	if (errCode) {
		return errCode;
	}
	int gameRegion = GetGameRegion(h.TargetID);
	geo.set_game_region(gameRegion);
	// 设置排行榜

	if (!IsGsMember(h.TargetID)) {
		RankData::DoSetGeoRank(strPlayerID, totalScore, weekScore, dailyScore, geo, kSocialFieldPopular);
	}

	return ErrCodeSucc;
}

ErrCodeType PlayerData::setMomoLove(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	// 检查是否完成建号,防止没建号完的上榜
	string accountKey(kKeyPrefixPlayerData + to_string(h.TargetID));
	unordered_map<string, string> mAccount;
	auto& accInfoStr = mAccount[kAccountInfo];
	if (!gRedis->hget(accountKey, mAccount)) {
		WARN_LOG("hget failed: %s! plid=%u", gRedis->last_error_message().c_str(), h.TargetID);
		return ErrCodeDB;
	}

	if (accInfoStr.empty()) {
		DEBUG_LOG("No such player! plid=%u target=%u", h.PlayerID, h.TargetID);
		return ErrCodeDB;
	}

	db::AccountInfo accInfo;
	accInfo.ParseFromString(accInfoStr);
	if (!accInfo.has_player_name()) {
		return ErrCodeSucc;
	}

	REAL_PROTOBUF_MSG(inMsg, db::Uint32Req, req);
	// Todo,改req为uint32_pair,内含总爱心和周爱心
	uint32_t momoLove = req.u32();
	uint32_t weekMomoLove = 0;
	string strPlayerID = to_string(h.TargetID);
	string key(kKeyPrefixSocialData + strPlayerID);
	double timeTail = GetTimeTail();
	ErrCodeType errCode = checkWeeklySocial(h.TargetID);
	if (errCode) {
		return errCode;
	}
	errCode = checkDailySocial(h.TargetID);
	if (errCode) {
		return errCode;
	}
	unordered_map<string, string> m;
	m[kSocialFieldMomoLove] = to_string(req.u32());
	m[kSocialFieldWeekMomoLove] = to_string(weekMomoLove);
	// stringstream ss;
	// ss << kKeyPrefixSocialData << to_string(NextMonday()) << '_' << strPlayerID;
	// string weekKey = ss.str();
	// 设陌陌总爱心
	if (!gRedis->hset(key, m)) {
		WARN_LOG("hset failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}
	setPlayerDataTouchInfo(h.TargetID, kKeyPrefixSocialData, true);
	// 设陌陌周爱心
	// m[kSocialFieldMomoLove] = to_string(weekMomoLove);
	// if (!gRedis->hset(weekKey, m)) {
	// 	WARN_LOG("hset failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
	// 	return ErrCodeDB;
	// }
	// 取游戏总爱心
	m.clear();
	string& gameLove = m[kSocialFieldLove];
	string& weekGameLove = m[kSocialFieldWeekLove];
	if (!gRedis->hget(key, m)) {
		WARN_LOG("hget failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}
	double totalScore = (double)momoLove + atoi(gameLove.c_str()) + timeTail; // 总爱心值
	// 取游戏周爱心
	// m.clear();
	// string& weekGameLove = m[kSocialFieldLove];
	// if (!gRedis->hget(weekKey, m)) {
	// 	WARN_LOG("hget failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
	// 	return ErrCodeDB;
	// }
	double weekScore = (double)weekMomoLove + atoi(weekGameLove.c_str()) + timeTail; // 周爱心值
	// 取玩家地理位置
	cs::GeoPos geo;
	errCode = GeoData::GetGeoData(h.TargetID, geo);
	if (errCode) {
		return errCode;
	}
	int gameRegion = GetGameRegion(h.TargetID);
	geo.set_game_region(gameRegion);
	// 设置排行榜
	RankData::DoSetGeoRank(strPlayerID, totalScore, weekScore, 0, geo, kSocialFieldLove);

	return ErrCodeSucc;
}

ErrCodeType PlayerData::setMomoFriendShip(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, center::MomoFriend, req);
	string strPlayerID = to_string(h.TargetID);
	string targetIDStr = to_string(req.player_id());
	unordered_map<string, string> m_friend;
	unordered_map<string, string> m_follower;
	string friendKey, followerKey;
	// 我注的
	// if (req.follow_status() == 1) {
	friendKey = kKeyPrefixMomoFollowing + strPlayerID;
	followerKey = kKeyPrefixMomoFollower + targetIDStr;
	m_friend[targetIDStr] = to_string(req.friendship());
	m_follower[strPlayerID] = to_string(req.friendship());
	// } else { // follow_status == 2, 关注我的
	// 	friendKey = kKeyPrefixMomoFollowing + targetIDStr;
	// 	followerKey = kKeyPrefixMomoFollower + strPlayerID;
	// 	m_friend[strPlayerID] = to_string(req.friendship());
	// 	m_follower[targetIDStr] = to_string(req.friendship());
	// }
	if (!gRedis->hset(friendKey, m_friend)) {
		WARN_LOG("hset failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}
	if (!gRedis->hset(followerKey, m_follower)) {
		WARN_LOG("hset failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}
	setPlayerDataTouchInfo(h.TargetID, kKeyPrefixMomoFollowing, true);
	setPlayerDataTouchInfo(req.player_id(), kKeyPrefixMomoFollower, true);

	return ErrCodeSucc;
}

ErrCodeType PlayerData::addLove(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::AddLoveReq, req);
	string strPlayerID = to_string(h.TargetID);
	// 检查是否完成建号,防止没建号完的上榜
	string accountKey(kKeyPrefixPlayerData + to_string(h.TargetID));
	unordered_map<string, string> mAccount;
	auto& accInfoStr = mAccount[kAccountInfo];
	if (!gRedis->hget(accountKey, mAccount)) {
		WARN_LOG("hget failed: %s! plid=%u", gRedis->last_error_message().c_str(), h.TargetID);
		return ErrCodeDB;
	}

	if (accInfoStr.empty()) {
		DEBUG_LOG("No such player! plid=%u target=%u", h.PlayerID, h.TargetID);
		return ErrCodeDB;
	}

	db::AccountInfo accInfo;
	accInfo.ParseFromString(accInfoStr);
	if (!accInfo.has_player_name()) {
		return ErrCodeSucc;
	}

	string key(kKeyPrefixSocialData + strPlayerID);
	long long totalLove, weekLove;
	double timeTail = GetTimeTail();
	ErrCodeType errCode = checkWeeklySocial(h.TargetID);
	if (errCode) {
		return errCode;
	}
	// stringstream ss;
	// ss << kKeyPrefixSocialData << to_string(NextMonday()) << '_' << strPlayerID;
	// string weekKey = ss.str();
	// 增加游戏总爱recordkey	
	if (!gRedis->hincrby(key, kSocialFieldLove, req.val(), &totalLove)) {
		WARN_LOG("hincrby failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}
	// 增加游戏周爱心值
	// if (!gRedis->hincrby(weekKey, kSocialFieldLove, req.u32(), &weekLove)) {
	// 	WARN_LOG("hincrby failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
	// 	return ErrCodeDB;
	// }
	if (!gRedis->hincrby(key, kSocialFieldWeekLove, req.val(), &weekLove)) {
		WARN_LOG("hincrby failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}
	// 取陌陌周爱心值
	unordered_map<string, string> m;
	// string& weekMomoLove = m[kSocialFieldMomoLove];
	// if (!gRedis->hget(weekKey, m)) {
	// 	WARN_LOG("hget failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
	// 	return ErrCodeDB;
	// }
	string& weekMomoLove = m[kSocialFieldWeekMomoLove];
	if (!gRedis->hget(key, m)) {
		WARN_LOG("hget failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}
	double weekScore = (double)weekLove + atoi(weekMomoLove.c_str()) + timeTail; // 周爱心值
	// 取陌陌总爱心值
	m.clear();
	string& momoLove = m[kSocialFieldMomoLove];
	if (!gRedis->hget(key, m)) {
		WARN_LOG("hget failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}
	double totalScore = (double)totalLove + atoi(momoLove.c_str()) + timeTail; // 总爱心值
	// 取玩家地理位置
	cs::GeoPos geo;
	errCode = GeoData::GetGeoData(h.TargetID, geo);
	if (errCode) {
		return errCode;
	}
	geo.set_game_region(req.game_region());
	// 设置排行榜
	RankData::DoSetGeoRank(strPlayerID, totalScore, weekScore, 0, geo, kSocialFieldLove);

	REAL_PROTOBUF_MSG(outMsg, db::ForMomoAddLovePopular, rsp);
	// 对方是momo用户的话要同步友好度
	string regKey(kKeyPrefixPlayerData + strPlayerID);
	unordered_map<string, string> mReg;
	auto& regInfoStr = mReg[kRegInfo];
	if (!gRedis->hget(regKey, mReg)) {
		WARN_LOG("hget failed: %s! plid=%u", gRedis->last_error_message().c_str(), h.TargetID);
		return ErrCodeDB;
	}
	db::RegInfo regInfo;
	regInfo.ParseFromString(regInfoStr);
	if (regInfo.account().account_type() == kChannelNameMomo) {
		rsp.set_momo_id(regInfo.account().account_id());
		rsp.set_love(totalLove);
	}

	setPlayerDataTouchInfo(h.TargetID, kKeyPrefixSocialData, true);
	return ErrCodeSucc;
}

ErrCodeType PlayerData::addPopular(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	// 检查是否完成建号,防止没建号完的上榜
	string accountKey(kKeyPrefixPlayerData + to_string(h.TargetID));
	unordered_map<string, string> mAccount;
	auto& accInfoStr = mAccount[kAccountInfo];
	if (!gRedis->hget(accountKey, mAccount)) {
		WARN_LOG("hget failed: %s! plid=%u", gRedis->last_error_message().c_str(), h.TargetID);
		return ErrCodeDB;
	}

	if (accInfoStr.empty()) {
		DEBUG_LOG("No such player! plid=%u target=%u", h.PlayerID, h.TargetID);
		return ErrCodeDB;
	}

	db::AccountInfo accInfo;
	accInfo.ParseFromString(accInfoStr);
	if (!accInfo.has_player_name()) {
		return ErrCodeSucc;
	}

	REAL_PROTOBUF_MSG(inMsg, db::AddPopularReq, req);
	string strPlayerID = to_string(h.TargetID);
	string key(kKeyPrefixSocialData + strPlayerID);
	long long totalPopular, weekPopular, dailyPopular;
	double timeTail = GetTimeTail();
	ErrCodeType errCode = checkWeeklySocial(h.TargetID);
	if (errCode) {
		return errCode;
	}
	errCode = checkDailySocial(h.TargetID);
	if (errCode) {
		return errCode;
	}
	// stringstream ss;
	// ss << kKeyPrefixSocialData << to_string(NextMonday()) << '_' << strPlayerID;
	// string weekKey = ss.str();
	
	// 增加被点赞次数
	if (req.src() == cs::PopularSrcLike) {
		if (!gRedis->hincrby(key, kSocialFieldLikedCnt, 1)) {
			WARN_LOG("hincrby failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
			return ErrCodeDB;
		}
	}

	if (req.popular() <= 0) {
		return ErrCodeSucc;
	}

	// 增加游戏总人气值
	if (!gRedis->hincrby(key, kSocialFieldPopular, req.popular(), &totalPopular)) {
		WARN_LOG("hincrby failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}
	// 增加游戏周人气值
	if (!gRedis->hincrby(key, kSocialFieldWeekPopular, req.popular(), &weekPopular)) {
		WARN_LOG("hincrby failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}
	// 增加游戏日人气值
	if (!gRedis->hincrby(key, kSocialFieldDailyPopular, req.popular(), &dailyPopular)) {
		WARN_LOG("hincrby failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}
	// 设置人气值等级
	if (req.popular_lv() > 0) {
		unordered_map<string, string> fields;
		fields[kSocialFieldPopularLv] = to_string(req.popular_lv());
		if (!gRedis->hset(key, fields)) {
			WARN_LOG("hset failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
			return ErrCodeDB;
		}
	}
	// 取陌陌周人气值
	unordered_map<string, string> m;
	string& weekMomoPopular = m[kSocialFieldWeekMomoPopular];
	if (!gRedis->hget(key, m)) {
		WARN_LOG("hget failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}
	double weekScore = (double)weekPopular + atoi(weekMomoPopular.c_str()) + timeTail; // 周人气值
	// 取陌陌总人气值
	m.clear();
	string& momoPopular = m[kSocialFieldMomoPopular];
	if (!gRedis->hget(key, m)) {
		WARN_LOG("hget failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}
	double totalScore = (double)totalPopular + atoi(momoPopular.c_str()) + timeTail; // 总人气值
	double dailyScore = (double)dailyPopular + timeTail; // 周人气值
	// 取玩家地理位置
	cs::GeoPos geo;
	errCode = GeoData::GetGeoData(h.TargetID, geo);
	if (errCode) {
		DEBUG_LOG("get geo data failed! plid=%u", h.TargetID);
		return errCode;
	}
	geo.set_game_region(req.game_region());
	// 设置排行榜
	if (!IsGsMember(h.TargetID)) {
		RankData::DoSetGeoRank(strPlayerID, totalScore, weekScore, dailyScore, geo, kSocialFieldPopular);
	}

	// 增加每日累积（建筑物增加人气值功能，和日人气值、周人气值之类无关）
	// (这个功能突然说不要了，何必当初呢。。。2018.09.06 记上)
	/*if (req.popular() > 0 && req.adddailyflag()) {
		doAddDailyPopular(h.TargetID, req.popular());
	}*/

	if (outMsg) {
		REAL_PROTOBUF_MSG(outMsg, db::ForMomoAddLovePopular, rsp);
		// 对方是momo用户的话要同步友好度
		string regKey(kKeyPrefixPlayerData + strPlayerID);
		unordered_map<string, string> mReg;
		auto& regInfoStr = mReg[kRegInfo];
		if (!gRedis->hget(regKey, mReg)) {
			WARN_LOG("hget failed: %s! plid=%u", gRedis->last_error_message().c_str(), h.TargetID);
			return ErrCodeDB;
		}
		db::RegInfo regInfo;
		regInfo.ParseFromString(regInfoStr);
		if (regInfo.account().account_type() == kChannelNameMomo) {
			rsp.set_momo_id(regInfo.account().account_id());
			rsp.set_popular(totalPopular);
		}
	}

	setPlayerDataTouchInfo(h.TargetID, kKeyPrefixSocialData, true);
	return ErrCodeSucc;
}

ErrCodeType PlayerData::addWealth(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::AddWealthReq, req);
	string strPlayerID = to_string(h.TargetID);
	// 检查是否完成建号,防止没建号完的上榜
	/*string accountKey(kKeyPrefixPlayerData + to_string(h.TargetID));
	unordered_map<string, string> mAccount;
	auto& accInfoStr = mAccount[kAccountInfo];
	if (!gRedis->hget(accountKey, mAccount)) {
		WARN_LOG("hget failed: %s! plid=%u", gRedis->last_error_message().c_str(), h.TargetID);
		return ErrCodeDB;
	}

	if (accInfoStr.empty()) {
		DEBUG_LOG("No such player! plid=%u target=%u", h.PlayerID, h.TargetID);
		return ErrCodeDB;
	}

	db::AccountInfo accInfo;
	accInfo.ParseFromString(accInfoStr);
	if (!accInfo.has_player_name()) {
		return ErrCodeSucc;
	}*/

	ErrCodeType errCode = checkWeeklySocial(h.TargetID);
	if (errCode) {
		return errCode;
	}
	errCode = checkDailySocial(h.TargetID);
	if (errCode) {
		return errCode;
	}

	string key(kKeyPrefixSocialData + strPlayerID);
	long long totalWealth, weekWealth, dailyWealth;
	// 增加游戏总财富值
	if (!gRedis->hincrby(key, kSocialFieldWealth, req.wealth(), &totalWealth)) {
		WARN_LOG("hincrby failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}
	if (!gRedis->hincrby(key, kSocialFieldWeekWealth, req.wealth(), &weekWealth)) {
		WARN_LOG("hincrby failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}
	if (!gRedis->hincrby(key, kSocialFieldDailyWealth, req.wealth(), &dailyWealth)) {
		WARN_LOG("hincrby failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}
	if (req.wealth_lv() > 0) {
		unordered_map<string, string> fields;
		fields[kSocialFieldWealthLv] = to_string(req.wealth_lv());
		if (!gRedis->hset(key, fields)) {
			WARN_LOG("hset failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
			return ErrCodeDB;
		}
	}

	double timeTail = GetTimeTail();
	double weekScore = (double)weekWealth + timeTail; // 周财富值
	double totalScore = (double)totalWealth + timeTail; // 总财富值
	double dailyScore = (double)dailyWealth + timeTail; // 总财富值
	
	// 取玩家地理位置
	cs::GeoPos geo;
	errCode = GeoData::GetGeoData(h.TargetID, geo);
	if (errCode) {
		return errCode;
	}
	geo.set_game_region(req.game_region());

	// 设置排行榜
	RankData::DoSetGeoRank(strPlayerID, totalScore, weekScore, dailyScore, geo, kSocialFieldWealth);
	
	setPlayerDataTouchInfo(h.TargetID, kKeyPrefixSocialData, true);
	return ErrCodeSucc;
}

ErrCodeType PlayerData::getWorship(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(outMsg, db::WorshipInfo, rsp);
	// 膜拜信息
	unordered_map<string, string> worship;
	if (!gRedis->hgetall(makeWorshipKey(h.TargetID), worship)) {
		WARN_LOG("hgetall failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}
	for (auto v : worship) {
		auto dayAndRegion = Split(v.first, '_');
		if (dayAndRegion.size() != 2) {
			DEBUG_LOG("invalid worship key! plid=%u", h.TargetID);
			continue;
		}
		auto data = rsp.add_worship_data();
		data->set_day(atoi(dayAndRegion[0].c_str()));
		data->set_region(atoi(dayAndRegion[1].c_str()));
		data->set_cnt(atoi(v.second.c_str()));
	}

	setPlayerDataTouchInfo(h.TargetID, kKeyPrefixWorship, false);
	return ErrCodeSucc;
}

ErrCodeType PlayerData::addWorship(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::WorshipData, req);
	REAL_PROTOBUF_MSG(outMsg, db::Uint32Rsp, rsp);
	string worshipKey = makeWorshipKey(h.TargetID);
	string strPlayerID = to_string(h.TargetID);

	unordered_map<string, string> m;
	auto& strRegion = m[strPlayerID];
	if (!gRedis->hget(kKeyArenaInfo, m)) {
		WARN_LOG("hget failed: %s! plid=%u,target_plid=%u", gRedis->last_error_cstr(), h.PlayerID, h.TargetID);
		return ErrCodeDB;
	}

	uint32_t region = 1;
	if (!strRegion.empty()) {
		region = atoi(strRegion.c_str());
	}
	stringstream ss;
	ss << req.day() << '_' << region;
	string hkey = ss.str();
	unordered_map<string, string> info;
	auto& strDayCnt = info[hkey];
	if (!gRedis->hget(worshipKey, info)) {
		WARN_LOG("hget failed: %s! plid=%u,target_plid=%u", gRedis->last_error_cstr(), h.PlayerID, h.TargetID);
		return ErrCodeDB;
	}
	rsp.set_u32(atoi(strDayCnt.c_str()) >= 100 ? 0 : h.TargetID);
	if (rsp.u32()) {
		if (!gRedis->hincrby(worshipKey, hkey, req.cnt())) {
			WARN_LOG("hincrby failed: %s! plid=%u,target_plid=%u", gRedis->last_error_cstr(), h.PlayerID, h.TargetID);
			return ErrCodeDB;
		}
	}

	setPlayerDataTouchInfo(h.TargetID, kKeyPrefixWorship, true);
	return ErrCodeSucc;
}

ErrCodeType PlayerData::delWorship(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::RepeatedStrReq, req);
	vector<string> v;
	for (auto hkey : req.strs()) {
		v.emplace_back(hkey);
	}
	string worshipKey = makeWorshipKey(h.TargetID);
	if (!gRedis->hdel(worshipKey, v)) {
		WARN_LOG("hdel failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}

	setPlayerDataTouchInfo(h.TargetID, kKeyPrefixWorship, true);
	return ErrCodeSucc;
}

ErrCodeType	PlayerData::giveGift(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::GiveGiftInfo, req);
	REAL_PROTOBUF_MSG(outMsg, db::GiveGiftInfo, rsp);

	uint32_t receiver = req.receiver_plid();

	// 查看旧排行榜
	long long oldRank = -1;
	string rankKey(kKeyPrefixGiftGivenRank + to_string(receiver));
	gRedis->zrevrank(rankKey, to_string(req.giver_plid()), &oldRank);

	// 添加gift
	string giftKey(kKeyPrefixGiftItem + to_string(receiver));
	if (!gRedis->hincrby(giftKey, to_string(req.gift().item_id()), req.gift().item_cnt())) {
		WARN_LOG("hincrby failed: %s! plid=%u", gRedis->last_error_cstr(), receiver);
		return ErrCodeDB;
	}
	
	// 增加可兑换彩钻的值
	if (req.gem_value() > 0) {
		if (!gRedis->hincrby(giftKey, kGiftItemFieldGemValue, req.gem_value())) {
			WARN_LOG("hincrby failed: %s! plid=%u", gRedis->last_error_cstr(), receiver);
			return ErrCodeDB;
		}
		if (!gRedis->hincrby(giftKey, kGiftItemFieldGemHistroyValue, req.gem_value())) {
			WARN_LOG("hincrby failed: %s! plid=%u", gRedis->last_error_cstr(), receiver);
			return ErrCodeDB;
		}
	}

	// 添加玩家收礼历史记录
	string recordKey(kKeyPrefixGiftGivenRecord + to_string(receiver));
	long long count = 0;
	cs::GiftRecvRecord rInfo;
	rInfo.set_giver_plid(req.giver_plid());
	rInfo.set_item_id(req.gift().item_id());
	rInfo.set_item_cnt(req.gift().item_cnt());
	rInfo.set_time(time(0));
	vector<string> vals(1);
	rInfo.SerializeToString(&(vals[0]));
	if (!gRedis->lpush(recordKey, vals, &count)) {
		WARN_LOG("lpush failed: %s! plid=%u", gRedis->last_error_cstr(), receiver);
		return ErrCodeDB;
	}
	if (count > 100) {
		string sResult;
		gRedis->rpop(recordKey, sResult);
	}

	// 添加玩家收礼价值排行(每个玩家单独排行)
	long long result = 0;
	if (!gRedis->zincrby(rankKey, to_string(req.giver_plid()), req.total_value(), &result)) {
		WARN_LOG("update failed: %s! plid=%u", gRedis->last_error_cstr(), receiver);
		return ErrCodeDB;
	}

	rsp.CopyFrom(req);

	// 查看新排行榜
	long long newRank = -1;
	gRedis->zrevrank(rankKey, to_string(req.giver_plid()), &newRank);
	if (newRank == 0 && oldRank != 0) {
		rsp.set_biggest_giver(true);
	}

	setPlayerDataTouchInfo(receiver, kKeyPrefixGiftItem, true);
	return ErrCodeSucc;
}

ErrCodeType PlayerData::getPlayerGiftInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(outMsg, cs::GiftItemData, rsp);

	unordered_map<string, string> gifts;
	string giftKey(kKeyPrefixGiftItem + to_string(h.TargetID));
	if (!gRedis->hgetall(giftKey, gifts)) {
		WARN_LOG("hgetall failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}

	for (auto v : gifts) {
		uint32_t itemId = atoi(v.first.c_str());
		uint32_t itemCnt = atoi(v.second.c_str());
		if (itemId > 0) {
			auto item = rsp.add_items();
			item->set_item_id(itemId);
			item->set_item_cnt(itemCnt);
		} else if (v.first.compare(kGiftItemFieldGemValue) == 0) {
			rsp.set_gem_value(itemCnt);
		}
	}
	
	setPlayerDataTouchInfo(h.TargetID, kKeyPrefixGiftItem, false);
	return ErrCodeSucc;
}

ErrCodeType PlayerData::getPlayerGiftRecvRecord(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(outMsg, db::GiftRecvRecordInfo, rsp);

	vector<string> result;
	string recordKey(kKeyPrefixGiftGivenRecord + to_string(h.TargetID));
	if (!gRedis->lrange(recordKey, 0, 100, result)) {
		WARN_LOG("lrange failed: %s! plid=%u", gRedis->last_error_message().c_str(), h.TargetID);
		return ErrCodeDB;
	}
	for (auto& s : result) {
		rsp.add_records()->ParseFromString(s);
	}

	// 获取彩钻值
	string giftKey(kKeyPrefixGiftItem + to_string(h.TargetID));
	unordered_map<string, string> m;
	string& gemValueStr = m[kGiftItemFieldGemValue];
	string& hisGemValueStr = m[kGiftItemFieldGemHistroyValue];
	if (!gRedis->hget(giftKey, m)) {
		WARN_LOG("hget failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}
	rsp.set_gem_value(atoi(gemValueStr.c_str()));
	rsp.set_his_gem_value(atoi(hisGemValueStr.c_str()));

	setPlayerDataTouchInfo(h.TargetID, kKeyPrefixGiftGivenRecord, false);
	return ErrCodeSucc;
}

ErrCodeType PlayerData::getPlayerGiftRecvRank(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(outMsg, db::GiftRecvRankInfo, rsp);

	vector<string> result;
	string recordKey(kKeyPrefixGiftGivenRank + to_string(h.TargetID));
	if (!gRedis->zrevrange(recordKey, 0, 100, result, true)) {
		WARN_LOG("zrevrank failed: %s!", gRedis->last_error_cstr());
		return ErrCodeDB;
	}

	auto iter = result.begin();
	for (; iter != result.end(); ++iter) {
		auto* record = rsp.mutable_records()->Add();
		record->set_giver_plid(atoi(iter->c_str()));
		++iter;
		record->set_total_value(atoi(iter->c_str()));
	}
	return ErrCodeSucc;
}

ErrCodeType PlayerData::getPlayerGiftRecordByPlid(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::RepeatedUint32Req, req);
	REAL_PROTOBUF_MSG(outMsg, db::GiftRecvRankInfo, rsp);

	string recordKey(kKeyPrefixGiftGivenRank + to_string(h.TargetID));
	for (int i = 0; i < req.u32_size(); i++) {
		auto plid = req.u32(i);
		int32_t value = 0;
		if (!gRedis->zscore(recordKey, to_string(plid), value)) {
			WARN_LOG("zscore failed: %s!", gRedis->last_error_cstr());
			return ErrCodeDB;
		}
		auto* record = rsp.mutable_records()->Add();
		record->set_giver_plid(plid);
		record->set_total_value(value);
	}
	return ErrCodeSucc;
}

ErrCodeType PlayerData::GiftGemExchange(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::Uint32Req, req);
	REAL_PROTOBUF_MSG(outMsg, db::GiftGemExchangeRsp, rsp);

	// 获取彩钻值
	string giftKey(kKeyPrefixGiftItem + to_string(h.TargetID));
	unordered_map<string, string> m;
	string& gemValueStr = m[kGiftItemFieldGemValue];
	if (!gRedis->hget(giftKey, m)) {
		WARN_LOG("hget failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}
	uint32_t curGemValue = atoi(gemValueStr.c_str());
	// 值不足
	if (curGemValue < req.u32()) {
		return ErrCodeGiftGemNotEnough;
	}

	// 扣除
	long long leftGem = 0;
	if (!gRedis->hincrby(giftKey, kGiftItemFieldGemValue, 0 - (int)req.u32(), &leftGem)) {
		WARN_LOG("hincrby failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}

	rsp.set_del_value(req.u32());
	rsp.set_left_value(leftGem);
	return ErrCodeSucc;
}

ErrCodeType PlayerData::modifyNameBirthPos(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	bool available = true;
	REAL_PROTOBUF_MSG(inMsg, cs::CSModifyNameBirthPos, req);

	//string key(kKeyPrefixNickname + req.name());
	string key(req.name());
	string playerID(to_string(h.PlayerID));
	//string orgPlayerID;
	RedisClient::ExpirationTime expire_tm(false, 0);

	bool keyExist;
	/*if (!gRedis->get(key, orgPlayerID, &keyExist)) {
		WARN_LOG("get failed: %s! plid=%u", gRedis->last_error_message().c_str(), h.PlayerID);
		return ErrCodeDB;
	}*/
	uint32_t orgPlid = 0;
	TblPlayerNick tblNick(h.PlayerID);
	if (tblNick.CheckNick(key, orgPlid, keyExist) != ErrCodeSucc) {
		return ErrCodeDB;
	}

	if (keyExist) {
		//if (orgPlayerID == playerID) {
		if (orgPlid == h.PlayerID) {
			available = false;
		} else {
			available = false;
		}
	} else {
		//if (!gRedis->set(key, playerID, 0, RedisClient::kSetIfNotExist)) {
		if (tblNick.InsertNick(key) != ErrCodeSucc) {
			available = false;
		}
	}
	REAL_PROTOBUF_MSG(outMsg, cs::CSModifyNameBirthPos, rsp);
	rsp.CopyFrom(req);
	if (!available) {
		rsp.clear_name();
	}

	return ErrCodeSucc;
}

ErrCodeType PlayerData::getTodayRandPlayers(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::GetGetTodayRandPlayersReq, req);

	vector<string> vals;
	if (!gRedis->srandmembers(makeTodayLoginedPlayersKey(req.game_region()), req.rand_limit(), vals)) {
		WARN_LOG("srandmembers failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}

	REAL_PROTOBUF_MSG(outMsg, db::RepeatedUint32Rsp, rsp);
	for (auto val : vals) {
		rsp.add_u32(atoi(val.c_str()));
	}

	return ErrCodeSucc;
}

ErrCodeType PlayerData::getPlayerGrpInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::RepeatedUint32Req, req);
	REAL_PROTOBUF_MSG(outMsg, db::GetPlayerGrpInfoRsp, rsp);

	string key;
	unordered_map<string, string> m;
	auto& grpInfoStr = m[kGroupInfo];
	db::GroupData grpInfo;
	for (int i = 0; i < req.u32_size(); ++i) {
		auto u32bool = rsp.add_u32bools();
		u32bool->set_u32(req.u32(i));

		key = kKeyPrefixPlayerData + to_string(req.u32(i));
		if (gRedis->hget(key, m) && grpInfoStr.size() && grpInfo.ParseFromString(grpInfoStr)) {
			u32bool->set_flag(grpInfo.group_id());
		}
	}

	return ErrCodeSucc;
}

ErrCodeType PlayerData::addOfflineMsg(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	// 注意：这个协议永远返回成功，错误码打包到rsp.err里
	REAL_PROTOBUF_MSG(inMsg, db::AddOfflineMsgReq, req);
	REAL_PROTOBUF_MSG(outMsg, db::AddOfflineMsgRsp, rsp);
	//DEBUG_LOG("======== addOfflineMsg. plid=%u target=%u", h.PlayerID, h.TargetID);

	rsp.set_allocated_rsp(req.release_rsp());
	if (req.sender_id()) {
		auto errCode = CheckIfInBlacklist(h.TargetID, req.sender_id());
		if (errCode != ErrCodeSucc) {
			rsp.set_err(errCode);
			return ErrCodeSucc;
		}
	}

	string key(kKeyPrefixOfflineMsg + to_string(h.TargetID));
	long long cnt = 0;
	vector<string> vals(1);
	vals[0].swap(*req.mutable_msg());
	if (!gRedis->rpush(key, vals, &cnt)) {
		WARN_LOG("rpush failed: %s! plid=%u", gRedis->last_error_message().c_str(), h.TargetID);
		rsp.set_err(ErrCodeDB);
		return ErrCodeSucc;
	}
	if (cnt > 1000) { // 最多保存1000条
		gRedis->ltrim(key, cnt - 1000, -1);
	}

	setPlayerDataTouchInfo(h.TargetID, kKeyPrefixOfflineMsg, true);
	return ErrCodeSucc;
}

ErrCodeType PlayerData::getOfflineMsgs(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	string key(kKeyPrefixOfflineMsg + to_string(h.TargetID));
	vector<string> vals;
	if (!gRedis->lrange(key, 0, -1, vals)) {
		WARN_LOG("lrange failed: %s! plid=%u", gRedis->last_error_message().c_str(), h.TargetID);
		return ErrCodeSucc; // 拉取离线消息失败不当成错误
	}

	REAL_PROTOBUF_MSG(outMsg, db::GetOfflineMsgsRsp, rsp);
	for (auto& val : vals) {
		rsp.add_msgs()->swap(val);
	}

	gRedis->del({key}); // 拉取后直接删除玩家的离线消息
	return ErrCodeSucc;
}

ErrCodeType PlayerData::allocNick(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	bool available = true;
	REAL_PROTOBUF_MSG(inMsg, db::AllocNickReq, req);

	//string key(kKeyPrefixNickname + req.nick());
	string key(req.nick());
	string playerID(to_string(h.PlayerID));
	//string orgPlayerID;
	//RedisClient::ExpirationTime expire_tm(false, req.expire_tm());

	bool keyExist;
	/*if (!gRedis->get(key, orgPlayerID, &keyExist)) {
		WARN_LOG("get failed: %s! plid=%u", gRedisNick->last_error_message().c_str(), h.PlayerID);
		return ErrCodeDB;
	}*/
	uint32_t orgPlid = 0;
	uint32_t expireTm = 0;
	TblPlayerNick tblNick(h.PlayerID);
	if (tblNick.CheckNick(key, orgPlid, expireTm, keyExist) != ErrCodeSucc) {
		return ErrCodeDB;
	}

	uint32_t setExpireTm = (req.expire_tm() > 0) ? req.expire_tm() + time(0) : 0;
	if (keyExist) {
		if (orgPlid == h.PlayerID) {
			/*if (req.expire_tm() > 0) {
				long long ttl;
				if (!gRedis->ttl(key, ttl)) {
					WARN_LOG("ttl failed: %s!", gRedis->last_error_message().c_str());
					return ErrCodeDB;
				}
				if (ttl != -1 && !gRedis->set(key, playerID, &expire_tm)) {
					available = false;
				}
			} else {
				if (!gRedis->set(key, playerID)) {
					WARN_LOG("set failed: %s!", gRedis->last_error_message().c_str());
					available = false;
				}
			}*/
			if (req.expire_tm() > 0) {
				if (expireTm > 0 && tblNick.UpdateExpireTm(key, setExpireTm) != ErrCodeSucc) {
					available = false;
				}
			} else {
				if (tblNick.UpdateExpireTm(key, 0) != ErrCodeSucc) {
					available = false;
				}
			}
		} else {
			available = false;
		}
	} else {
		/*if (req.expire_tm() > 0) {
			if (!gRedisNick->set(key, playerID, &expire_tm, RedisClient::kSetIfNotExist)) {
				WARN_LOG("set failed: %s!", gRedisNick->last_error_message().c_str());
				available = false;
			}
		} else {
			if (!gRedisNick->set(key, playerID, 0, RedisClient::kSetIfNotExist)) {
				WARN_LOG("set failed: %s!", gRedisNick->last_error_message().c_str());
				available = false;
			}
		}*/
		if (tblNick.InsertNick(key, setExpireTm) != ErrCodeSucc) {
			available = false;
		}
	}

	DEBUG_LOG("AllocNick plid=%u nick=%s tm=%u", h.PlayerID, req.nick().c_str(), req.expire_tm());
	REAL_PROTOBUF_MSG(outMsg, db::AllocNickRsp, rsp);
	rsp.set_nick(req.nick());
	rsp.set_avail(available);

	return ErrCodeSucc;
}

ErrCodeType PlayerData::releaseNick(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::ReleaseNickReq, req);

	//string key(kKeyPrefixNickname + req.nick());
	string key(req.nick());
	string playerID(to_string(req.player_id()));
	//string orgPlayerID;

	bool keyExist;
	/*if (!gRedis->get(key, orgPlayerID, &keyExist)) {
		WARN_LOG("Failed to del %s@%u! get: %s!", key.c_str(), req.player_id(),
			gRedisNick->last_error_message().c_str());
		return ErrCodeDB;
	}*/
	uint32_t orgPlid = 0;
	TblPlayerNick tblNick(h.PlayerID);
	if (tblNick.CheckNick(key, orgPlid, keyExist) != ErrCodeSucc) {
		return ErrCodeDB;
	}

	if (!keyExist) {
		return ErrCodeSucc;
	}

	//if (orgPlayerID != playerID) {
	if (orgPlid != req.player_id()) {
		return ErrCodeInvalidArgument;
	}

	/*std::vector<std::string> keysToDel;
	keysToDel.emplace_back(key);
	if (!gRedis->del(keysToDel)) {
		WARN_LOG("Failed to del %s@%u! del: %s!", key.c_str(), req.player_id(),
			gRedis->last_error_message().c_str());
		return ErrCodeDB;
	}*/
	if (tblNick.DeleteNick(key) != ErrCodeSucc) {
		return ErrCodeDB;
	}

	DEBUG_LOG("releaseNick plid=%u nick=%s", req.player_id(), req.nick().c_str());
	return ErrCodeSucc;
}

ErrCodeType PlayerData::getPlayerIDByNick(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::StrReq, req);

	//string key(kKeyPrefixNickname + req.str());
	string key(req.str());
	uint32_t playerID;
	bool keyExist;
	/*if (!gRedis->get(key, playerID, &keyExist)) {
		WARN_LOG("Failed to get %s: %s!", key.c_str(), gRedisNick->last_error_message().c_str());
		return ErrCodeDB;
	}*/
	TblPlayerNick tblNick(h.PlayerID);
	if (tblNick.CheckNick(key, playerID, keyExist) != ErrCodeSucc) {
		return ErrCodeDB;
	}

	if (!keyExist) {
		return ErrCodeNickNotFound;
	}

	REAL_PROTOBUF_MSG(outMsg, db::Uint32Rsp, rsp);
	rsp.set_u32(playerID);
	return ErrCodeSucc;
}

ErrCodeType PlayerData::getPlayerFormation(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::GetPlayerFormationReq, req);
	REAL_PROTOBUF_MSG(outMsg, db::DBMonsterFormation, rsp);

	string key(kKeyPrefixPlayerData + to_string(req.plid()));
	unordered_map<string, string> m;
	auto& formationStr = m[kMonsterFormation];
	if (!gRedis->hget(key, m)) {
		WARN_LOG("getPlayerFormation hget failed: %s!", gRedis->last_error_message().c_str());
		return ErrCodeDB;
	}

	db::AllMonsterFormation info;
	if (info.ParseFromString(formationStr)) {
		for (int i = 0; i < info.formations_size(); i++) {
			const auto& formation = info.formations(i);
			if (formation.type() == req.type()) {
				rsp.CopyFrom(formation);
				break;
			}
		}
	}
	
	setPlayerDataTouchInfo(h.TargetID, kKeyPrefixPlayerData, true);
	return ErrCodeSucc;
}

ErrCodeType PlayerData::banPlayer(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::BanReq, req);

	time_t banTime = 0;
	if (req.ban_time()) {
		banTime = time(0) + req.ban_time();
	}

	unordered_map<string, string> m;
	switch (req.ban_type()) {
	case 1: // login
		m[kBEDataFieldBanLogin] = to_string(banTime);
		break;
	case 2: // chat
		m[kBEDataFieldBanChat] = to_string(banTime);
		break;
	default:
		break;
	}
	if (!gRedis->hset(makeBasicExtraDataKey(to_string(h.TargetID)), m)) {
		DEBUG_LOG("Ban player failed! plid=%u banTime=%u err=%s",
				  h.TargetID, req.ban_time(), gRedis->last_error_cstr());
		return ErrCodeDB;
	}

	DEBUG_LOG("Ban player. plid=%u banTime=%u", h.TargetID, req.ban_time());
	REAL_PROTOBUF_MSG(outMsg, db::Uint32Rsp, rsp);
	rsp.set_u32(req.ban_time());
	return ErrCodeSucc;
}

ErrCodeType PlayerData::banDeviceOrIP(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::BanDeviceOrIP, req);

	string banKey = "";
	string memberKey = "";
	time_t banTime = 0;
	if (req.ban_time()) {
		banTime = time(0) + req.ban_time();
	}

	unordered_map<string, string> m;
	if (req.ban_by() == 1) { // 设备
		memberKey = makePlayerOfDeviceKey(req.target_id());
		if (req.ban_type() == 1) {
			banKey = kMiscFieldBanLoginDevice;
		}
		if (req.ban_type() == 2) {
			banKey = kMiscFieldBanChatDevice;
		}
	}
	if (req.ban_by() == 2) { // IP
		memberKey = makePlayerOfIPKey(req.target_id());
		if (req.ban_type() == 1) {
			banKey = kMiscFieldBanLoginIP;
		}
		if (req.ban_type() == 2) {
			banKey = kMiscFieldBanChatIP;
		}
	}
	if (banKey == "") {
		DEBUG_LOG("Wrong ban arguments! ban_by=%d, ban_type=%d", req.ban_by(), req.ban_type());
		return ErrCodeDB;
	}
	m[req.target_id()] = to_string(banTime);
	if (!gRedis->hset(banKey, m)) {
		DEBUG_LOG("Ban failed! ban_target=%s, ban_by=%d ban_type=%d err=%s",
				  req.target_id().c_str(), req.ban_by(), req.ban_type(), gRedis->last_error_cstr());
		return ErrCodeDB;
	}

	DEBUG_LOG("Ban DeviceOrIP. plid=%s banType=%u %u key=%s", req.target_id().c_str(), req.ban_by(), req.ban_type(), banKey.c_str());

	vector<string> ids;
	if (!gRedis->smembers(memberKey, ids)) {
		WARN_LOG("get player ids of device or ip failed! smember key=%s, err=%s", memberKey.c_str(), gRedis->last_error_cstr());
		return ErrCodeDB;
	}

	REAL_PROTOBUF_MSG(outMsg, db::RepeatedUint32Rsp, rsp);
	for (auto playerID : ids) {
		rsp.add_u32(atoi(playerID.c_str()));
	}
	return ErrCodeSucc;
}

ErrCodeType PlayerData::getMultiPlayerDevice(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	unordered_map<string, string> mDubiousDevice, mDubiousIP, mBanDevice, mBanIP;
	if (!gRedis->hgetall(kMiscFieldDubiousDevice, mDubiousDevice)) {
		WARN_LOG("Get dubious device failed: %s!", gRedis->last_error_cstr());
		return ErrCodeDB;
	}
	if (!gRedis->hgetall(kMiscFieldDubiousIP, mDubiousIP)) {
		WARN_LOG("Get dubious ip failed: %s!", gRedis->last_error_cstr());
		return ErrCodeDB;
	}
	if (!gRedis->hgetall(kMiscFieldBanLoginDevice, mBanDevice)) {
		WARN_LOG("Get ban login device failed: %s!", gRedis->last_error_cstr());
		return ErrCodeDB;
	}
	if (!gRedis->hgetall(kMiscFieldBanLoginIP, mBanIP)) {
		WARN_LOG("Get ban login ip failed: %s!", gRedis->last_error_cstr());
		return ErrCodeDB;
	}

	REAL_PROTOBUF_MSG(outMsg, db::MultiPlayerDeviceRsp, rsp);

	uint32_t cnt = 0;
	for (auto& it : mDubiousDevice) {
		cnt = atoi(it.second.c_str());
		if (cnt < 5) continue;
		auto info = rsp.add_info();
		info->set_device(it.first);
		info->set_cnt(cnt);
		auto banIt = mBanDevice.find(it.first);
		info->set_ban(banIt != mBanDevice.end() && time(0) < atoi(banIt->second.c_str()));
	}
	for (auto& it : mDubiousIP) {
		cnt = atoi(it.second.c_str());
		if (cnt < 5) continue;
		auto info = rsp.add_info();
		info->set_ip(it.first);
		info->set_cnt(cnt);
		auto banIt = mBanIP.find(it.first);
		info->set_ban(banIt != mBanIP.end() && time(0) < atoi(banIt->second.c_str()) ? 1 : 0);
	}

	return ErrCodeSucc;
}

ErrCodeType PlayerData::onAddFailedSyncMomoItem(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::SyncMomoItemData, req);
	if (!gRedis->hincrby(makeFailedSyncMomoItemKey(h.TargetID), req.post_data(), 1)) {
		WARN_LOG("hincrby failed sync momo item failed! err=%s, plid=%u", gRedis->last_error_cstr(), h.TargetID);
	}
	REAL_PROTOBUF_MSG(outMsg, db::SyncMomoItemData, rsp);
	rsp.Swap(&req);
	return ErrCodeSucc;
}

ErrCodeType PlayerData::onGetFailedSyncMomoItem(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	unordered_map<string, string> m;
	if (!gRedis->hgetall(makeFailedSyncMomoItemKey(h.TargetID), m)) {
		WARN_LOG("hgetall failed sync momo item failed! err=%s, plid=%u", gRedis->last_error_cstr(), h.TargetID);
	}
	REAL_PROTOBUF_MSG(outMsg, db::RepeatedStrRsp, rsp);
	for (auto it : m) {
		rsp.add_strs(it.first);
	}
	return ErrCodeSucc;
}

ErrCodeType PlayerData::onDelFailedSyncMomoItem(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::StrReq, req);
	vector<string> toDel = { req.str() };
	if (!gRedis->hdel(makeFailedSyncMomoItemKey(h.TargetID), toDel)) {
		WARN_LOG("hdel failed sync momo item failed! err=%s, plid=%u", gRedis->last_error_cstr(), h.TargetID);
	}
	return ErrCodeSucc;
}

ErrCodeType PlayerData::banChannel(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::BanPayReq, req);

	string banKey = "";
	time_t banTime = 0;
	if (req.ban_time()) {
		banTime = time(0) + req.ban_time();
	}
	unordered_map<string, string> m;
	m[req.channel()] = to_string(banTime);
	switch (req.ban_type()) {
	case 1: // login
		banKey = kMiscFieldBanChannelLogin;
		break;
	case 2: // pay
		banKey = kMiscFieldBanChannelPay;
		break;
	default:
		WARN_LOG("Invalid ban channel type! type=%d", req.ban_type());
		return ErrCodeSucc;
	}
	if (!gRedis->hset(banKey, m)) {
		WARN_LOG("Ban pay channel failed! chanle=%s, err=%s", req.channel().c_str(), gRedis->last_error_cstr());
	}

	return ErrCodeSucc;
}

ErrCodeType PlayerData::gsMember(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::GsMemberInfo, req);
	REAL_PROTOBUF_MSG(outMsg, db::GsMemberListInfo, rsp);

	unordered_map<string, string> m;
	string key(kKeyPrefixPlayerData + to_string(h.TargetID));
	const auto& accInfoStr = m[kAccountInfo];
	if (!gRedis->hget(key, m)) {
		DEBUG_LOG("update gsMember failed, err=%s", gRedis->last_error_message().c_str());
		return ErrCodeDB;
	}
	db::AccountInfo acc;
	acc.ParseFromString(accInfoStr);
	
	// 这里的存储用于gm操作
	m.clear();
	auto& str = m[to_string(req.pid())];
	req.SerializeToString(&str);
	if (!gRedis->hset(kKeyGsMember, m)) {
		DEBUG_LOG("update gsMember failed, err=%s", gRedis->last_error_message().c_str());
		return ErrCodeDB;
	}

	// 在玩家的social数据里面也存一份，用于游戏中使用
	m.clear();
	auto& str1 = m[kSocialFieldGsMember];
	req.SerializeToString(&str1);
	if (!gRedis->hset(kKeyPrefixSocialData + to_string(req.pid()), m)) {
		DEBUG_LOG("update gsMember failed, err=%s", gRedis->last_error_message().c_str());
		return ErrCodeDB;
	}

	auto unit = rsp.add_units();
	unit->set_pid(req.pid());
	unit->set_pname(acc.player_name());
	unit->set_time_beg(req.time_beg());
	unit->set_time_end(req.time_end());
	unit->set_is_rec(req.is_rec());

	DEBUG_LOG("gsMember, req_plid:%u, req_time_beg:%d, req_time_end:%d, rsp_mem_size:%d", req.pid(), req.time_beg(), req.time_end(), rsp.units_size());

	// 从排行榜中删除(玫瑰榜 助战榜 星光榜)
	cs::GeoPos geo;
	ErrCodeType errCode = GeoData::GetGeoData(req.pid(), geo);
	if (errCode == ErrCodeType::ErrCodeSucc) {
		int gameRegion = GetGameRegion(req.pid());
		geo.set_game_region(gameRegion);
		string strPlayerID = to_string(req.pid());
		RankData::DoDelGeoRank(strPlayerID, geo, kSocialFieldPopular);
		gBehaviourRank.DelRankInfo(req.pid(), gameRegion);
	}

	return ErrCodeSucc;
}

ErrCodeType PlayerData::gsMemberList(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(outMsg, db::GsMemberListInfo, rsp);

	unordered_map<string, string> m;
	if (!gRedis->hgetall(kKeyGsMember, m)) {
		DEBUG_LOG("gsMemberList failed, err=%s", gRedis->last_error_message().c_str());
		return ErrCodeDB;
	}

	for (auto& it : m) {
		db::GsMemberInfo tmp;
		tmp.ParseFromString(it.second);
		if (!tmp.time_beg() || !tmp.time_end()) {
			continue;
		}

		unordered_map<string, string> m1;
		string key(kKeyPrefixPlayerData + it.first);
		const auto& accInfoStr = m1[kAccountInfo];
		if (!gRedis->hget(key, m1)) {
			DEBUG_LOG("gsMemberList failed, err=%s", gRedis->last_error_message().c_str());
			continue;
		}
		db::AccountInfo acc;
		acc.ParseFromString(accInfoStr);

		auto unit = rsp.add_units();
		unit->set_pid(tmp.pid());
		unit->set_pname(acc.player_name());
		unit->set_time_beg(tmp.time_beg());
		unit->set_time_end(tmp.time_end());
		unit->set_is_rec(tmp.is_rec());
	}

	DEBUG_LOG("gsMemberList, member size=%d", rsp.units_size());
	return ErrCodeSucc;
}

ErrCodeType PlayerData::gsRecRoomUpdate(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, cs::SCRepeatedUint32Rsp, req);
	std::string tmp;
	req.SerializeToString(&tmp);
	if (!gRedis->set(kKeyGsRecRoom, tmp)) {
		DEBUG_LOG("gsRecRoomUpdate failed, err=%s", gRedis->last_error_message().c_str());
		return ErrCodeDB;
	}
	return ErrCodeSucc;
}

ErrCodeType PlayerData::gsRecRoomList(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(outMsg, cs::SCRepeatedUint32Rsp, rsp);
	std::string tmp;
	if (!gRedis->get(kKeyGsRecRoom, tmp)) {
		DEBUG_LOG("gsRecRoomList failed, err=%s", gRedis->last_error_message().c_str());
		return ErrCodeDB;
	}
	rsp.ParseFromString(tmp);
	return ErrCodeSucc;
}

ErrCodeType PlayerData::onChangePlayerNameForcible(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(outMsg, cs::SCStringRsp, rsp);
	DEBUG_LOG("onChangePlayerNameForcible %u", h.TargetID);

	string strPlayerID = to_string(h.TargetID);
	string key(kKeyPrefixPlayerData + strPlayerID);
	unordered_map<string, string> m;
	string& accountInfoStr = m[kAccountInfo];
	if (!gRedis->hget(key, m)) {
		WARN_LOG("hgetall failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}


	db::AccountInfo info;
	info.ParseFromString(accountInfoStr);
	string namekey = kKeyRandMaleName;
	if (info.gender() == 2) {
		namekey = kKeyRandFemaleName;
	}

	bool flag = true;
	for(int i = 0; i < 10000; i++) {
		vector<string> pname;
		if (!gRedis->srandmembers(namekey, 1, pname)) {
			WARN_LOG("hgetall failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
			return ErrCodeDB;
		}
		if (pname.size() == 0) {
			break;
		}
		//string indexkey(kKeyPrefixNickname + pname[0]);
		string indexkey(pname[0]);
		//uint32_t playerID;
		bool keyExist;
		/*if (!gRedis->get(indexkey, playerID, &keyExist)) {
			WARN_LOG("Failed to get %s: %s!", key.c_str(), gRedisNick->last_error_message().c_str());
			return ErrCodeDB;
		}*/
		TblPlayerNick tblNick(h.TargetID);
		if (tblNick.CheckNick(indexkey, keyExist) != ErrCodeSucc) {
			return ErrCodeDB;
		}

		if (!keyExist) {
			flag = false;
			/*
			if (!gRedis->hdel(indexkey, {info.player_name()})) {
				WARN_LOG("Failed to get %s: %s!", key.c_str(), gRedis->last_error_message().c_str());
				return ErrCodeDB;
			}*/
			/*unordered_map<string, string> mkey;
			mkey[pname[0]] = to_string(h.TargetID);
			if (!gRedis->set(indexkey, to_string(h.TargetID))) {
				WARN_LOG("Failed to get %s: %s!", key.c_str(), gRedis->last_error_message().c_str());
			}*/
			if (tblNick.InsertNick(indexkey) != ErrCodeSucc) {
				return ErrCodeDB;
			}

			rsp.set_str(pname[0]);
			info.set_player_name(pname[0]);
			info.SerializeToString(&accountInfoStr);
			if (!gRedis->hset(key, m)) {
				WARN_LOG("hgetall failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
				return ErrCodeDB;
			}
			if (!gRedis->srem(namekey, pname)) {
				WARN_LOG("sadd failed: %s!", gRedis->last_error_cstr());
			}
			break;
		} else {
			if (!gRedis->srem(namekey, pname)) {
				WARN_LOG("sadd failed: %s!", gRedis->last_error_cstr());
			}
		}
	}

	if (flag) {
		DEBUG_LOG("Name out");
		rsp.set_str("name out");
		return ErrCodeDB;
	}
	return ErrCodeSucc;
}

ErrCodeType PlayerData::onChannelSwitchUpdate(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, GmMsg::GmChannelSwitchUnit, req);
	DEBUG_LOG("onChannelUpdate, channel_id:%s, login:%d, pay:%d, role:%d", req.channel_id().c_str(), req.switch_login(), req.switch_pay(), req.switch_role());
	unordered_map<string, string> m;
	auto& str = m[req.channel_id()];
	req.SerializeToString(&str);
	if (!gRedis->hset(kKeyChannelSwitch, m)) {
		DEBUG_LOG("onChannelSwitchUpdate failed, err=%s", gRedis->last_error_message().c_str());
		return ErrCodeDB;
	}

	uint32_t lgtime = 0;
	uint32_t roletime  = 0;
	uint32_t paytime = 0;

	if (req.switch_login() == 2) {
		lgtime = 2000000000;
	}

	if (req.switch_role() == 2) {
		roletime = 2000000000;
	}

	if (req.switch_pay() == 2) {
		paytime = 2000000000;
	}



	m[req.channel_id()] = to_string(lgtime);
	// 渠道封禁登录
	if (!gRedis->hset(kMiscFieldBanChannelLogin, m)) {
		WARN_LOG("hget failed: %s! plid=%u", gRedis->last_error_message().c_str(), h.TargetID);
		return ErrCodeDB;
	}

	m[req.channel_id()] = to_string(roletime);
	// 渠道封禁创角色
	if (!gRedis->hset(kMiscFieldBanChannelCreate, m)) {
			WARN_LOG("hget failed: %s! plid=%u", gRedis->last_error_message().c_str(), h.TargetID);
			return ErrCodeDB;
	}

	m[req.channel_id()] = to_string(paytime);
	// 渠道封禁支付
	if (!gRedis->hset(kMiscFieldBanChannelPay, m)) {
		WARN_LOG("hget failed: %s! plid=%u", gRedis->last_error_message().c_str(), h.TargetID);
		return ErrCodeDB;
	}



	return ErrCodeSucc;
}

ErrCodeType PlayerData::onChannelSwitchList(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(outMsg, GmMsg::GmChannelSwitchInfo, rsp);
	unordered_map<string, string> m;
	if (!gRedis->hgetall(kKeyChannelSwitch, m)) {
		DEBUG_LOG("onChannelSwitchList failed, err=%s", gRedis->last_error_message().c_str());
		return ErrCodeDB;
	}
	for (auto& it : m) {
		auto p = rsp.add_units();
		p->ParseFromString(it.second);
	}
	return ErrCodeSucc;
}

ErrCodeType PlayerData::gmSelfTest(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::GmSelfTestInfo, req);

	DEBUG_LOG("gmSelfTest,  req=%s", req.Utf8DebugString().c_str());
	SSProtoHead fh;
	db::Uint32Rsp rsp;
	fh.TargetID = req.src_pid();
	if (req.type() == 0) { // ban chat
		fh.ProtoID = DBProtoBanPlayer;

		db::BanReq r;
		r.set_ban_type(2);
		r.set_ban_time(600);
		banPlayer(fh, &r, &rsp);
	}
	else if (req.type() == 1) { // unban chat
		fh.ProtoID = DBProtoBanPlayer;

		db::BanReq r;
		r.set_ban_type(2);
		r.set_ban_time(0);
		banPlayer(fh, &r, &rsp);
	}
	else if (req.type() == 2) { // ban login
		fh.TargetID = req.dst_pid();
		fh.ProtoID = DBProtoBanPlayer;

		db::BanReq r;
		r.set_ban_type(1);
		r.set_ban_time(600);
		banPlayer(fh, &r, &rsp);
	}
	else if (req.type() == 3) { // unban login
		fh.TargetID = req.dst_pid();
		fh.ProtoID = DBProtoBanPlayer;

		db::BanReq r;
		r.set_ban_type(1);
		r.set_ban_time(0);
		banPlayer(fh, &r, &rsp);
	} 
	else if (req.type() == 4) { // gsmem
		fh.ProtoID = DBProtoGsMember;

		db::GsMemberInfo r;
		db::GsMemberListInfo rsp1;
		r.set_pid(req.src_pid());
		r.set_time_beg(time(nullptr));
		r.set_time_end(time(nullptr) + req.dst_pid() * 60);
		gsMember(fh, &r, &rsp1);
	}

	return ErrCodeSucc;
}

ErrCodeType PlayerData::getShopParam(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(outMsg, db::ShopParam, rsp);
	unordered_map<string, string> m;
	if (!gRedis->hgetall(kMiscFieldBanChannelPay, m)) {
		WARN_LOG("Get pay channel failed! %s", gRedis->last_error_cstr());
	}
	for (auto it : m) {
		auto ban = rsp.add_info();
		ban->set_channel(it.first);
		ban->set_ban_time(atoi(it.second.c_str()));
	}
	return ErrCodeSucc;
}

static const string kScriptCheckIfCanFollow =
		// "if redis.call('SISMEMBER', KEYS[1], ARGV[1]) == 1 then\n" // 判断是否在对方黑名单里
		"if redis.call('HEXISTS', KEYS[1], ARGV[1]) == 1 then\n"
		"  return 1\n"
		"end\n"
		// "local r = redis.call('HLEN', KEYS[2])\n"
		// "if r >= 120 then\n" // 判断对方粉丝数是否已满
		// "  return 2\n"
		// "end\n"
		"return 0";
static const string kScriptFollowPlayer =
		"if redis.call('HLEN', KEYS[1]) >= 100 then\n" // 检查本人是否还能加关注
		"  return 3\n"
		"end\n"
		"redis.call('HINCRBY', KEYS[1], ARGV[1], ARGV[2])\n" // 加入本人关注列表。使用hincrby，确保不会因为时序问题导致友情值被覆盖
		// "redis.call('SREM', KEYS[2], ARGV[1], 0)\n" // 从黑名单中删除
		"redis.call('HDEL', KEYS[2], ARGV[1])\n" // 从黑名单中删除
		"if tonumber(ARGV[3]) > 0 then\n" // 加入互相关注
		  "redis.call('SADD', KEYS[3], ARGV[1])\n"
		"end\n"
		"return 0";
static const ErrCodeType kFollowPlayerErrCodes[] = {
		ErrCodeSucc, ErrCodeYouInBlacklist, ErrCodeFollowersLimit, ErrCodeFollowersLimit
};

ErrCodeType PlayerData::followPlayer(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::FollowPlayerReq, req);

	uint32_t plid = req.player_id();
	if (req.player_name().size()) {
		//string nickKey(kKeyPrefixNickname + req.player_name());
		string nickKey(req.player_name());
		bool exist;
		/*if (!gRedis->get(nickKey, plid, &exist)) {
			DEBUG_LOG("Failed to get %s! plid=%u err=%s",
					  nickKey.c_str(), h.PlayerID, gRedis->last_error_message().c_str());
			return ErrCodeNoPlayerWithTheGivenName;
		}*/
		TblPlayerNick tblNick(plid);
		if (tblNick.CheckNick(nickKey, plid, exist) != ErrCodeSucc) {
			return ErrCodeDB;
		}

		if (!exist) {
			DEBUG_LOG("No player with the given name '%s'! plid=%u",
					  req.player_name().c_str(), h.PlayerID);
			return ErrCodeNoPlayerWithTheGivenName;
		}

	} else { // 检查给定的ID是否存在
		string pdKey(kKeyPrefixPlayerData + to_string(plid));
		bool exist;
		if (!gRedis->exists(pdKey, exist)) {
			WARN_LOG("exists failed! plid=%u err=%s", h.PlayerID, gRedis->last_error_cstr());
			return ErrCodeDB;
		}
		if (!exist) {
			DEBUG_LOG("No such player!. plid=%u following=%u", h.PlayerID, plid);
			return ErrCodeNoPlayerWithTheGivenID;
		}
	}

	if (h.PlayerID == plid) {
		DEBUG_LOG("Can't follow yourself! plid=%u", h.PlayerID);
		return ErrCodeInvalidArgument;
	}

	string myPlayerIDStr = to_string(h.PlayerID);
	string otherFollowerKey = makeFollowerKey(plid);
	string otherFollowingKey = makeFollowingKey(plid);
	// 检查对方是否可被关注
	vector<string> keys = { makeBlacklistKey(plid), otherFollowerKey };
	vector<string> args = { myPlayerIDStr };
	ScopedReplyPointer reply = gRedis->eval(kScriptCheckIfCanFollow, &keys, &args);
	CHECK_REPLY_EC(reply, REDIS_REPLY_INTEGER, h.PlayerID);
	if (reply->integer != 0) {
		DEBUG_LOG("Follow failed! plid=%u follow=%u errCode=%u",
				  h.PlayerID, plid, kFollowPlayerErrCodes[reply->integer]);
		return kFollowPlayerErrCodes[reply->integer];
	}
	// 检查对方是否已关注了自己,并同步友好度
	bool hasFollowing;
	if (!gRedis->hexists(otherFollowingKey, myPlayerIDStr, hasFollowing)) {
		WARN_LOG("hexists failed! err=%s plid=%u", gRedis->last_error_cstr(), h.PlayerID);
		return ErrCodeDB;
	}
	unordered_map<string, string> m;
	string& strFriendship = m[myPlayerIDStr];
	if (!gRedis->hget(otherFollowingKey, m)) {
		WARN_LOG("hget failed! plid=%u", h.PlayerID);
		return ErrCodeDB;
	}
	int friendship = atoi(strFriendship.c_str());
	// 把对方加入关注列表，并从黑名单中移除
	keys = { makeFollowingKey(h.PlayerID), makeBlacklistKey(h.PlayerID), makeIntersectionKey(h.PlayerID) };
	args = { to_string(plid), to_string(friendship), hasFollowing ? "1" : "0" };
	reply = gRedis->eval(kScriptFollowPlayer, &keys, &args);
	CHECK_REPLY_EC(reply, REDIS_REPLY_INTEGER, h.PlayerID);
	if (reply->integer != 0) {
		DEBUG_LOG("Follow failed! plid=%u follow=%u errCode=%u",
				  h.PlayerID, plid, kFollowPlayerErrCodes[reply->integer]);
		return kFollowPlayerErrCodes[reply->integer];
	}
	// 加到对方的粉丝列表，不判断是否成功
	// gRedis->hincrby(otherFollowerKey, myPlayerIDStr, friendship);
	gRedisRank->zadd(makeFansKey(plid), time(nullptr), to_string(h.PlayerID));
	// 加对方互相关注
	if (hasFollowing) {
		vector<string> addInter = { myPlayerIDStr };
		gRedis->sadd(makeIntersectionKey(plid), addInter);
	}

	DEBUG_LOG("Follow. plid=%u following=%u", h.PlayerID, plid);
	REAL_PROTOBUF_MSG(outMsg, db::FollowPlayerRsp, rsp);
	rsp.set_player_id(plid);
	if (req.player_name().size()) {
		rsp.set_allocated_player_name(req.release_player_name());
	}
	rsp.set_friendship(friendship);
	rsp.set_follow_each_other(hasFollowing);
	// 对方是momo用户的话要设momoid
	string regKey(kKeyPrefixPlayerData + to_string(plid));
	unordered_map<string, string> mReg;
	auto& regInfoStr = mReg[kRegInfo];
	if (!gRedis->hget(regKey, mReg)) {
		WARN_LOG("hget failed: %s! plid=%u", gRedis->last_error_message().c_str(), h.PlayerID);
		return ErrCodeDB;
	}
	db::RegInfo regInfo;
	regInfo.ParseFromString(regInfoStr);
	if (regInfo.account().account_type() == kChannelNameMomo) {
		rsp.set_momo_id(regInfo.account().account_id());
	}
	return ErrCodeSucc;
}

static string kScriptRemoveFollowing =
		"redis.call('SREM', KEYS[1], unpack(ARGV))\n"
		"redis.call('HDEL', KEYS[2], unpack(ARGV))\n"
		"redis.call('SREM', KEYS[3], unpack(ARGV))\n"
		"return 1";

ErrCodeType PlayerData::removeFollowings(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::RepeatedUint32Req, req);

	string strPlayerID = to_string(h.TargetID);
	vector<string> followings;
	vector<string> followers = { strPlayerID };
	for (int i = 0; i < req.u32_size(); ++i) {
		uint32_t followingPlayer = req.u32(i);
		string followerKey = makeFollowerKey(followingPlayer);
		if (gRedis->hdel(followerKey, followers)) {
			string strFollowingPlayer = to_string(followingPlayer);
			followings.emplace_back(strFollowingPlayer);
			// 从对方的互相关注中删除我
			gRedis->srem(makeIntersectionKey(followingPlayer), followers);
			// 从对方的粉丝榜中删除我
			gRedisRank->zrem(makeFansKey(followingPlayer), strPlayerID);
		}
	}

	vector<string> keys = { makeFavFollowingKey(h.TargetID), makeFollowingKey(h.TargetID), makeIntersectionKey(h.TargetID) };
	gRedis->eval_only(kScriptRemoveFollowing, &keys, &followings);

	DEBUG_LOG("Remove followings. plid=%u cnt=%d %zu", h.PlayerID, req.u32_size(), followings.size());
	return ErrCodeSucc;
}

ErrCodeType PlayerData::removeFollowers(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::RepeatedUint32Req, req);

	// 先移除粉丝
	vector<string> followers;
	for (int i = 0; i < req.u32_size(); ++i) {
		followers.emplace_back(to_string(req.u32(i)));
	}
	if (!gRedis->hdel(makeFollowerKey(h.TargetID), followers)) {
		WARN_LOG("hdel failed! plid=%u err=%s", h.TargetID, gRedis->last_error_cstr());
		return ErrCodeDB;
	}
	if (!gRedisRank->zrem(makeFansKey(h.TargetID), followers)) {
		WARN_LOG("zrem failed! plid=%u err=%s", h.TargetID, gRedis->last_error_cstr());
		return ErrCodeDB;
	}
	// 从我的互相关注中删除粉丝
	if (!gRedis->srem(makeIntersectionKey(h.TargetID), followers)) {
		WARN_LOG("srem failed! plid=%u err=%s", h.TargetID, gRedis->last_error_cstr());
		return ErrCodeDB;
	}

	// 从关注我的人的关注列表中把我移除
	vector<string> keys(3);
	vector<string> followings = { to_string(h.TargetID) };
	for (int i = 0; i < req.u32_size(); ++i) {
		keys[0] = makeFavFollowingKey(req.u32(i));
		keys[1] = makeFollowingKey(req.u32(i));
		keys[2] = makeIntersectionKey(req.u32(i));
		gRedis->eval_only(kScriptRemoveFollowing, &keys, &followings);
	}

	DEBUG_LOG("Remove followers. plid=%u cnt=%d %zu", h.PlayerID, req.u32_size(), followers.size());
	return ErrCodeSucc;
}

ErrCodeType PlayerData::addFavFollowing(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::Uint32Req, req);

	string favFollowingID = to_string(req.u32());

	// 判断是否已经关注了
	bool exist;
	if (!gRedis->hexists(makeFollowingKey(h.TargetID), favFollowingID, exist)) {
		WARN_LOG("hexists failed! plid=%u err=%s", h.TargetID, gRedis->last_error_cstr());
		return ErrCodeDB;
	}
	if (!exist) {
		DEBUG_LOG("Must follow first! plid=%u fav=%u", h.TargetID, req.u32());
		return ErrCodeCantFavNonfollowing;
	}

	// 判断特别关注列表是否已经满了
	string favFollowingKey = makeFavFollowingKey(h.TargetID);
	long long cnt;
	if (!gRedis->scard(favFollowingKey, cnt)) {
		WARN_LOG("scard failed! plid=%u err=%s", h.TargetID, gRedis->last_error_cstr());
		return ErrCodeDB;
	}
	if (cnt >= kMaxFavFollowingCnt) {
		DEBUG_LOG("Fav followings limit! plid=%u", h.TargetID);
		return ErrCodeFavFollowingsLimit;
	}

	if (!gRedis->sadd(favFollowingKey, { std::move(favFollowingID) })) {
		WARN_LOG("sadd failed! plid=%u err=%s", h.TargetID, gRedis->last_error_cstr());
		return ErrCodeDB;
	}

	DEBUG_LOG("Fav follow. plid=%u fav=%u", h.TargetID, req.u32());
	REAL_PROTOBUF_MSG(outMsg, db::Uint32Rsp, rsp);
	rsp.set_u32(req.u32());
	return ErrCodeSucc;
}

ErrCodeType PlayerData::removeFavFollowing(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::Uint32Req, req);

	DEBUG_LOG("Remove fav follow. plid=%u fav=%u", h.TargetID, req.u32());
	gRedis->srem(makeFavFollowingKey(h.TargetID), { to_string(req.u32()) });

	return ErrCodeSucc;
}

static string kScriptAddFollowingFriendship =
		"if redis.call('HEXISTS', KEYS[1], ARGV[1]) == 0 then\n"
		"  return -1\n"
		"end\n"
		"return redis.call('HINCRBY', KEYS[1], ARGV[1], ARGV[2])\n";
// static string kScriptAddFollowerFriendship =
// 		"if redis.call('HEXISTS', KEYS[1], ARGV[1]) == 0 then\n"
// 		"  if redis.call('HLEN', KEYS[1]) >= tonumber(ARGV[3]) then\n"
// 		"    return -1\n"
// 		"  end\n"
// 		"end\n"
// 		"return redis.call('HSET', KEYS[1], ARGV[1], ARGV[2])\n";
// static string kScriptAddFriendship = 
// 		"if redis.call('HEXISTS', KEYS[1], ARGV[1]) == 0 then\n" // 检查我是否已关注对方
// 		"  return -1\n"
// 		"end\n"
// 		"local friendship = redis.call('HINCRBY', KEYS[1], ARGV[1], ARGV[2])\n" // inc我关注列表中对方的友好度
// 		"if redis.call('HEXISTS', KEYS[2], ARGV[3]) == 0 then\n" // 检查对方是否已关注我
// 		"  return friendship\n"
// 		"end\n"
// 		"redis.call('HSET', KEYS[2], ARGV[3], friendship)\n" // set对方关注列表中对我的友好度
// 		"return friendship";

ErrCodeType PlayerData::addFriendship(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::RepeatedUint32Req, req);
	if (req.u32_size() < 2) {
		DEBUG_LOG("Invalid req! plid=%u argCnt=%d", h.TargetID, req.u32_size());
		return ErrCodeInvalidArgument;
	}

	// 加对关注玩家的友好度      #和反向友好度,粉丝友好度及反向粉丝友好度
	vector<string> keys = { makeFollowingKey(h.TargetID)};
	vector<string> args = { to_string(req.u32(0)), to_string(req.u32(1)) };
	ScopedReplyPointer reply = gRedis->eval(kScriptAddFollowingFriendship, &keys, &args);
	CHECK_REPLY_EC(reply, REDIS_REPLY_INTEGER, h.TargetID);
	if (reply->integer < 0) {
		DEBUG_LOG("No such following! plid=%u follow=%u", h.TargetID, req.u32(0));
		return ErrCodeInvalidArgument;
	}
	// // 加粉丝的友好度
	// keys = { makeFollowerKey(req.u32(0)) } ;
	// args = { to_string(h.TargetID), to_string(reply->integer), to_string(kMaxFollowerCnt) };
	// gRedis->eval_only(kScriptAddFollowerFriendship, &keys, &args);

	// 加反向的友好度
	vector<string> revKeys = { makeFollowingKey(req.u32(0)) };
	vector<string> revArgs = { to_string(h.TargetID), to_string(req.u32(1)) };
	ScopedReplyPointer revReply = gRedis->eval(kScriptAddFollowingFriendship, &revKeys, &revArgs);
	CHECK_REPLY_EC(revReply, REDIS_REPLY_INTEGER, h.TargetID);
	// if (revReply->integer > 0) {
	// 	// 加反向粉丝的友好度
	// 	revKeys = { makeFollowerKey(h.TargetID) } ;
	// 	revArgs = { to_string(req.u32(0)), to_string(revReply->integer), to_string(kMaxFollowerCnt) };
	// 	gRedis->eval_only(kScriptAddFollowerFriendship, &revKeys, &revArgs);
	// }

	DEBUG_LOG("Add friend ship. plid=%u target=%u cnt=%u", h.TargetID, req.u32(0), req.u32(1));
	REAL_PROTOBUF_MSG(outMsg, db::ForMomoAddFriendShip, rsp);
	rsp.set_friend_id(req.u32(0));
	// 对方是momo用户的话要同步友好度
	string key(kKeyPrefixPlayerData + to_string(req.u32(0)));
	unordered_map<string, string> m;
	auto& regInfoStr = m[kRegInfo];
	if (!gRedis->hget(key, m)) {
		WARN_LOG("hget failed: %s! plid=%u", gRedis->last_error_message().c_str(), h.TargetID);
		return ErrCodeDB;
	}
	db::RegInfo regInfo;
	regInfo.ParseFromString(regInfoStr);
	if (regInfo.account().account_type() == kChannelNameMomo) {
		rsp.set_momo_id(regInfo.account().account_id());
	}
	rsp.set_friendship(reply->integer > revReply->integer ? reply->integer : revReply->integer);
	return ErrCodeSucc;
}

// 加入黑名单时，从特别关注、关注和粉丝列表中移除
static string kScriptAddToBlacklist =
		// "if redis.call('SCARD', KEYS[4]) >= 200 then\n"
		"if redis.call('HLEN', KEYS[4]) >= 200 then\n"
		"  return -1\n"
		"end\n"
		"redis.call('SREM', KEYS[1], ARGV[1])\n" // 特别关注
		"redis.call('HDEL', KEYS[2], ARGV[1])\n" // 关注
		"redis.call('HDEL', KEYS[3], ARGV[1])\n" // 粉丝
		// "redis.call('ZREM', KEYS[3], ARGV[1])\n" // 粉丝
		// "redis.call('SADD', KEYS[4], ARGV[1])\n" // 加入黑名单
		"redis.call('HSET', KEYS[4], ARGV[1], ARGV[2])\n" // 加入黑名单
		"return 0";
static string kScriptRemoveFriend =
		"redis.call('SREM', KEYS[1], ARGV[1])\n" // 特别关注
		"redis.call('HDEL', KEYS[2], ARGV[1])\n" // 关注
		"redis.call('HDEL', KEYS[3], ARGV[1])\n" // 粉丝
		// "redis.call('ZREM', KEYS[3], ARGV[1])\n" // 粉丝
		"return 0";

ErrCodeType PlayerData::addToBlacklist(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::Uint32Req, req);
	DEBUG_LOG("Add to blacklist. plid=%u black=%u", h.TargetID, req.u32());

	// 我是对方的粉丝
	long long fansRank = 0;
	if (!gRedisRank->zrank(makeFansKey(req.u32()), to_string(h.TargetID), &fansRank)) {
		WARN_LOG("zrank failed! err=%s, plid=%u", gRedisRank->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}
	// 对方是我的粉丝
	bool isFollowed;
	if (!gRedis->hexists(makeFollowingKey(req.u32()), to_string(h.TargetID), isFollowed)) {
		WARN_LOG("hexists failed! err=%s, plid=%u", gRedisRank->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}
	// 加入本人黑名单
	vector<string> keys = {
		makeFavFollowingKey(h.TargetID), makeFollowingKey(h.TargetID),
		makeFollowerKey(h.TargetID), makeBlacklistKey(h.TargetID)
	};
	vector<string> args = { to_string(req.u32()), to_string(time(nullptr)) };
	ScopedReplyPointer reply = gRedis->eval(kScriptAddToBlacklist, &keys, &args);
	CHECK_REPLY_EC(reply, REDIS_REPLY_INTEGER, h.TargetID);
	if (reply->integer != 0) {
		DEBUG_LOG("Blacklist full! plid=%u black=%u", h.TargetID, req.u32());
		return ErrCodeBlacklistFull;
	}
	// 从对方的关注中删除
	keys = { makeFavFollowingKey(req.u32()), makeFollowingKey(req.u32()), makeFollowerKey(req.u32()) };
	args = { to_string(h.TargetID) };
	gRedis->eval_only(kScriptRemoveFriend, &keys, &args); // 不检查是否成功
	// 将我从对方粉丝中删除
	if (fansRank > -1 && !gRedisRank->zrem(makeFansKey(req.u32()), to_string(h.TargetID))) {
		WARN_LOG("zrem failed! err=%s, plid=%u", gRedisRank->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}
	// 将对方从我的粉丝中删除
	if (isFollowed && !gRedisRank->zrem(makeFansKey(h.TargetID), to_string(req.u32()))) {
		WARN_LOG("zrem failed! err=%s, plid=%u", gRedisRank->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}

	REAL_PROTOBUF_MSG(outMsg, db::AddToBlackListRsp, rsp);
	rsp.set_player_id(req.u32());
	rsp.set_is_fans(fansRank > -1 ? true : false);
	rsp.set_is_followed(isFollowed);
	return ErrCodeSucc;
}

ErrCodeType PlayerData::delFromBlacklist(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::Uint32Req, req);
	DEBUG_LOG("Del from blacklist. plid=%u black=%u", h.TargetID, req.u32());

	vector<string> members = { to_string(req.u32()) };
	// if (!gRedis->srem(makeBlacklistKey(h.TargetID), members)) {
	if (!gRedis->hdel(makeBlacklistKey(h.TargetID), members)) {
		DEBUG_LOG("Del from blacklist failed! plid=%u black=%u err=%s",
				  h.TargetID, req.u32(), gRedis->last_error_cstr());
		return ErrCodeDB;
	}

	REAL_PROTOBUF_MSG(outMsg, db::Uint32Rsp, rsp);
	rsp.set_u32(req.u32());
	return ErrCodeSucc;
}

ErrCodeType PlayerData::chkIfInBlacklist(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	ErrCodeType errCode = CheckIfInBlacklist(h.TargetID, h.PlayerID);
	REAL_PROTOBUF_MSG(outMsg, db::BoolRsp, rsp);
	rsp.set_flag(errCode ? true : false);
	return ErrCodeSucc;
}


ErrCodeType PlayerData::updatePlayerActivityData(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::UpdateActivityReq, req);
	//DEBUG_LOG("UpdateActivityReq plid=%u dailydeadtime %u", h.TargetID, req.daily_tm());
	std::string key = kKeyPrefixActivityData + to_string(h.TargetID);
	
	unordered_map<string, string> fields;
	for (int i = 0; i < req.activitys_size(); ++i) {
		const db::Activity& ac = req.activitys(i);
		ac.SerializeToString(&fields[to_string(ac.ac_id())]);
		DEBUG_LOG("updatePlayerActivityData plid=%u acid=%u dtm=%ud", h.TargetID, ac.ac_id(), req.daily_tm());
	}

	fields[kKeyActivtiyDailyDeadTMData] = to_string(req.daily_tm());

	if (!gRedis->hset(key, fields)) {
		WARN_LOG("hset failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}

	return ErrCodeSucc;	
}

ErrCodeType PlayerData::getPlayersAdventureRecord(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, cs::CSRepeatedUint32Req, req);
	REAL_PROTOBUF_MSG(outMsg, cs::PlayersAdventureRecord, rsp);
	for (const auto& pid:req.u32()) {
		cs::PlayersAdventureRecord_Record* record = rsp.add_records();
		record->set_pid(pid);
		doGetPlayerAdventureTowerDatas(h.PlayerID, pid, record->mutable_tower_data());
	}
	return ErrCodeSucc;
}


ErrCodeType	PlayerData::getPlayerTaskGuid(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::Uint32Req, req);
	REAL_PROTOBUF_MSG(outMsg, db::Uint32Rsp, rsp);
	std::string key;
	// 男
	if (req.u32()) {
		key = kGameMGuide;
	} else {		// 女
		key = kGameFGuide;
	}
	vector<string> guids;
	//性别不同所以一定不重复
	if (!gRedis->srandmembers(key, 1, guids)) {
		WARN_LOG("srandmembers failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}

	if(guids.size()) {
		rsp.set_u32(atoi(guids[0].c_str()));
	}

	return ErrCodeSucc;	
}

ErrCodeType	PlayerData::setPlayerRedPointInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, cs::SCRedPointInfo, req);
	REAL_PROTOBUF_MSG(outMsg, cs::SCRedPointInfo, rsp);

	string typeStr = to_string(req.type());
	string key = kKeyPrefixRedPoint + to_string(h.TargetID);
	if (req.flag()) { // 加入红点
		if (!gRedis->sadd(key, { typeStr })) {
			WARN_LOG("setPlayerRedPointInfo sadd failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
			return ErrCodeDB;
		}
	} else {
		if (!gRedis->srem(key, { typeStr })) {
			WARN_LOG("setPlayerRedPointInfo sadd failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
			return ErrCodeDB;
		}
	}

	rsp.set_type(req.type());
	rsp.set_flag(req.flag());
	return ErrCodeSucc;
}


ErrCodeType PlayerData::updatePlayerAccInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::UpdateAcInfoAttrReq, req);
	std::string key = kKeyPrefixPlayerAcInfo + to_string(h.TargetID);

	unordered_map<string, string> fields;
	for (int i = 0; i < req.infos_size(); ++i) {
		const cs::PlayerAcInfo& info = req.infos(i);
		info.SerializeToString(&fields[to_string(info.id())]);
		DEBUG_LOG("updatePlayerAccInfo plid=%u acid=%u vl=%u dtm=%u", h.TargetID, info.id(), info.val(), info.deadtm());
	}

	if (!gRedis->hset(key, fields)) {
		WARN_LOG("hset failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}

	return ErrCodeSucc;
}

ErrCodeType PlayerData::getMoreFans(const SSProtoHead & h, google::protobuf::Message * inMsg, google::protobuf::Message * outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, cs::MoreFriendReq, req);
	REAL_PROTOBUF_MSG(outMsg, db::MultiPlayers, rsp);

	vector<string> result;
	if (!gRedisRank->zrange(makeFansKey(h.PlayerID), req.start(), req.stop(), result)) {
		WARN_LOG("zrange failed: %s! plid=%u", gRedisRank->last_error_cstr(), h.PlayerID);
		return ErrCodeDB;
	}
	// 关注
	unordered_map<string, string> followings;
	if (!gRedis->hgetall(makeFollowingKey(h.PlayerID), followings)) {
		WARN_LOG("hgetall failed: %s! plid=%u", gRedis->last_error_cstr(), h.PlayerID);
		return ErrCodeDB;
	}
	// Momo关注
	unordered_map<string, string> momoFollowings;
	if (!gRedis->hgetall(kKeyPrefixMomoFollowing + to_string(h.PlayerID), momoFollowings)) {
		WARN_LOG("hgetall failed: %s! plid=%u", gRedis->last_error_cstr(), h.PlayerID);
		return ErrCodeDB;
	}
	for (auto plid : result) {
		auto frd = rsp.add_players();
		frd->set_player_id(atoi(plid.c_str()));
		auto followingIt = followings.find(plid);
		if (followingIt != followings.end()) {
			frd->set_friendship(atoi(followingIt->second.c_str()));
		}
		auto momoFollowingIt = momoFollowings.find(plid);
		if (momoFollowingIt != momoFollowings.end()) {
			frd->set_momo_friendship(atoi(momoFollowingIt->second.c_str()));
			// 对方是momo用户的话要设momoid
			string regKey(kKeyPrefixPlayerData + plid);
			unordered_map<string, string> mReg;
			auto& regInfoStr = mReg[kRegInfo];
			if (!gRedis->hget(regKey, mReg)) {
				WARN_LOG("hget failed: %s! plid=%s", gRedis->last_error_message().c_str(), plid.c_str());
				continue;
			}
			db::RegInfo regInfo;
			regInfo.ParseFromString(regInfoStr);
			if (regInfo.account().account_type() == kChannelNameMomo) {
				frd->set_momo_id(regInfo.account().account_id());
			}
		}
	}
	return ErrCodeSucc;
}

ErrCodeType PlayerData::offLine(const SSProtoHead & h, google::protobuf::Message * inMsg, google::protobuf::Message * outMsg)
{
	if (!MysqlProxy::Instance().IsLoad()) {
		return ErrCodeType::ErrCodeSucc;
	}
	std::stringstream ss;
	ss << h.TargetID;
	auto key = MysqlProxy::Instance().GetOfflineKey();
	time_t now = time(0);
	gRedis->zadd(key, now, ss.str());
	return ErrCodeType::ErrCodeSucc;
}

void PlayerData::online(uint32_t playerId)
{
	if (!MysqlProxy::Instance().IsLoad()) {
		return;
	}
	std::stringstream ss;
	ss << playerId;
	auto key = MysqlProxy::Instance().GetOfflineKey();
	gRedis->zrem(key, ss.str());
}


ErrCodeType PlayerData::updateOnlineInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::OnlineInfo, req);
	std::unordered_map<std::string, std::string> fields;
	fields[std::to_string(req.svr_id())] = std::to_string(req.player_cnt());
	gRedis->hset(kGSOnlineInfo, fields);
	return ErrCodeSucc;
}


ErrCodeType PlayerData::checkArFaceId(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::ARFaceIDReq, req);
	REAL_PROTOBUF_MSG(outMsg, db::ARFaceIDRsp, rsp);
	bool isUsed = false;
	if (!gRedis->hexists(makePlayerArFaceID(), req.person_id(), isUsed)) {
		WARN_LOG("checkArFaceId ! target=%u err=%s",
					 h.TargetID, gRedis->last_error_cstr());
		return ErrCodeDB;
	}

	db::ARFaceMon minfo;
	std::unordered_map<std::string, std::string> fields;
	if (isUsed) {
		if (req.plid() != 0) {
			auto& str = fields[req.person_id()];
			if (!gRedis->hget(makePlayerArFaceID(), fields)) {
				WARN_LOG("checkArFaceId hset fail ! target=%u err=%s",
						h.TargetID, gRedis->last_error_cstr());
				return ErrCodeDB;
			}
			minfo.ParseFromString(str);
			minfo.add_info()->set_plid(req.plid());
		}

		//rsp.set_ret(ErrCodeFaceIDDuplicate);
	} else {
		minfo.set_person_id(req.person_id());
		minfo.set_mon_id(req.mon_id());
		minfo.set_mon_model_id(req.mon_model_id());
		minfo.set_etree_id(req.etree_id());
		if(req.plid() != 0) {
			minfo.add_info()->set_plid(req.plid());
		}
	}

	minfo.SerializeToString(&fields[req.person_id()]);
	if (!gRedis->hset(makePlayerArFaceID(), fields)) {
		WARN_LOG("checkArFaceId hset fail ! target=%u err=%s",
				h.TargetID, gRedis->last_error_cstr());
		return ErrCodeDB;
	}

	rsp.set_person_id(req.person_id());
	rsp.set_mon_id(minfo.mon_id());
	rsp.set_ch_id(req.ch_id());
	rsp.set_etree_id(minfo.etree_id());
	rsp.set_mon_model_id(minfo.mon_model_id());
	
	return ErrCodeSucc;
}


ErrCodeType PlayerData::getArFaceInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::StrReq, req);
	REAL_PROTOBUF_MSG(outMsg, db::ARFaceIDCheckRsp, rsp);
	
	bool isUsed = false;
	rsp.set_noexit(true);
	if (!gRedis->hexists(makePlayerArFaceID(), req.str(), isUsed)) {
		WARN_LOG("checkArFaceId ! target=%u err=%s",
					 h.TargetID, gRedis->last_error_cstr());
		return ErrCodeDB;
	}

	if (isUsed) {
		unordered_map<string, string> moninfo;
		auto& minfo = moninfo[req.str()];
		if (!gRedis->hget(makePlayerArFaceID(), moninfo)) {
			WARN_LOG("hget() failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
			return ErrCodeDB;
		}
		rsp.set_noexit(false);
		rsp.mutable_mon_info()->ParseFromString(minfo);
		for (int i = 0; i < rsp.mon_info().info_size(); ++i) {
			uint32_t plid = rsp.mon_info().info(i).plid();
			unordered_map<string, string> m;
			string key(kKeyPrefixPlayerData + to_string(plid));
			const auto& accInfoStr = m[kAccountInfo];
			if (!gRedis->hget(key, m)) {
				DEBUG_LOG("update hget failed, err=%s", gRedis->last_error_message().c_str());
				return ErrCodeDB;
			}
			db::AccountInfo acc;
			acc.ParseFromString(accInfoStr);
			rsp.mutable_mon_info()->mutable_info(i)->set_pname(acc.player_name());


			// 是否开始
			key = kKeyPrefixPlayerAcInfo + to_string(plid);
			unordered_map<string, string> fields;

			if (!gRedis->hexists(key, to_string(cs::kClientStoreAreaBegin), isUsed)) {
				WARN_LOG("hget failed: %s! plid=%u ", gRedis->last_error_message().c_str(), plid);
				return ErrCodeDB;
			} 
			
			if(!isUsed) {
				rsp.mutable_mon_info()->mutable_info(i)->set_show(true);
			}
			else {
				auto& str = fields[to_string(cs::kClientStoreAreaBegin)];
				if (!gRedis->hget(key, fields)) {
					WARN_LOG("hget failed: %s! plid=%u ", gRedis->last_error_message().c_str(), plid);
					return ErrCodeDB;
				}

				cs::PlayerAcInfo acinfo;
				acinfo.ParseFromString(str);
				bool val = (acinfo.val() & 0x4);
				rsp.mutable_mon_info()->mutable_info(i)->set_show(!val);

			}
		
		}

		/*
		string key(kKeyPrefixPlayerData + plid);
		unordered_map<string, string> fields;
		auto& faceinfo = fields[kPlayerArFace];
		if (!gRedis->hget(key, fields)) {
			WARN_LOG("hget() failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
			return ErrCodeDB;
		}
		rsp.set_noexit(true);
		rsp.mutable_mon_info()->ParseFromString(faceinfo);
		*/
	}

	return ErrCodeSucc;
}


ErrCodeType PlayerData::checkArFaceDailyTask(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::ARFaceIDReq, req);
	REAL_PROTOBUF_MSG(outMsg, db::Uint32Rsp, rsp);
	uint32_t ret = 0;
	uint32_t now = time(nullptr);
	ret = checkDoArMonDailyTask(req.person_id(), req.plid());
	string plid = to_string(req.plid());

	/*
	string key(kKeyPrefixPlayerData + plid);
	unordered_map<string, string> m;
	auto& task = m[kDailyTask];
	if (!gRedis->hget(key, m)) {
		WARN_LOG("hgetall failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}
	cs::DailyTask dailytask;
	dailytask.ParseFromString(task);
	DEBUG_LOG("checkArFaceDailyTask %s", dailytask.Utf8DebugString().c_str());
	uint32_t now = time(nullptr);*/
	/*
	if (now >= dailytask.refreshtime()) {
		ret = 0;
	} else {
		for (int i = 0; i < dailytask.infos_size(); ++i) {
			auto& t = dailytask.infos(i);
			auto* dt = gCSVDailyTasksBasic.GetItem(t.task_id());
			if (dt && (dt->TaskType == 14 || dt->TaskType == 18) ) {
				DEBUG_LOG("checkDoArMonDailyTask %p %u", dt, dt->TaskType);
				auto& itemStr = dt->Steps[0];	
				auto itemAndCnt = Split(itemStr, '#');
				if (itemAndCnt.size() != 3) {
					continue;
				}
				uint32_t type = atoi(itemAndCnt[0].c_str());
				uint32_t arg = atoi(itemAndCnt[1].c_str());
				DEBUG_LOG("checkDoArMonDailyTask %u %u ", type, req.mon_id());
				//uint32_t val = atoi(itemAndCnt[2].c_str());
				if (type == 119) {			// 扫到某个状态
					auto item = gCSVMonster.GetItem(req.mon_id());
					if (item) {
						if (item->Stage >= int(arg)) {
						}
						DEBUG_LOG("checkDoArMonDailyTask %u %u %u %u", type, req.mon_id(), item->Stage, ret);
					}
				} else if (type == 118) {	// 扫到某只
					DEBUG_LOG("checkDoArMonDailyTask %u %u", type, arg);
					if (!arg || req.mon_id() == arg) {
						ret = checkDoArMonDailyTask(req.mon_id(), req.plid());
					}
				}

			}
		}
	}*/


	// 简单记录
	//================================
	auto recordkey = makePlayerFaceARKey(plid);
	long long val;
	if (!gRedis->hincrby(recordkey, req.person_id(), 1, &val)) {
		WARN_LOG("hincrby failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}


	unordered_map<string, string> fields;
	fields[makePlayerFaceARPersonTmKey(req.person_id())] = to_string(now);
	if (!gRedis->hset(recordkey, fields)) {
		WARN_LOG("hset failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}


	//================================

	rsp.set_u32(ret);

	return ErrCodeSucc;	
}


bool PlayerData::checkDoArMonDailyTask(const std::string& monid, uint32_t plid)
{
	const auto& script = ScriptMgr::Instance().GetScriptStr("script/arface_mon_dailytask.lua", false);
	std::vector<std::string> keys{ kKeyArMonDailyTask };
	time_t now = time(NULL);
	uint32_t timeSpan = NextDay() + 5 * 3600 - now;
	std::vector<std::string> args { monid, to_string(timeSpan) , "5"};

	auto reply = gRedis->eval(script, &keys, &args);
	switch (reply->type)
	{
		case REDIS_REPLY_INTEGER:
			return reply->integer == 1; 
			break;
		case REDIS_REPLY_ERROR:
			WARN_LOG("Redis error: %s! plid=%u", reply->str, plid);
			break;
		default:
			WARN_LOG("Redis error: unexpected reply type %d! plid=%u", reply->type, plid);
			break;
	}

	return false;	
}

bool PlayerData::checkPlayerIsBan(uint32_t pid)
{
	string strPid = to_string(pid);
	unordered_map<string, string> ban;
	auto& banChat = ban[kBEDataFieldBanChat];
	if (!gRedis->hgetall(makeBasicExtraDataKey(strPid), ban)) {
		WARN_LOG("hgetall failed: %s! plid=%u", gRedis->last_error_cstr(), pid);
		return false;
	}
	unordered_map<string, string> m;
	auto& regInfoStr = m[kRegInfo];
	if (!gRedis->hget(kKeyPrefixPlayerData + strPid, m)) {
		WARN_LOG("hget failed: %s! plid=%u", gRedis->last_error_message().c_str(), pid);
		return false;
	}

	db::RegInfo regInfo;
	regInfo.ParseFromString(regInfoStr);
	m.clear();
	auto& banDeviceChatTimeStr = m[regInfo.device().did()];
	if (!gRedis->hget(kMiscFieldBanChatDevice, m)) {
		WARN_LOG("hget failed: %s! plid=%u", gRedis->last_error_message().c_str(), pid);
		return false;
	}
	auto banDeviceChatTime = atoi(banDeviceChatTimeStr.c_str());
	m.clear();
	auto& banIPChatTimeStr = m[TransIP(regInfo.device().ip())];
	if (!gRedis->hget(kMiscFieldBanChatIP, m)) {
		WARN_LOG("hget failed: %s! plid=%u", gRedis->last_error_message().c_str(), pid);
		return false;
	}
	auto banIPChatTime = atoi(banIPChatTimeStr.c_str());
	auto banChatTime = atoi(banChat.c_str());
	if (banDeviceChatTime > banChatTime) {
		banChatTime = banDeviceChatTime;
	}
	if (banIPChatTime > banChatTime) {
		banChatTime = banIPChatTime;
	}
	
	return time(nullptr) < banChatTime;
}

ErrCodeType PlayerData::setARFaceIncrBy(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::ARFaceIDReq, req);

	gRedis->hincrby(kKeyArMonDailyTask, req.person_id(), 1);
	
	return ErrCodeSucc;	
}

ErrCodeType PlayerData::setARFaceShowInfofbd(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	/*
	REAL_PROTOBUF_MSG(inMsg, db::ARFaceShowInfofbdReq, req);
	uint32_t plid = h.TargetID;

	unordered_map<string, string> moninfo;
	auto& minfo = moninfo[req.persion_id()];
	if (!gRedis->hget(makePlayerArFaceID(), moninfo)) {
		WARN_LOG("hget() failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}

	db::ARFaceMon info;
	info.ParseFromString(minfo);
	*/

	return ErrCodeSucc;	
}

ErrCodeType PlayerData::updateGarment(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::GarmentData, req);

	std::string logAddIdStr;
	std::string logDelIdStr;
	std::string logDelPic;
	std::string key = makeGarmentKey(to_string(h.TargetID));
	unordered_map<string, string> mapUpdate;
	vector<string> vecDelete;
	mapUpdate[kKeyGar_Uid] = to_string(req.uid());
	mapUpdate[kKeyGar_SuitUse] = to_string(req.data().suit_use());
	for (int i = 0; i < req.data().garments_size(); i++) {
		std::string uidStr = to_string(req.data().garments(i).gar_uid());
		auto fieldKey = kKeyGar_Garments + uidStr;
		if (req.data().garments(i).expire_time() >= -2) { // -3 出售 -2 过期 -1永久
			auto& tmp = mapUpdate[fieldKey];
			req.data().garments(i).SerializeToString(&tmp);
			logAddIdStr += uidStr + ' ';
		}
		else {
			vecDelete.push_back(fieldKey);
			logDelIdStr += uidStr + ' ';
		}
	}
	for (int i = 0; i < req.data().suits_size(); i++) {
		auto& tmp = mapUpdate[kKeyGar_GarSuits + to_string(req.data().suits(i).suit_id())];
		req.data().suits(i).SerializeToString(&tmp);
	}
	for (int i = 0; i < req.data().pics_size(); i++) {
		std::string fieldKey = kKeyGar_GarPics + to_string(req.data().pics(i).pic_id());
		if (req.data().pics(i).expire_time() >= -2) { // -3 出售 -2 过期 -1永久
			auto& tmp = mapUpdate[fieldKey];
			req.data().pics(i).SerializeToString(&tmp);
		}
		else {
			vecDelete.push_back(fieldKey);
			logDelPic += to_string(req.data().pics(i).pic_id()) + ' ';
		}
	}

	DEBUG_LOG("UpdateGarmentReq plid=%u set=[%s] del=[%s] del_pic=[%s]", h.TargetID, logAddIdStr.c_str(), logDelIdStr.c_str(), logDelPic.c_str());

	if (!mapUpdate.empty()) {
		if (!gRedis->hset(key, mapUpdate)) {
			WARN_LOG("hset failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
			return ErrCodeDB;
		}
	}
	
	if (!vecDelete.empty()) {
		if (!gRedis->hdel(key, vecDelete)) {
			WARN_LOG("hdel failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
			return ErrCodeDB;
		}
	}

	return ErrCodeSucc;
}

ErrCodeType PlayerData::checkBetaSubscribe(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::BetaAssistCheckReq, req);
	REAL_PROTOBUF_MSG(outMsg, db::BetaAssistCheckRsp, rsp);

	rsp.set_phone(req.phone());
	std::unordered_map<std::string, std::string> tmp;
	
	std::string key = makeBetaPhoneKey();
	const string& val = tmp[req.phone()];
	if (!gRedis->hget(key, tmp)) {
		WARN_LOG("checkBetaSubscribe hget call failed, phone=%s, plid=%u", req.phone().c_str(), h.TargetID);
		return ErrCodeDB;
	}
	if (!val.empty()) {
		rsp.set_exist(true);
		rsp.set_time(atoi(val.c_str()));
	}

	tmp.clear();
	
	std::string officalKey = makeBetaPhoneOfficialKey();
	const string& officalVal = tmp[req.phone()];
	if (!gRedis->hget(officalKey, tmp)) {
		WARN_LOG("checkBetaSubscribe hget call failed, phone=%s, plid=%u", req.phone().c_str(), h.TargetID);
		return ErrCodeDB;
	}
	if (!officalVal.empty()) {
		rsp.set_official(1);
	}

	//DEBUG_LOG("checkBetaSubscribe, pid=%u", h.TargetID);
	return ErrCodeSucc;
}

ErrCodeType PlayerData::onBetaPhoneSave(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, cs::BetaPhone, req);
	DEBUG_LOG("onBetaPhoneSave plid=%u, phone=%s", h.TargetID, req.phone().c_str());
	
	std::string key = makeBetaPhoneKey();
	std::unordered_map<std::string, std::string> tmp;
	tmp[req.phone()] = to_string(req.timestamp());
	if (!gRedis->hset(key, tmp)) {
		WARN_LOG("set failed: %s! plid=%u, betaPhone=%s", gRedis->last_error_cstr(), h.TargetID, req.phone().c_str());
		return ErrCodeDB;
	}

	return ErrCodeSucc;
}

ErrCodeType PlayerData::updatePlayerBeta(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, cs::BetaAssist, req);
	DEBUG_LOG("updatePlayerBeta plid=%u, betaAssist=%s", h.TargetID, req.Utf8DebugString().c_str());

	std::string key = makePlayerBetaKey(to_string(h.TargetID));
	std::string val;
	req.SerializeToString(&val);
	if (!gRedis->set(key, val)) {
		WARN_LOG("updatePlayerBeta set failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}

	return ErrCodeSucc;
}

ErrCodeType PlayerData::onUpdatePlayerGarWearing(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, cs::GarmentWearing, req);
	DEBUG_LOG("onUpdatePlayerGarWearing plid=%u", h.TargetID);

	string key(kKeyPrefixPlayerData + to_string(h.TargetID));
	std::unordered_map<std::string, std::string> m;
	auto& str = m[kGarWearing];
	req.SerializeToString(&str);
	if (!gRedis->hset(key, m)) {
		WARN_LOG("onUpdatePlayerGarWearing hset failed: %s,  plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}

	return ErrCodeSucc;
}

ErrCodeType PlayerData::onUpdateAcTaskInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	return ErrCodeSucc;
}

ErrCodeType PlayerData::onUpdateHutInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, cs::HutInfo, req);

	string key = makePlayerHutKey(to_string(h.TargetID));
	string val;
	req.SerializeToString(&val);
	if (!gRedis->set(key, val)) {
		WARN_LOG("onUpdateHutInfo hset failed: %s, plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}

	return ErrCodeSucc;
}

ErrCodeType PlayerData::onUpdatePlayerCollectCacheInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::PlayerCollectCacheInfo, req);

	string key = makePlayerCacheKey(h.TargetID);

	unordered_map<string, string> m;
	auto& collectCnt = m[kCacheFieldCollectionCnt];
	auto& collectPoint = m[kCacheFieldCollectionPoint];
	collectCnt = to_string(req.collect_cnt());
	collectPoint = to_string(req.collect_point());
	if (!gRedis->hset(key, m)) {
		WARN_LOG("onUpdatePlayerCollectCacheInfo hset failed: %s, plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}

	return ErrCodeSucc;
}

ErrCodeType PlayerData::getPlayerMonsterInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::Uint32Req, req);
	REAL_PROTOBUF_MSG(outMsg, db::MonsterInfo, rsp);

	string key(kKeyPrefixPlayerData + to_string(h.TargetID));
	unordered_map<string, string> m;
	string mFiled = "_M" + to_string(req.u32());
	auto& mInfo = m[mFiled];
	if (!gRedis->hget(key, m)) {
		WARN_LOG("hget failed: %s! plid=%u target=%u", gRedis->last_error_message().c_str(), h.PlayerID, h.TargetID);
		return ErrCodeMonsterMayBeExpired;
	}
	if (mInfo.size() <= 0 || !rsp.ParseFromString(mInfo)) {
		return ErrCodeMonsterMayBeExpired;
	}

	return ErrCodeSucc;
}

ErrCodeType PlayerData::getActGlobalInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(outMsg, cs::ActivityGlobalInfo, rsp);
	string value;
	if (!gRedis->get(kKeyActivityGlobalData, value)) {
		WARN_LOG("get failed: %s!", gRedis->last_error_message().c_str());
		return ErrCodeDB;
	}
	rsp.ParseFromString(value);
	return ErrCodeSucc;
}

ErrCodeType PlayerData::updateActGlobalInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, cs::ActivityGlobalInfo, req);
	string value;
	req.SerializeToString(&value);
	if (!gRedis->set(kKeyActivityGlobalData, value)) {
		WARN_LOG("set failed: %s!", gRedis->last_error_message().c_str());
		return ErrCodeDB;
	}
	return ErrCodeSucc;
}

ErrCodeType PlayerData::giftGivenChannel(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::StrReq, req);
	REAL_PROTOBUF_MSG(outMsg, db::Uint32Rsp, rsp);
	// "MEIZU"."#".$_REQUEST["uid"]."#".$_REQUEST["zoneId"]."#".$_REQUEST["roleId"]."#".$_REQUEST["giftId"];
	auto info = Split(req.str(), '#');
	if (info.size() != 5) {
		ERROR_LOG("invalid gift channel key! %s", req.str().c_str());
		return ErrCodeDB;
	}

	uint32_t pid = atoi(info[1].c_str());
	string plidKey(kKeyPrefixPlayerData + to_string(pid)); 
	bool exist = false;
	if (!gRedis->exists(plidKey, exist)) {
		WARN_LOG("exists failed! plid=%u err=%s", h.PlayerID, gRedis->last_error_cstr());             
		return ErrCodeDB;
	}   

	DEBUG_LOG("giftGivenChannel %s %u", req.str().c_str(), pid);                                      
	if (!exist) {
		rsp.set_u32(1);
		return ErrCodeSucc;
	}


	long long first = 0;
	if (!gRedis->hincrby(kKeyChannelGiftUnKey, req.str(), 1, &first)) {
		WARN_LOG("hincrby failed: %s! key=%s", gRedis->last_error_cstr(), req.str().c_str());
		return ErrCodeDB;

	}

	if (first > 1) {
		// double
		rsp.set_u32(1);
		return ErrCodeSucc;
	}


	DEBUG_LOG("giftGivenChannel %s %s", info[0].c_str(), info[4].c_str());

	unordered_map<string, string> m;
	auto& gift = m[info[4]];
	if (!gRedis->hexists(kKeyChannelGiftInfo, info[4], exist)) {
		WARN_LOG("hexists failed: %s! key=%s", gRedis->last_error_cstr(), req.str().c_str());
		return ErrCodeDB;
	}
	if (exist) {
		if (!gRedis->hget(kKeyChannelGiftInfo, m)) {
			WARN_LOG("hincrby failed: %s! key=%s", gRedis->last_error_cstr(), req.str().c_str());
			return ErrCodeDB;
		}
		db::AddMailReq tempreq;
		tempreq.set_player_id(pid);
		auto* mail = tempreq.add_mails();
		mail->ParseFromString(gift);
		mail->set_send_time(time(0));
		if (gMailData.AddMail(tempreq) == ErrCodeSucc) {
			rsp.set_u32(0);
		} else {
			rsp.set_u32(3);
		}
		return ErrCodeSucc;
	}

	rsp.set_u32(2);
	return ErrCodeSucc;	
}


ErrCodeType PlayerData::getPlayerExtraInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(outMsg, db::PlayerExtraInfo, rsp);

	string key(kKeyPrefixPlayerData + to_string(h.TargetID));
	unordered_map<string, string> m;
	auto& accInfoStr = m[kAccountInfo];
	auto& playerAttrStr = m[kPlayerAttr];
	if (!gRedis->hget(key, m)) {
		WARN_LOG("hget failed: %s! plid=%u", gRedis->last_error_message().c_str(), h.TargetID);
		return ErrCodeDB;
	}

	db::AccountInfo accInfo;
	accInfo.ParseFromString(accInfoStr);

	rsp.mutable_player_name()->swap(*accInfo.mutable_player_name());
	cs::PlayerAttr playerAttr;
	playerAttr.ParseFromString(playerAttrStr);

	rsp.set_lv(playerAttr.lv());

	return ErrCodeSucc;	
}

ErrCodeType PlayerData::setOperatorVer(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::Uint32Req, req);

	string strPlayerID = to_string(h.TargetID);

	unordered_map<string, string> opInfo;
	const string& opVer = opInfo[kMiscFieldOperationVer];
	if (!gRedis->hget(kKeyPrefixSocialData + strPlayerID, opInfo)) {
		WARN_LOG("hget failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}

	if (atol(opVer.c_str()) > req.u32()) {
		return ErrCodeSucc;
	}

	opInfo[kMiscFieldOperationVer] = to_string(req.u32());
	if (!gRedis->hset(kKeyPrefixSocialData + strPlayerID, opInfo)) {
		WARN_LOG("hset failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}

	DEBUG_LOG("setOperatorVer plid=%u ver=%u", h.TargetID, req.u32());
	return ErrCodeSucc;
}

ErrCodeType PlayerData::onChangePlayerData(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, GmMsg::GmChangePlayerDataReq, req);
	DEBUG_LOG("onChangePlayerData, pid:%u, type:%s, data:%s", req.pid(), req.type().c_str(), req.data().c_str());

	if (req.type() == "sign") { // 修改签名
		string strPid = to_string(req.pid());
		string key(kKeyPrefixPlayerData + strPid);
		unordered_map<string, string> m;
		string& account_ = m[kAccountInfo];
		if (!gRedis->hget(key, m)) {
			DEBUG_LOG("onChangePlayerData, hget failed: %s, pid:%u", gRedis->last_error_cstr(), req.pid());
			return ErrCodeDB;
		}
		db::AccountInfo accInfo;
		accInfo.ParseFromString(account_);
		accInfo.set_slogan(req.data());
		account_.clear();
		accInfo.SerializeToString(&account_);
		if (!gRedis->hset(key, m)) {
			DEBUG_LOG("onChangePlayerData, hset failed: %s, pid:%u", gRedis->last_error_cstr(), req.pid());
			return ErrCodeDB;
		}
	} else if (req.type() == "wealth") { // 增加财富值
		uint32_t add_ = atoi(req.data().c_str());
		string key = kKeyPrefixSocialData + to_string(req.pid());
		long long totalWealth, weekWealth, dailyWealth;
		// 增加游戏总财富值
		if (!gRedis->hincrby(key, kSocialFieldWealth, add_, &totalWealth)) {
			WARN_LOG("hincrby failed: %s! plid=%u", gRedis->last_error_cstr(), req.pid());
			return ErrCodeDB;
		}
		if (!gRedis->hincrby(key, kSocialFieldWeekWealth, add_, &weekWealth)) {
			WARN_LOG("hincrby failed: %s! plid=%u", gRedis->last_error_cstr(), req.pid());
			return ErrCodeDB;
		}
		if (!gRedis->hincrby(key, kSocialFieldDailyWealth, add_, &dailyWealth)) {
			WARN_LOG("hincrby failed: %s! plid=%u", gRedis->last_error_cstr(), req.pid());
			return ErrCodeDB;
		}
	} else if (req.type() == "role") { // 换绑momosdk的游客账号数据到momo账号上
		string key(kKeyPrefixPlayerData + to_string(req.pid()));
		unordered_map<string, string> m;
		auto& regInfoStr = m[kRegInfo];
		if (!gRedis->hget(key, m)) {
			DEBUG_LOG("onChangePlayerData hget failed: %s, pid=%u", gRedis->last_error_cstr(), req.pid());
			return ErrCodeDB;
		}
		db::RegInfo regInfo;
		regInfo.ParseFromString(regInfoStr);
		if (regInfo.account().account_type() != "momo") { // 不是momosdk账号
			DEBUG_LOG("onChangePlayerData, pid:%u, account not momosdk:%s", req.pid(), regInfo.account().account_type().c_str());
			return ErrCodeDB;
		}
		string extraKey(kKeyPrefixExtrPhotos + to_string(req.pid()));
		string extraVal;
		if (!gRedis->get(extraKey, extraVal)) {
			DEBUG_LOG("onChangePlayerData get failed: %s, pid=%u", gRedis->last_error_cstr(), req.pid());
			return ErrCodeDB;
		}
		cs::ExtraInfo extra_;
		extra_.ParseFromString(extraVal);
		if (atoi(extra_.from().c_str()) != 3) { // 不是游客账号
			DEBUG_LOG("onChangePlayerData, from:%s != 3, pid=%u", extra_.from().c_str(), req.pid());
			return ErrCodeDB;
		}
		long long cnt = 0;
		string orgAccount = kKeyPrefixLogin + "momo" + kLoginSplit + regInfo.account().account_id();
		if (!gRedis->del({orgAccount}, &cnt)) { // 将游客账号与pid解绑
			DEBUG_LOG("onChangePlayerData del failed: %s, pid=%u", gRedis->last_error_cstr(), req.pid());
			return ErrCodeDB;
		}
		regInfo.mutable_account()->set_account_id(req.data());
		regInfoStr.clear();
		regInfo.SerializeToString(&regInfoStr); // 将regInfo中的accountid设置为新的momoid
		if (!gRedis->hset(key, m)) {
			DEBUG_LOG("onChangePlayerData hset failed: %s, pid=%u", gRedis->last_error_cstr(), req.pid());
			return ErrCodeDB;
		}
		string curAccount = kKeyPrefixLogin + "momo" + kLoginSplit + req.data();
		if (!gRedis->set(curAccount, to_string(req.pid()))) { // 将pid换绑到给定的momoid账号上
			DEBUG_LOG("onChangePlayerData, set failed: %s, pid:%u", gRedis->last_error_cstr(), req.pid());
			return ErrCodeDB;
		}
	} else {
		DEBUG_LOG("onChangePlayerData, pid:%u, type:%s, data:%s, invalid type!", req.pid(), req.type().c_str(), req.data().c_str());
	}
	
	return ErrCodeSucc;
}

ErrCodeType PlayerData::onGetLoginRecords(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, cs::GmLoginRecordReq, req);
	REAL_PROTOBUF_MSG(outMsg, cs::GmLoginRecordRsp, rsp);
	DEBUG_LOG("onGetLoginRecords, pid:%u, index:%d, limit:%d", req.pid(), req.index(), req.limit());
	TblLoginRecord mysqlLoginRecord(req.pid(), "digimon", "login_record");
	uint32_t total = mysqlLoginRecord.GetLoginRecords(req.index(), req.limit(), [&rsp](std::string ip, uint32_t time_stamp) {
		auto p = rsp.add_records();
		p->set_ip(ip);
		p->set_time(time_stamp);
	});
	rsp.set_total(total);
	return ErrCodeSucc;
}

ErrCodeType PlayerData::onGetBindInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	DEBUG_LOG("onGetBindInfo, pid:%u", h.PlayerID);
	REAL_PROTOBUF_MSG(outMsg, cs::GmPlayerBindInfo, rsp);
	db::AllPlayerData data;
	rsp.set_pid(h.PlayerID);
	rsp.set_momo_sdk(1);
	rsp.set_guest_login(1);
	getGMPlayerData(h, nullptr, &data);
	if (data.player_data().reg_info().account().account_type() != "momo") { // 不是momosdk账号
		rsp.set_momo_sdk(0);
	}
	if (atoi(data.extra_data().extra_info().from().c_str()) != 3) { // 不是游客账号
		rsp.set_guest_login(0);
	} else {
		rsp.set_guest_id(data.player_data().reg_info().account().account_id());
	}
	uint32_t payTime = 0;
	uint32_t totalPay = 0;
	time_t tempTime = 0;
	for (auto order : data.pay_order().finished_orders()) {
		tempTime = atoi(order.create_time().c_str());
		if (payTime == 0 || payTime < tempTime) {
			payTime = tempTime;
		}
		totalPay += order.total_fee();
	}
	rsp.set_pay_time(payTime);
	rsp.set_total_pay(totalPay);
	rsp.set_login_ip(TransIP(data.player_data().online_stat_info().last_device().ip()));
	rsp.set_create_time(data.player_data().reg_info().reg_time());
	return ErrCodeSucc;
}

ErrCodeType PlayerData::onAddLotteryRecord(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, cs::GmLotteryRecord, req);
	DEBUG_LOG("onAddLotteryRecord, pid:%u, lottery_type:%d, consume_type:%d, bef:%d, aft:%d", 
		h.TargetID, req.type(), req.consume(), req.bef(), req.aft());
	TblLotteryRecord mysqlLotteryRecord(h.TargetID, "digimon", "lottery_record");
	string data;
	req.SerializeToString(&data);
	mysqlLotteryRecord.AddLotteryRecord(req.type(), data);
	return ErrCodeSucc;
}

ErrCodeType PlayerData::onGetLotteryRecords(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, cs::GmLotteryRecordReq, req);
	REAL_PROTOBUF_MSG(outMsg, cs::GmLotteryRecordRsp, rsp);
	DEBUG_LOG("onGetLotteryRecords, pid:%u, time_beg:%d, time_end:%d, type:%d, index:%d, limit:%d",
		req.pid(), req.time_start(), req.time_end(), req.type(), req.index(), req.limit());

	TblLotteryRecord mysqlLotteryRecord(req.pid(), "digimon", "lottery_record");
	uint32_t total = mysqlLotteryRecord.GetLotteryRecords(req.type(), req.index(), req.limit(), req.time_start(), req.time_end(), [&rsp](std::string& data) {
		if (!data.empty()) {
			auto p = rsp.add_records();
			p->ParseFromString(data);
		}
	});
	rsp.set_total(total);
	return ErrCodeSucc;
}

ErrCodeType PlayerData::onAddSpamRecord(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, cs::GmSpamRecord, req);
	DEBUG_LOG("onAddSpamRecord, pid:%u, ban_time:%d, ban_type:%d, result:%d", req.pid(), req.ban_time(), req.ban_type(), req.result());
	TblSpam mysqlSpam(0, "global", "SpamRecord");
	mysqlSpam.AddSpamRecord(req.name(), req.pid(), req.channel(), req.ban_time(), req.content(), req.result(), req.ban_type());
	return ErrCodeSucc;
}

ErrCodeType PlayerData::onGetSpamRecords(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, cs::GmSpamRecordReq, req);
	REAL_PROTOBUF_MSG(outMsg, cs::GmSpamRecordRsp, rsp);
	TblSpam mysqlSpam(0, "global", "SpamRecord");
	uint32_t total_ = mysqlSpam.GetSpamRecords(req.index(), req.limit(), [&rsp](uint32_t id, uint32_t read, uint32_t state, std::string name, 
		uint32_t pid, std::string channel, uint32_t ban_time, std::string content, uint32_t result, uint32_t ban_type) {
		auto p = rsp.add_records();
		p->set_id(id);
		p->set_read(read);
		p->set_state(state);
		p->set_name(name);
		p->set_pid(pid);
		p->set_channel(channel);
		p->set_ban_time(ban_time);
		p->set_content(content);
		p->set_result(result);
		p->set_ban_type(ban_type);
	});
	rsp.set_total(total_);
	return ErrCodeSucc;
}

ErrCodeType PlayerData::onUpdateSpamState(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, cs::GmSpamUpdateReq, req);
	REAL_PROTOBUF_MSG(outMsg, cs::GmSpamRecordRsp, rsp);
	DEBUG_LOG("onUpdateSpamState, id:%d, state:%d, rsp:%d", req.id(), req.state(), rsp.total());
	TblSpam mysqlSpam(0, "global", "SpamRecord");
	mysqlSpam.UpdateSpamRecord(req.id(), req.state());
	return ErrCodeSucc;
}

//===============================================================================
// Private static methods
//===============================================================================

ErrCodeType PlayerData::doSetPlayerData(const SSProtoHead& h, const std::string& key, db::PlayerData& data, bool isCreatePlayer)
{
	vector<const google::protobuf::FieldDescriptor*> fields;
	auto reflection = data.GetReflection();
	reflection->ListFields(data, &fields);
	if (fields.empty()) {
		WARN_LOG("No PlayerData! plid=%u target=%u proto=%u origProto=%u",
					h.PlayerID, h.TargetID, h.ProtoID, h.OrigProtoID);
		return ErrCodeSucc;
	}

	db::AccountInfo* newAccInfo = nullptr;
	unordered_map<string, string> m;
	vector<string> delFromMonBag;
	for (auto field : fields) {
		auto msg = reflection->MutableMessage(&data, field);
		if (field->name() == kMonsterBag) { // 特殊处理宠物背包
			db::MonsterBag* bag = static_cast<db::MonsterBag*>(msg);
			for (int i = 0; i < bag->monster_eggs_size(); ++i) {
				const auto& egg = bag->monster_eggs(i);
				string hashKey(kEggHashKeyPrefix + to_string(egg.guid()));
				if (!egg.SerializeToString(&(m[hashKey]))) {
					WARN_LOG("Failed to serialize %s! guid=%u", egg.GetTypeName().c_str(), egg.guid());
					m.erase(hashKey);
				}
			}
			for (int i = 0; i < bag->monsters_size(); ++i) {
				const auto& mon = bag->monsters(i);
				string hashKey(kMonHashKeyPrefix + to_string(mon.guid()));
				if (!mon.SerializeToString(&(m[hashKey]))) {
					WARN_LOG("Failed to serialize %s! guid=%u", mon.GetTypeName().c_str(), mon.guid());
					m.erase(hashKey);
				}
			}
			for (int i = 0; i < bag->del_eggs_size(); ++i) {
				delFromMonBag.emplace_back(kEggHashKeyPrefix + to_string(bag->del_eggs(i)));
			}
			for (int i = 0; i < bag->del_mons_size(); ++i) {
				delFromMonBag.emplace_back(kMonHashKeyPrefix + to_string(bag->del_mons(i)));
			}

			bag->clear_monster_eggs();
			bag->clear_monsters();
			bag->clear_del_eggs();
			bag->clear_del_mons();
		}

		if (msg->SerializeToString(&(m[field->name()]))) {
			if (field->name() == kAccountInfo) {
				if (!isCreatePlayer && !newAccInfo) {
					newAccInfo = static_cast<db::AccountInfo*>(msg);
				}
			}
			else if (field->name() == kRegInfo) {
				if (isCreatePlayer) {
					auto regInfo = static_cast<db::RegInfo*>(msg);
					if (regInfo) {
						Arena::addArenaTop30Award(regInfo->account(), h.PlayerID);
					}
				}
			}
		} else {
			WARN_LOG("Failed to serialize %s!", field->name().c_str());
			m.erase(field->name());
		}
	}

	string oldNick;
	while (newAccInfo) {
		// 这里发生错误，顶多是老昵称没有被正确释放，故就算错误也继续保存玩家数据
		unordered_map<string, string> accountInfo;
		const string& info = accountInfo[kAccountInfo];
		if (!gRedis->hget(key, accountInfo)) {
			WARN_LOG("hget failed: %s! plid=%u target=%u proto=%u origProto=%u",
						gRedis->last_error_message().c_str(), h.PlayerID, h.TargetID,
						h.ProtoID, h.OrigProtoID);
			break;
		}

		db::AccountInfo oldAccInfo;
		if (info.size() && oldAccInfo.ParseFromString(info)
				&& oldAccInfo.player_name() != newAccInfo->player_name()) {
			oldNick.swap(*oldAccInfo.mutable_player_name());
		}

		break;
	}

	if (gRedis->hset(key, m)) {
		std::string logStr;
		for (auto& kv : m) {
			logStr += " " + kv.first + ":" + to_string(kv.second.size());
		}
		DEBUG_LOG("SetPlayerData plid=%u %d info=[%s]", h.PlayerID, isCreatePlayer, logStr.c_str());

		if (delFromMonBag.size() && !gRedis->hdel(key, delFromMonBag)) { // 能成功最好，不能成功则玩家得益
			WARN_LOG("hdel failed: %s! plid=%u target=%u proto=%u origProto=%u",
						gRedis->last_error_message().c_str(), h.PlayerID, h.TargetID,
						h.ProtoID, h.OrigProtoID);;
		}

		if (oldNick.size()) {
			/*std::vector<std::string> keysToDel;
			keysToDel.emplace_back(kKeyPrefixNickname + oldNick);
			if (!gRedisNick->del(keysToDel)) {
				DEBUG_LOG("Failed to del %s@%u! del: %s!", keysToDel[0].c_str(), h.TargetID,
							gRedis->last_error_message().c_str());
			}*/
			TblPlayerNick tblNick(h.TargetID);
			tblNick.DeleteNick(oldNick);
		}

		return ErrCodeSucc;
	}

	WARN_LOG("hset failed: %s! plid=%u target=%u proto=%u origProto=%u data=%s",
				gRedis->last_error_message().c_str(), h.PlayerID, h.TargetID,
				h.ProtoID, h.OrigProtoID, data.Utf8DebugString().c_str());
	return ErrCodeDB;
}

void PlayerData::SetPlayerLoginedToday(std::string& strPlayerID, bool newbieFinished, int gameRegion)
{
	// 如果过了新手引导，把玩家加到当日登录列表
	//if (rsp.newbie_info().complete_time()) {
	if (newbieFinished) {
		string todayPlayersKey = makeTodayLoginedPlayersKey(gameRegion);
		gRedis->sadd(todayPlayersKey, { strPlayerID });
		// 设置当日登录列表的过期时间
		long long int ttl = -1;
		gRedis->ttl(todayPlayersKey, ttl);
		if (ttl == -1) {
			gRedis->expire(todayPlayersKey, 86700); // 多留300秒
		}
	}

	string allTodayPlayersKey = makeAllTodayLoginedPlayersKey(gameRegion);
	DEBUG_LOG("SetPlayerLoginedToday key = %s", allTodayPlayersKey.c_str());
	gRedis->sadd(allTodayPlayersKey, { strPlayerID });
	// 设置当日登录列表的过期时间
	long long int ttl = -1;
	gRedis->ttl(allTodayPlayersKey, ttl);
	if (ttl == -1) {
		gRedis->expire(allTodayPlayersKey, 86400 * 5); // 多留5天
	}
}

bool PlayerData::toPlayerData(std::unordered_map<std::string, std::string>& m, db::PlayerData& data)
{
	auto it = m.find(kMonsterBag);
	if (it == m.end()) {
		WARN_LOG("Can't find %s!", kMonsterBag.c_str());
		return false;
	}

	auto monBag = data.mutable_monster_bag();
	if (!monBag->ParseFromString(it->second)) {
		WARN_LOG("Failed to parse %s!", kMonsterBag.c_str());
		return false;
	}
	m.erase(it);

	auto reflection = data.GetReflection();
	auto descriptor = data.GetDescriptor();
	for (const auto& v : m) {
		if (v.first.size() > 2 && v.first[0] == '_') { // 宠物蛋和宠物数据
			switch (v.first[1]) {
			case 'M': // 宠物
				if (!monBag->add_monsters()->ParseFromString(v.second)) {
					WARN_LOG("Failed to parse a monster!");
					monBag->mutable_monsters()->RemoveLast();
				}
				break;
			case 'E': // 宠物蛋
				if (!monBag->add_monster_eggs()->ParseFromString(v.second)) {
					WARN_LOG("Failed to parse a monster egg!");
					monBag->mutable_monster_eggs()->RemoveLast();
				}
				break;
			default:
				WARN_LOG("Unsupported key %s!", v.first.c_str());
				break;
			}
		} else { // 玩家其他数据
			auto field = descriptor->FindFieldByName(v.first);
			if (field) {
				auto msg = reflection->MutableMessage(&data, field);
				if (!msg->ParseFromString(v.second)) {
					WARN_LOG("Failed to parse %s!", field->name().c_str());
					reflection->ClearField(&data, field);
				}
			}
		}
	}


	return true;
}

ErrCodeType PlayerData::doGetPlayerAndMonBasicInfo(uint32_t plid, uint32_t targetID, cs::PlayerSimpleInfo* playerInfo, cs::MonsterSimpleInfo* monInfo, bool lastRegion)
{
	string key(kKeyPrefixPlayerData + to_string(targetID));
	unordered_map<string, string> m;
	auto& accInfoStr = m[kAccountInfo];
	auto& monBagStr = m[kMonsterBag];
	auto& olStatStr = m[kOnlineStatInfo];
	auto& playerAttrStr = m[kPlayerAttr];
	auto& regInfoStr = m[kRegInfo];
	auto& garmentInfoStr = m[kGarWearing];
	//auto& collectionStr = m[kCollectionInfo];
	auto& playerSettingsStr = m[kPlayerSettings];
	auto& digivice = m[kPlayerDigivice];

	if (!gRedis->hget(key, m)) {
		WARN_LOG("hget failed: %s! plid=%u", gRedis->last_error_message().c_str(), plid);
		return ErrCodeDB;
	}

	if (accInfoStr.empty()) {
		DEBUG_LOG("No such player! plid=%u target=%u", plid, targetID);
		return ErrCodeEntryNotFound;
	}

	db::AccountInfo accInfo;
	accInfo.ParseFromString(accInfoStr);
	db::RegInfo regInfo;
	regInfo.ParseFromString(regInfoStr);
	db::MonsterBag monBag;
	monBag.ParseFromString(monBagStr);
	db::PlayerOnlineStatInfo olStat;
	olStat.ParseFromString(olStatStr);
	cs::PlayerAttr playerAttr;
	playerAttr.ParseFromString(playerAttrStr);
	cs::DigiviceInfo digiviceInfo;
	digiviceInfo.ParseFromString(digivice);

	cs::GarmentWearing garmentWearing;
	garmentWearing.ParseFromString(garmentInfoStr);

	cs::KeyVals playerSettings;
	playerSettings.ParseFromString(playerSettingsStr);
	playerInfo->set_player_id(targetID);
	playerInfo->mutable_player_name()->swap(*accInfo.mutable_player_name());
	playerInfo->set_player_level(playerAttr.lv());
	playerInfo->set_gender(accInfo.gender());
	playerInfo->mutable_birthday()->swap(*accInfo.mutable_birthday());
	playerInfo->mutable_picture()->swap(*accInfo.mutable_picture());
	playerInfo->mutable_slogan()->swap(*accInfo.mutable_slogan());
	playerInfo->mutable_momo_account()->swap(*regInfo.mutable_account()->mutable_account_type());
	playerInfo->set_primary_monster_id(monBag.primary_id());
	playerInfo->set_monster_num(monBag.monster_num());
	playerInfo->set_last_online_time(olStat.last_online_time());
	playerInfo->set_hide_momo_info(accInfo.hide_momo_info());
	playerInfo->mutable_wearing()->Swap(&garmentWearing);
	playerInfo->mutable_wearing()->set_body_size(accInfo.body_size());
	playerInfo->set_avatar_frame(accInfo.avatar_frame());
	playerInfo->set_title(accInfo.title());
	playerInfo->set_avatar(accInfo.avatar());
	playerInfo->set_enable_follow(getPlayerSetting(&playerSettings, cs::PlayerSettingType::PlayerSettingEnableFollow) == 1 ? true : false);
	playerInfo->set_digicnt(digiviceInfo.lv());
	playerInfo->set_digivice_skin(digiviceInfo.skin());
	
	/*cs::PlayerCollection collection;
	collection.ParseFromString(collectionStr);
	uint32_t collectionCnt = 0;
	for (auto cc : collection.collections()) {
		if (!cc.state()) {
			continue;
		}
		auto monItm = gCSVMonster.GetItem(cc.mon_id());
		if (!monItm) {
			continue;
		}
		if (cc.state() == 2) {
			collectionCnt += monItm->HandbookGetPoint;
		} else if (cc.state() == 1) {
			collectionCnt += monItm->HandbookMeetPoint;
		}
	}
	playerInfo->set_collection_cnt(collectionCnt);*/
	string cacheKey = makePlayerCacheKey(targetID);
	unordered_map<string, string> cacheM;
	auto& collectPoint = cacheM[kCacheFieldCollectionPoint];
	gRedis->hget(cacheKey, cacheM);
	playerInfo->set_collection_cnt(atoi(collectPoint.c_str()));

	if (monBag.primary_guid()) {
		m.clear();
		auto& monStr = m[kMonHashKeyPrefix + to_string(monBag.primary_guid())];
		gRedis->hget(key, m); // 这里拉取失败就算了，无所谓
		if (monStr.size()) {
			db::MonsterInfo mon;
			mon.ParseFromString(monStr);
			monInfo->set_id(mon.id());
			monInfo->set_primary_id(monBag.primary_id());
			monInfo->mutable_nick()->swap(*mon.mutable_nick());
			monInfo->set_nature(mon.nature());
			monInfo->set_lv(mon.lv());
			monInfo->set_exp(mon.exp());
			monInfo->set_max_stage(mon.max_stage());
		}
	}

	bool exist = false;
	string key1(kKeyPrefixExtrPhotos + to_string(targetID));
	string value;
	if (!gRedis->get(key1, value, &exist)) {
		WARN_LOG("exists failed: %s!", gRedis->last_error_cstr());
		return ErrCodeDB;
	}
	if (exist) {
		playerInfo->mutable_info()->ParseFromString(value);
	}


	//战队信息
	exist = false;
	key1 = makePlayersKey(to_string(targetID));
	if (!gRedis->get(key1, value, &exist)) {
		WARN_LOG("exists failed: %s!", gRedis->last_error_cstr());
		return ErrCodeDB;
	}
	if (exist) {
		uint32_t val = atoi(value.c_str());
		playerInfo->set_cid(val);
		std::string basic;
		bool exists = false;
		if (!gRedis->get(makeCorpsBasicKey(value), basic, &exists)) {
			WARN_LOG("hgetall failed: %s!corpsid=%u", gRedis->last_error_cstr(), val);
		} else {
			cs::CorpsBasicInfo binfo;
			binfo.ParseFromString(basic);
			playerInfo->set_cname(binfo.corps_name());
			playerInfo->set_icon(binfo.icon());
		}

	}

	// 家族活跃度
	key1 = kKeyPrefixPlayerAcInfo + to_string(targetID);
	unordered_map<string, string> fields;
	fields[std::to_string(cs::kCorpsAcVal)];
	if (!gRedis->hget(key1, fields)) {
		WARN_LOG("hset failed: %s! plid=%u", gRedis->last_error_cstr(), targetID);
		return ErrCodeDB;
	}
	cs::PlayerAcInfo acinfo;
	acinfo.ParseFromString(fields[std::to_string(cs::kCorpsAcVal)]);
	if (acinfo.deadtm() <= (LastDay() + 5 * 3600)) {
		playerInfo->set_active(0);
	} else {
		playerInfo->set_active(acinfo.val());
	}

	string strPlayerID = to_string(targetID);
	db::SimpleSocialInfo ssInfo;
	GetSocialSimpleInfo(strPlayerID, ssInfo);
	playerInfo->mutable_social_info()->set_total_popular(ssInfo.total_popular());
	playerInfo->mutable_social_info()->set_total_wealth(ssInfo.total_wealth());
	playerInfo->mutable_social_info()->set_popular_lv(ssInfo.popular_lv());
	playerInfo->mutable_social_info()->set_wealth_lv(ssInfo.wealth_lv());

	string worshipKey = makeWorshipKey(targetID);
	string strDay(to_string(DayNumber()));
	unordered_map<string, string> mWorship;
	auto& strDayCnt = mWorship[strDay];
	if (!gRedis->hget(worshipKey, mWorship)) {
		WARN_LOG("hget failed: %s! plid=%u", gRedis->last_error_cstr(), targetID);
		return ErrCodeDB;
	}
	playerInfo->set_worshiped_cnt(atoi(strDayCnt.c_str()));

	// 许愿暂时米有了
	/*cs::WishDataWithRank wishMsg;
	SSProtoHead h;
	h.TargetID = targetID;
	wishData.GetWishData(h, nullptr, &wishMsg);
	for (auto wish : wishMsg.wish_data().wishes()) {
		if (wish.item_max_cnt() != wish.item_cur_cnt()) {
			playerInfo->set_uncomplete_wish(true);
			break;
		}
	}*/

	uint32_t giftCnt = 0;
	calTotalGiftCnt(targetID, giftCnt);
	playerInfo->set_total_gift_cnt(giftCnt);

	cs::ArenaInfo aInfo;
	if (ErrCodeType::ErrCodeSucc == gArena.GetInfoImp(targetID, aInfo)) {
		if (lastRegion) {
			playerInfo->set_arena_region(aInfo.last_region());
		} else {
			playerInfo->set_arena_region(aInfo.region());
		}
	}

	//DEBUG_LOG("playerInfo %s ",playerInfo->Utf8DebugString().c_str());
	return ErrCodeSucc;
}

void PlayerData::doGetFriendBasicInfo(uint32_t plid, uint32_t targetID, db::GetFriendsBasicInfoRsp& rsp)
{
	string key(kKeyPrefixPlayerData + to_string(targetID));
	unordered_map<string, string> m;
	auto& accInfoStr = m[kAccountInfo];
	auto& onlineInfoStr = m[kOnlineStatInfo];
	auto& playerAttrStr = m[kPlayerAttr];
	auto& garmentInfoStr = m[kGarWearing];
	auto& lazySocialInfo = m[kLazySocialInfo];
	auto& digivice = m[kPlayerDigivice];
	if (!gRedis->hget(key, m)) {
		WARN_LOG("hget failed: %s! plid=%u target=%u", gRedis->last_error_cstr(), plid, targetID);
		return;
	}

	if (accInfoStr.empty()) {
		DEBUG_LOG("Player not found. plid=%u target=%u", plid, targetID);
		return;
	}

	db::AccountInfo accInfo;
	accInfo.ParseFromString(accInfoStr);
	db::PlayerOnlineStatInfo olInfo;
	olInfo.ParseFromString(onlineInfoStr);
	cs::PlayerAttr playerAttr;
	playerAttr.ParseFromString(playerAttrStr);
	cs::GarmentWearing garmentWearing;
	garmentWearing.ParseFromString(garmentInfoStr);
	db::LazySocialAttr lazySoical;
	lazySoical.ParseFromString(lazySocialInfo);
	cs::DigiviceInfo digiviceInfo;
	digiviceInfo.ParseFromString(digivice);

	auto friendInfo = rsp.add_info();
	friendInfo->set_player_id(targetID);
	friendInfo->set_allocated_player_name(accInfo.release_player_name());
	friendInfo->set_player_level(playerAttr.lv());
	friendInfo->set_gender(accInfo.gender());
	friendInfo->set_allocated_birthday(accInfo.release_birthday());
	friendInfo->set_allocated_picture(accInfo.release_picture());
	friendInfo->set_allocated_slogan(accInfo.release_slogan());
	friendInfo->set_last_online_time(olInfo.last_online_time());
	friendInfo->mutable_wearing()->Swap(&garmentWearing);
	friendInfo->mutable_wearing()->set_body_size(accInfo.body_size());
	friendInfo->mutable_social_info()->mutable_geo_info()->Swap(lazySoical.mutable_geo_info());
	friendInfo->set_digicnt(digiviceInfo.lv());

	friendInfo->set_avatar_frame(accInfo.avatar_frame());
	friendInfo->set_title(accInfo.title());
	friendInfo->set_avatar(accInfo.avatar());

	friendInfo->mutable_player_attr()->CopyFrom(playerAttr);

	cs::ArenaInfo aInfo;
	gArena.GetInfoImp(targetID, aInfo);
	friendInfo->set_arena_region(aInfo.region());

	string strPlayerID = to_string(targetID);
	db::SimpleSocialInfo ssInfo;
	GetSocialSimpleInfo(strPlayerID, ssInfo);
	friendInfo->mutable_social_info()->set_total_popular(ssInfo.total_popular());
	friendInfo->mutable_social_info()->set_total_wealth(ssInfo.total_wealth());
	friendInfo->mutable_social_info()->set_wealth_lv(ssInfo.wealth_lv());

	//战队信息
	bool exist = false;
	std::string key1 = makePlayersKey(to_string(targetID));
	std::string value;
	if (!gRedis->get(key1, value, &exist)) {
		WARN_LOG("exists failed: %s!", gRedis->last_error_cstr());
		return;
	}
	if (exist) {
		uint32_t val = atoi(value.c_str());
		friendInfo->set_cid(val);
		std::string basic;
		bool exists = false;
		if (!gRedis->get(makeCorpsBasicKey(value), basic, &exists)) {
			WARN_LOG("hgetall failed: %s!corpsid=%u", gRedis->last_error_cstr(), val);
		} else {
			cs::CorpsBasicInfo binfo;
			binfo.ParseFromString(basic);
			friendInfo->set_cname(binfo.corps_name());
			friendInfo->set_icon(binfo.icon());
		}
	}

	// 家族的活跃度
	key1 = kKeyPrefixPlayerAcInfo + to_string(targetID);
	unordered_map<string, string> fields;
	fields[std::to_string(cs::kCorpsAcVal)];
	if (!gRedis->hget(key1, fields)) {
		WARN_LOG("hset failed: %s! plid=%u", gRedis->last_error_cstr(), targetID);
		return;
	}
	cs::PlayerAcInfo acinfo;
	acinfo.ParseFromString(fields[std::to_string(cs::kCorpsAcVal)]);
	if (acinfo.deadtm() <= (LastDay() + 5 * 3600)) {
		friendInfo->set_active(0);
	} else {
		friendInfo->set_active(acinfo.val());
	}

	/*uint32_t cnt = 0;
	uint32_t skin = 1;
	calMaxDigiCnt(targetID, cnt, skin);
	friendInfo->set_digicnt(cnt);*/
}

void PlayerData::toFriendsData(unordered_map<string, string>& m, unordered_map<string, string>& momo, google::protobuf::RepeatedPtrField<cs::FriendsData_OneFriend>* friends)
{
	for (const auto& v : m) {
		auto itm = friends->Add();
		itm->set_player_id(atoi(v.first.c_str()));
		itm->set_friendship(atoi(v.second.c_str()));
		auto it = momo.find(v.first);
		if (it != momo.end()) {
			itm->set_momo_friendship(atoi(it->second.c_str()));
			// 对方是momo用户的话要设momoid
			string regKey(kKeyPrefixPlayerData + v.first);
			unordered_map<string, string> mReg;
			auto& regInfoStr = mReg[kRegInfo];
			if (!gRedis->hget(regKey, mReg)) {
				WARN_LOG("hget failed: %s! plid=%s", gRedis->last_error_message().c_str(), v.first.c_str());
				continue;
			}
			db::RegInfo regInfo;
			regInfo.ParseFromString(regInfoStr);
			if (regInfo.account().account_type() == kChannelNameMomo) {
				itm->set_momo_id(regInfo.account().account_id());
			}
		}
	}
	for (const auto& v : momo) {
		auto it = m.find(v.first);
		if (it == m.end()) {
			auto itm = friends->Add();
			itm->set_player_id(atoi(v.first.c_str()));
			itm->set_momo_friendship(atoi(it->second.c_str()));
			// 对方是momo用户的话要设momoid
			string regKey(kKeyPrefixPlayerData + v.first);
			unordered_map<string, string> mReg;
			auto& regInfoStr = mReg[kRegInfo];
			if (!gRedis->hget(regKey, mReg)) {
				WARN_LOG("hget failed: %s! plid=%s", gRedis->last_error_message().c_str(), v.first.c_str());
				continue;
			}
			db::RegInfo regInfo;
			regInfo.ParseFromString(regInfoStr);
			if (regInfo.account().account_type() == kChannelNameMomo) {
				itm->set_momo_id(regInfo.account().account_id());
			}
		}
	}
}

void PlayerData::doGetPlayerAdventureTowerDatas(uint32_t plid, uint32_t targetID, google::protobuf::RepeatedPtrField<cs::AdventureData_AdventureTowerData>* towerDatas)
{
	string key(kKeyPrefixPlayerData + to_string(targetID));
	unordered_map<string, string> m;
	auto& adventureInfo = m[kAdventureInfo];
	if (!gRedis->hget(key, m)) {
		WARN_LOG("hget failed: %s! plid=%u target=%u", gRedis->last_error_cstr(), plid, targetID);
		return;
	}

	cs::AdventureData adventureData;
	adventureData.ParseFromString(adventureInfo);
	if(!adventureData.tower_datas().empty()) {
		towerDatas->Swap(adventureData.mutable_tower_datas());
	}
}

ErrCodeType PlayerData::checkDailyPopular(uint32_t plid, int* yPopular)
{
	string key(kKeyPrefixSocialData + to_string(plid));

	unordered_map<string, string> social;
	auto& totayPopluar = social[kSocialFieldTodayPopular];
	auto& yesterdayPopluar = social[kSocialFieldYesterdayPopular];
	auto& lastPopularTime = social[kSocialFieldLastPopularTime];
	if (!gRedis->hgetall(key, social)) {
		WARN_LOG("hgetall failed: %s! plid=%u", gRedis->last_error_cstr(), plid);
		return ErrCodeDB;
	}

	bool modifyFlag = false;
	uint32_t popularTime = atol(lastPopularTime.c_str());
	if (popularTime > 0) {
		int dayDiff = DayDiff(time(NULL), popularTime);
		if (dayDiff >= 2) {
			totayPopluar = to_string(0);
			yesterdayPopluar = to_string(0);
			lastPopularTime = to_string(time(NULL));
			modifyFlag = true;
		} else if (dayDiff >= 1) {
			yesterdayPopluar = totayPopluar;
			totayPopluar = to_string(0);
			lastPopularTime = to_string(time(NULL));
			modifyFlag = true;
		}
	} else {
		lastPopularTime = to_string(time(NULL));
		modifyFlag = true;
	}

	if (yPopular) {
		*yPopular = atoi(yesterdayPopluar.c_str());
	}

	if (modifyFlag && !gRedis->hset(key, social)) {
		WARN_LOG("hset failed: %s! plid=%u", gRedis->last_error_cstr(), plid);
		return ErrCodeDB;
	}
	
	return ErrCodeSucc;
}

ErrCodeType PlayerData::doAddDailyPopular(uint32_t plid, uint32_t addValue)
{
	ErrCodeType retCode = checkDailyPopular(plid);
	if (retCode != ErrCodeSucc) {
		return retCode;
	}

	long long newValue = 0;
	string key(kKeyPrefixSocialData + to_string(plid));
	if (!gRedis->hincrby(key, kSocialFieldTodayPopular, addValue, &newValue)) {
		WARN_LOG("hincrby failed: %s! plid=%u", gRedis->last_error_cstr(), plid);
		return ErrCodeDB;
	}
	
	return ErrCodeSucc;
}

static const string kScriptCheckWeeklySocial =
		"local r = redis.call('HGET', KEYS[1], ARGV[3])\n"
		"if not r or tonumber(r) < tonumber(ARGV[1]) then\n"
		"  redis.call('HMSET', KEYS[1], ARGV[3], ARGV[2], ARGV[4], 0, ARGV[5], 0, ARGV[6], 0, ARGV[7], 0, ARGV[8], 0)\n"
		"end\n"
		"return 0";
ErrCodeType PlayerData::checkWeeklySocial(uint32_t plid)
{
	string lastMondayStamp = to_string(LastMonday());
	string nowStamp = to_string(time(nullptr));
	vector<string> keys = { kKeyPrefixSocialData + to_string(plid) };
	vector<string> args = { lastMondayStamp, nowStamp, kSocialFieldResetWeekTime, kSocialFieldWeekPopular, 
		kSocialFieldWeekMomoPopular, kSocialFieldWeekLove, kSocialFieldWeekMomoLove,
		kSocialFieldWeekWealth };

	ScopedReplyPointer reply = gRedis->eval(kScriptCheckWeeklySocial, &keys, &args);
	CHECK_REPLY_EC(reply, REDIS_REPLY_INTEGER, plid);
	return ErrCodeSucc;
}

static const string kScriptCheckDailySocial =
		"local r = redis.call('HGET', KEYS[1], ARGV[3])\n"
		"if not r or tonumber(r) < tonumber(ARGV[1]) then\n"
		"  redis.call('HMSET', KEYS[1], ARGV[3], ARGV[2], ARGV[4], 0, ARGV[5], 0)\n"
		"end\n"
		"return 0";
ErrCodeType PlayerData::checkDailySocial(uint32_t plid)
{
	string lastDayStamp = to_string(LastDay());
	string nowStamp = to_string(time(nullptr));
	vector<string> keys = { kKeyPrefixSocialData + to_string(plid) };
	vector<string> args = { lastDayStamp, nowStamp, kSocialFieldResetDailyTime, kSocialFieldDailyPopular, kSocialFieldDailyWealth };

	ScopedReplyPointer reply = gRedis->eval(kScriptCheckDailySocial, &keys, &args);
	CHECK_REPLY_EC(reply, REDIS_REPLY_INTEGER, plid);
	return ErrCodeSucc;
}

//void PlayerData::doGetPlayerHomeInfo(uint32_t plid, cs::PlayerSimpleInfo* playerInfo, 
//									google::protobuf::RepeatedField<::google::protobuf::int32>* monIds, 
//									db::PlayerHomeBuffInfo* homeBuffs)
void PlayerData::doGetPlayerHomeInfo(uint32_t plid, cs::PlayerSimpleInfo* playerInfo, std::string& mon_nick, db::PlayerHomeBuffInfo* homeBuffs)
{
	string key(kKeyPrefixPlayerData + to_string(plid));
	unordered_map<string, string> m;
	auto& accInfoStr = m[kAccountInfo];
	auto& monBagStr = m[kMonsterBag];
	auto& homeBuffStr = m[kHomeBuffInfo];
	auto& wearingStr = m[kGarWearing];
	if (!gRedis->hget(key, m)) {
		WARN_LOG("doGetPlayerHomeInfo hget failed: %s!", gRedis->last_error_message().c_str());
		return;
	}

	// 判断该玩家是否存在
	bool playerFound = false;
	for (const auto& v : m) {
		if (v.second.size()) {
			playerFound = true;
			break;
		}
	}
	if (!playerFound) {
		DEBUG_LOG("No such player! plid=%u", plid);
		return;
	}

	db::PlayerData playerData;
	auto& accInfo = *playerData.mutable_account_info();
	accInfo.ParseFromString(accInfoStr);
	auto& monBag = *playerData.mutable_monster_bag();
	monBag.ParseFromString(monBagStr);
	auto& buffInfo = *playerData.mutable_homebuff_info();
	buffInfo.ParseFromString(homeBuffStr);
	//auto& wearingInfo = *playerData.mutable_garment_wearing();
	//wearingInfo.ParseFromString(wearingStr);

	playerInfo->set_player_id(accInfo.player_id());
	playerInfo->mutable_player_name()->swap(*accInfo.mutable_player_name());
	playerInfo->mutable_picture()->swap(*accInfo.mutable_picture());
	playerInfo->mutable_birthday()->swap(*accInfo.mutable_birthday());
	playerInfo->set_primary_monster_id(monBag.primary_id());
	playerInfo->set_gender(accInfo.gender());
	playerInfo->mutable_wearing()->ParseFromString(wearingStr);
	playerInfo->set_avatar_frame(accInfo.avatar_frame());
	playerInfo->set_title(accInfo.title());
	playerInfo->set_avatar(accInfo.avatar());

	cs::ArenaInfo aInfo;
	gArena.GetInfoImp(plid, aInfo);
	playerInfo->set_arena_region(aInfo.region());

	// 主宠的昵称
	if (monBag.primary_guid()) {
		unordered_map<string, string> monIdKey;
		auto& monStr = monIdKey[kMonHashKeyPrefix + to_string(monBag.primary_guid())];
		gRedis->hget(key, monIdKey); // 这里拉取失败就算了，无所谓
		if (monStr.size()) {
			db::MonsterInfo mon;
			mon.ParseFromString(monStr);
			if (mon.has_nick()) {
				mon_nick = mon.nick();
			}
		}
	}

	homeBuffs->Swap(&buffInfo);
	return;
}

void PlayerData::doGetPlayerHomeBuffInfo(uint32_t plid, db::PlayerHomeBuffInfo* homeBuffs)
{
	string key(kKeyPrefixPlayerData + to_string(plid));
	unordered_map<string, string> m;
	auto& homeBuffStr = m[kHomeBuffInfo];
	if (!gRedis->hget(key, m)) {
		WARN_LOG("doGetPlayerHomeInfo hget failed: %s!", gRedis->last_error_message().c_str());
		return;
	}

	// 判断该玩家是否存在
	bool playerFound = false;
	for (const auto& v : m) {
		if (v.second.size()) {
			playerFound = true;
			break;
		}
	}
	if (!playerFound) {
		//DEBUG_LOG("No such player! plid=%u", plid);
		return;
	}

	homeBuffs->ParseFromString(homeBuffStr);
	return;
}

ErrCodeType PlayerData::GetPopAndLove(std::string& strPlayerID, long long* totalPopular, long long* totalLove, long long* weekPopular, long long* weekLove)
{
	string key(kKeyPrefixSocialData + strPlayerID);

	ErrCodeType errCode = checkWeeklySocial(atoi(strPlayerID.c_str()));
	if (errCode) {
		return errCode;
	}
	errCode = checkDailySocial(atoi(strPlayerID.c_str()));
	if (errCode) {
		return errCode;
	}
	// stringstream ss;
	// ss << kKeyPrefixSocialData << to_string(NextMonday()) << '_' << strPlayerID;
	// string weekKey = ss.str();

	// 取总值和周值
	unordered_map<string, string> m;
	string& gameTotalPopular = m[kSocialFieldPopular];
	string& momoTotalPopular = m[kSocialFieldMomoPopular];
	string& gameTotalLove = m[kSocialFieldLove];
	string& momoTotalLove = m[kSocialFieldMomoLove];
	string& gameWeekPopular = m[kSocialFieldWeekPopular];
	string& momoWeekPopular = m[kSocialFieldWeekMomoPopular];
	string& gameWeekLove = m[kSocialFieldWeekLove];
	string& momoWeekLove = m[kSocialFieldWeekMomoLove];
	if (!gRedis->hget(key, m)) {
		WARN_LOG("hget failed: %s! plid=%s", gRedis->last_error_cstr(), strPlayerID.c_str());
		return ErrCodeDB;
	}
	*totalPopular = atoi(gameTotalPopular.c_str()) + atoi(momoTotalPopular.c_str());
	*totalLove = atoi(gameTotalLove.c_str()) + atoi(momoTotalLove.c_str());
	// 取周值
	// m.clear();
	// string& gameWeekPopular = m[kSocialFieldPopular];
	// string& momoWeekPopular = m[kSocialFieldMomoPopular];
	// string& gameWeekLove = m[kSocialFieldLove];
	// string& momoWeekLove = m[kSocialFieldMomoLove];
	// if (!gRedis->hget(weekKey, m)) {
	// 	WARN_LOG("hget failed: %s! plid=%s", gRedis->last_error_cstr(),strPlayerID.c_str());
	// 	return ErrCodeDB;
	// }
	*weekPopular = atoi(gameWeekPopular.c_str()) + atoi(momoWeekPopular.c_str());
	*weekLove = atoi(gameWeekLove.c_str()) + atoi(momoWeekLove.c_str());

	return ErrCodeSucc;
}

ErrCodeType PlayerData::GetSocialSimpleInfo(std::string& strPlayerID, db::SimpleSocialInfo& info)
{
	string key(kKeyPrefixSocialData + strPlayerID);

	ErrCodeType errCode = checkWeeklySocial(atoi(strPlayerID.c_str()));
	if (errCode) {
		return errCode;
	}
	errCode = checkDailySocial(atoi(strPlayerID.c_str()));
	if (errCode) {
		return errCode;
	}

	// 取总值和周值
	unordered_map<string, string> m;
	string& gameTotalPopular = m[kSocialFieldPopular];
	string& momoTotalPopular = m[kSocialFieldMomoPopular];
	string& gameWeekPopular = m[kSocialFieldWeekPopular];
	string& momoWeekPopular = m[kSocialFieldWeekMomoPopular];
	string& gameDailyPopular = m[kSocialFieldDailyPopular];
	string& gameTotalWealth = m[kSocialFieldWealth];
	string& gameWeekWealth = m[kSocialFieldWeekWealth];
	string& gameDailyWealth = m[kSocialFieldDailyWealth];
	string& popularLv = m[kSocialFieldPopularLv];
	string& wealthLv = m[kSocialFieldWealthLv];
	if (!gRedis->hget(key, m)) {
		WARN_LOG("hget failed: %s! plid=%s", gRedis->last_error_cstr(), strPlayerID.c_str());
		return ErrCodeDB;
	}

	info.set_total_popular(atoi(gameTotalPopular.c_str()) + atoi(momoTotalPopular.c_str()));
	info.set_week_popular(atoi(gameWeekPopular.c_str()) + atoi(momoWeekPopular.c_str()));
	info.set_daily_popular(atoi(gameDailyPopular.c_str()));

	info.set_total_wealth(atoi(gameTotalWealth.c_str()));
	info.set_week_wealth(atoi(gameWeekWealth.c_str()));
	info.set_daily_wealth(atoi(gameDailyWealth.c_str()));

	info.set_popular_lv(atoi(popularLv.c_str()));
	info.set_wealth_lv(atoi(wealthLv.c_str())); // 财富值本来就是从0开始的

	return ErrCodeSucc;
}

int PlayerData::GetPlayerTotalPopluar(uint32_t plid)
{
	string key(kKeyPrefixSocialData + std::to_string(plid));

	unordered_map<string, string> m;
	string& gameTotalWealth = m[kSocialFieldWealth];
	if (!gRedis->hget(key, m)) {
		WARN_LOG("hget failed: %s! plid=%u", gRedis->last_error_cstr(), plid);
		return 0;
	}

	return atoi(gameTotalWealth.c_str());
}

int PlayerData::GetPlayerTotalWealth(uint32_t plid)
{
	string key(kKeyPrefixSocialData + std::to_string(plid));

	unordered_map<string, string> m;
	string& gameTotalPopular = m[kSocialFieldPopular];
	string& momoTotalPopular = m[kSocialFieldMomoPopular];
	if (!gRedis->hget(key, m)) {
		WARN_LOG("hget failed: %s! plid=%u", gRedis->last_error_cstr(), plid);
		return 0;
	}

	return atoi(gameTotalPopular.c_str()) + atoi(momoTotalPopular.c_str());
}

int PlayerData::GetPlayerTrialTowerRankScore(uint32_t plid)
{
	string key(kKeyPrefixPlayerData + to_string(plid));
	unordered_map<string, string> m;
	auto& trialTowerDataStr = m[kTrialTower];
	int maxPassFloor = 0;
	int passRecord = 0;
	int score = (100000 - passRecord);
	if (!gRedis->hget(key, m)) {
		WARN_LOG("hget failed: %s! plid=%u", gRedis->last_error_message().c_str(), plid);
		return score;
	}
	
	if(trialTowerDataStr.empty()) {
		return score;
	} else {
		cs::TrialTowerData trialTowerData;
		trialTowerData.ParseFromString(trialTowerDataStr);
		maxPassFloor = trialTowerData.max_pass_floor();
		for (auto kv : trialTowerData.records()) {
			if(maxPassFloor ==(int) kv.attr_id()) {
				passRecord = kv.attr_val();
			}
		}
		return maxPassFloor * 1000000 + (100000 - passRecord);
	}
}


int PlayerData::GetGameRegion(uint32_t plid)
{
	unordered_map<string, string> mp;
	auto& str = mp[kPlayerGameRegion];
	std::string key = (kKeyPrefixPlayerData + to_string(plid));
	if (!gRedis->hget(key, mp)) {
		WARN_LOG("hget failed: %s! plid=%u ", gRedis->last_error_message().c_str(), plid);
		return ErrCodeDB;
	}
	db::Uint32Req info;
	info.ParseFromString(str);
	if (info.has_u32()) {
		return info.u32();
	}
	else {
		return 1;
	}
}

ErrCodeType PlayerData::calMaxDigiCnt(uint32_t playerId, uint32_t &cnt, uint32_t &skin)
{
	unordered_map<string, string> mp;
	auto& digivice = mp[kPlayerDigivice];
	std::string key = (kKeyPrefixPlayerData + to_string(playerId));
	if (!gRedis->hget(key, mp)) {
		WARN_LOG("hget failed: %s! plid=%u ", gRedis->last_error_message().c_str(), playerId);
		return ErrCodeDB;
	}

	cs::DigiviceInfo info;
	info.ParseFromString(digivice);
	cnt = info.lv();
	skin = info.skin();
	// for (int i = 0; i < info.digivice_stage_size(); ++i) {
	// 	//硬代码
	// 	if (cnt < (uint32_t)info.digivice_stage(i).id() 
	// 				&& info.digivice_stage(i).state() == 2) {
	// 		cnt = info.digivice_stage(i).id();
	// 	}
	// }
	
	return ErrCodeSucc;

}

ErrCodeType PlayerData::calTotalGiftCnt(uint32_t playerID, uint32_t &cnt)
{
	unordered_map<string, string> gifts;
	string giftKey(kKeyPrefixGiftItem + to_string(playerID));
	if (!gRedis->hgetall(giftKey, gifts)) {
		WARN_LOG("hgetall failed: %s! plid=%u", gRedis->last_error_cstr(), playerID);
		return ErrCodeDB;
	}

	for (auto v : gifts) {
		uint32_t itemId = atoi(v.first.c_str());
		uint32_t itemCnt = atoi(v.second.c_str());
		if (itemId > 0) {
			cnt += itemCnt;
		}
	}
	return ErrCodeSucc;
}
/*
int PlayerData::calAvatar(cs::GarmentWearing* info, int avatar, int gender)
{
	// 头像如果没设，则由头饰决定
	if (avatar) {
		return avatar;
	} else {
		uint32_t head = 0;
		for (auto it : info->wearing()) {
			if (it.pos() == cs::GarmentHead) {
				head = it.id();
				break;
			}
		}
		if (head == 0) {
			head = 100000000 + gender;
		}
		auto avIt = gPlayerData.avatarMap_.find(head);
		if (avIt != gPlayerData.avatarMap_.end()) {
			return avIt->second;
		}
	}
	return 0;
}
*/
int PlayerData::getPlayerSetting(cs::KeyVals* settings, cs::PlayerSettingType type)
{
	for(const auto& kv : settings->vals()) {
		if((int)kv.attr_id() == (int)type) {
			return (int)kv.attr_val();
		}
	}
	return 0;
}

void PlayerData::setPlayerDataTouchInfo(uint32_t plid, const std::string& tKey, bool setFlag)
{
	string filed = tKey + (setFlag ? "_S" : "_G");
	string key = kKeyPlayerDataTouchInfo + to_string(plid);

	unordered_map<string, string> m;
	m[filed] = to_string(time(NULL));
	if (!gRedis->hset(key, m)) {
		WARN_LOG("hset failed: %s! plid=%u", gRedis->last_error_cstr(), plid);
	}

	return;
}

bool PlayerData::IsGsMember(uint32_t plid, bool ever)
{
	unordered_map<string, string> social;
	auto& gsMem = social[kSocialFieldGsMember];

	string strPlayerID = to_string(plid);
	if (!gRedis->hget(kKeyPrefixSocialData + strPlayerID, social)) {
		WARN_LOG("hget failed: %s! plid=%u", gRedis->last_error_cstr(), plid);
		return false;
	}

	db::GsMemberInfo gsm;
	gsm.ParseFromString(gsMem);
	if (ever) {
		return gsm.time_beg() > 0;
	}

	uint32_t now = time(0);
	return gsm.time_beg() <= now && gsm.time_end() >= now;
}

void PlayerData::updatePlayerLoginRecord(uint32_t pid, std::string ip)
{
	DEBUG_LOG("updatePlayerLoginRecord, pid:%u, ip:%s", pid, ip.c_str());
	TblLoginRecord mysqlLoginRecord(pid, "digimon", "login_record");
	mysqlLoginRecord.AddLoginRecord(ip);
}
