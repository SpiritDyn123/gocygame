/*
 * KeyDef.cc
 *
 *  Created on: 2017年5月3日
 *      Author: antigloss
 */

#include <arpa/inet.h>
#include "KeyPrefixDef.h"

using namespace std;

const std::string kKeyPrefixPlayerData("PD"); // PlayerData前缀
const std::string kKeyPrefixNickname("Nick-"); // 用于保证昵称唯一性的数据前缀
const std::string kKeyPrefixOfflineMsg("Msg"); // 离线消息前缀
const std::string kKeyPrefixTodayLoginedPlayers("TodayPlayers"); // 当日登录玩家前缀(过了新手引导的)
const std::string kKeyPrefixAllTodayLoginedPlayers("AllTodayPlayers"); // 当日登录玩家前缀
const std::string kKeyPrefixAllTodayLoginedPlayersSnapshot("AllTodayPlayersSnapshot");
const std::string kKeyPrefixLogin("LG");
const std::string kKeyPLayerGUID("LGGUID");

const std::string kKeyPrefixFollowing("Frd{"); // 关注列表前缀
const std::string kKeyPrefixFavFollowing("FavFrd{"); // 特别关注列表前缀
const std::string kKeyPrefixFollower("Fan{"); // 粉丝列表前缀
const std::string kKeyPrefixBlacklist("Black_V1{"); // 黑名单前缀
const std::string kKeyPrefixIntersection("InterSection{"); // 互相关注前缀
const std::string kKeyPrefixFans("Fans{"); // 粉丝列表前缀

const std::string kKeyPrefixHatchData("HD"); // 孵化数据前缀
const std::string kKeyPrefixHatchMsg("HMsg"); // 孵化消息前缀

const std::string kKeyPrefixHomeDecorateData("Decorate");

const std::string kKeyPrefixHomeResFactory("ResFactory");	//家园资源生产
const std::string kKeyPrefixHomeResHire("ResFacHire");	//家园资源租赁

const std::string kKeyPrefixMailData("MAIL"); // 玩家邮件数据前缀
const std::string kKeyGlobalMailData("GMAIL"); // 全局邮件数据key

const std::string kKeyPrefixWishData("Wish"); // 心愿数据前缀
const std::string kKeyPrefixWishData_V1("Wish_V1"); // 心愿数据前缀V1
const std::string kKeyPrefixWishRecord("WishRec_"); // 心愿赠送记录前缀

const std::string kKeyPrefixActivityData("ACT_"); // 活动数据前缀
const std::string kKeyActivtiyDailyDeadTMData("ACTDailyDeadTime"); // 活动每日失效时间

const std::string kKeyTrialTowerRecords("TrialTowerRecords"); // 修炼之塔最佳纪录
// Fields
const std::string kBEDataFieldBanLogin("BanLG"); // BasicExtraData中的封停登录字段
const std::string kBEDataFieldBanChat("BanChat"); // BasicExtraData中的封停聊天字段

const std::string kKeyGsMember("GsMemberInfo"); // gs成员信息 用于房间置顶
const std::string kKeyGsRecRoom("GsRecRooms"); // gs推荐房间信息
const std::string kKeyChannelSwitch("ChannelSwitch"); // 渠道开关
const std::string kMiscFieldBanLoginDevice("BanDeviceLG"); // 封禁的设备号
const std::string kMiscFieldBanLoginIP("BanIPLG"); // 封禁的ip
const std::string kMiscFieldBanChatDevice("BanDeviceChat"); // 封禁的设备号
const std::string kMiscFieldBanChatIP("BanIPChat"); // 封禁的ip

const std::string kKeyPrefixPlayerOfDevice("{Multi}PlayerOfDevice_"); // 设备下账号
const std::string kKeyPrefixPlayerOfIP("{Multi}PlayerOfIP_"); // IP下账号
const std::string kMiscFieldDubiousDevice("{Multi}DubiousDevice"); // 建号数超标的设备
const std::string kMiscFieldDubiousIP("{Multi}DubiousIP"); // 建号数超标的IP

const std::string kMiscFieldBanChannelPay("BanChannelPay"); // 封禁充值的渠道
const std::string kMiscFieldBanChannelLogin("BanChannelLogin"); // 封禁登录的渠道
const std::string kMiscFieldBanChannelCreate("BanChannelCreate"); // 封禁登录的渠道

const std::string kMiscFieldOperationVer("OpVer"); // 运营数据版本号

const std::string kGameMGuide("GameMGuide"); // 男性游戏引导员
const std::string kGameFGuide("GameFGuide"); // 女性游戏引导员

const std::string kKeyPrefixPendingOrder("{Order}Pending_"); 	// 未完成订单
const std::string kKeyPrefixPaidOrder("{Order}Paid_"); 		// 已支付订单
const std::string kKeyPrefixFinishedOrder("{Order}Finished_");// 已完成订单
const std::string kKeyPrefixDayPayOrder("{DayPayNo}_");	// 今日生成订单自增序号
const std::string kKeyPrefixDayPaidOrderQueue("{Order}DayPaid_");	// 今日已支付订单队列
const std::string kMiscFieldBoughtGems("{Order}BoughtGems"); // 玩家购买的宝石数
const std::string kRebateBoughtGems("{Order}RebateBoughtGems"); // 玩家充值返利购买宝石数量
const std::string kKeyOrderGotBoughtGems("{Order}GotRebateGems"); // 玩家是否已领充值返利宝石

const std::string kKeyPrefixRedPoint("RedPt");

const std::string kKeyDelPlayers("DelPlayers");

const std::string kKeyPrefixOfflineReq("OfflineReq_"); // 离线操作
const std::string kKeyPrefixSocialData("Social_"); // 社交属性
const std::string kSocialFieldPopular("Popular"); // 人气
const std::string kSocialFieldMomoPopular("MomoPopular"); // momo内嵌人气
const std::string kSocialFieldPopularLv("PopularLv");
const std::string kSocialFieldLove("Love"); // 爱心值
const std::string kSocialFieldWealth("Wealth"); // 财富值
const std::string kSocialFieldMomoLove("MomoLove"); // momo爱心值
const std::string kSocialFieldWeekPopular("WeekPopular"); // 周人气
const std::string kSocialFieldDailyPopular("DailyPopular"); // 日人气
const std::string kSocialFieldWeekMomoPopular("WeekMomoPopular"); // 周momo内嵌人气
const std::string kSocialFieldWeekLove("WeekLove"); // 周爱心值
const std::string kSocialFieldWeekMomoLove("WeekMomoLove"); // 周momo爱心值
const std::string kSocialFieldWeekWealth("WeekWealth"); // 周财富值
const std::string kSocialFieldDailyWealth("DailyWealth"); // 周财富值
const std::string kSocialFieldWealthLv("WealthLv");
const std::string kSocialFieldResetWeekTime("WeekResetTime"); // 上次重置周数据时间
const std::string kSocialFieldResetDailyTime("DailyResetTime"); // 上次重置日数据时间
const std::string kSocialFieldLikedCnt("LikedCnt"); // 被点赞次数
const std::string kSocialFieldTodayPopular("TodayPopular"); // 人气
const std::string kSocialFieldYesterdayPopular("YesterdayPopular"); // 人气
const std::string kSocialFieldLastPopularTime("LastPopularTime"); // 人气
const std::string kSocialFieldGsMember("GsMemberInfo"); // GsMember信息 与房间置顶相关
const std::string kSocialFieldStrangerCnt("SocialStrangerCnt"); // 社交交互陌生人访问次数
const std::string kSocialFieldLookedCnt("SocialLookedCnt"); // 社交交互次数
const std::string kSocialFieldVisitedCnt("SocialVisitCnt"); // 社交交互家园访问次数
const std::string kKeyPrefixWishRank("Wish_"); // 心愿送礼排行榜
const std::string kKeyPrefixMomoFollowing("MomoFollowing_"); // momo友好度
const std::string kKeyPrefixMomoFollower("MomoFollower_"); // 粉丝对我的momo友好度
const std::string kKeyPrefixWorship("Worship_{"); // 膜拜前缀
const std::string kKeyPrefixStranger("Stranger_{"); // 陌生人首次拜访时间
const std::string kKeyPrefixInteractTime("InteractTime_{"); // 最后一次拜访时间
const std::string kSocialFieldCheckInteractTime("CheckInteractTime"); // 最后查看访客记录时间
const std::string kKeyPrefixInteractInfo("Interact_{"); // 最后一次互动行为记录

const std::string kKeyPrefixFailedSyncMomoItem("FailedSyncMomoItem{"); // 内嵌端同步失败请求前缀

const std::string kKeyPrefixClearRankProgress("{ClearRankProgress}_"); // 榜单结算进度
const std::string kRankFieldArenaRank("Arena"); // Arena榜的名字
const std::string kRankFieldTrialTowerRank("TrialTower"); // Arena榜的名字
const std::string kRankFieldPavilionRank("Pavilion"); // 道馆榜的名字

const std::string kKeyPlayerRecruitData("Recruit_");
const std::string kRecruitFieldCode("InviteCode");
const std::string kRecruitFieldMyRecruitPlid("MyRePlid");
const std::string kRecruitFieldMyRecruitNick("MyReNick");
const std::string kRecruitFieldLoginScore("lgscore");
const std::string kRecruitFieldAdventureScore("adscore");
const std::string kRecruitFieldBtlGrpScore("btlscore");
const std::string kRecruitFieldLevelScore("lvscore");

const std::string kKeyPrefixGiftItem("GiftItem");
const std::string kGiftItemFieldGemValue("GIGemValue");
const std::string kGiftItemFieldGemHistroyValue("GIGemHistroyValue");
const std::string kKeyPrefixGiftGivenRecord("GiftRecvRecord");
const std::string kKeyPrefixGiftGivenRank("GiftRecvRank");

// 地理位置
const std::string kKeyPrefixGeoCountry("{Geo}Country_"); // 国家key前缀
const std::string kKeyPrefixGeoProvince("{Geo}Province_"); // 省key前缀
const std::string kKeyPrefixGeoCity("{Geo}City_"); // 市key前缀
const std::string kKeyPrefixGeoDistrict("{Geo}District_"); // 区key前缀

const std::string kKeyPrefixGeoOnlineCountry("{Geo}OnlineCountry_"); // 国家key前缀
const std::string kKeyPrefixGeoOnlineProvince("{Geo}OnlineProvince_"); // 省key前缀
const std::string kKeyPrefixGeoOnlineCity("{Geo}OnlineCity_"); // 市key前缀
const std::string kKeyPrefixGeoOnlineDistrict("{Geo}OnlineDistrict_"); // 区key前缀

const std::string kKeyPrefixPlayerAcInfo("ACINFO_") ; // 活动额外数据
const std::string kKeyPrefixExtrPhotos("EPHOTO_"); // 区key前缀
//竞技场相关
const std::string kKeyArenaInfo("{Arena}Info");		//竞技场信息前缀
const std::string kKeyArenaInfoLast("ArenaInfoLast");		//竞技场信息前缀(上期)
const std::string kKeyArenaInfoYesterday("ArenaInfoYesterday");	//竞技场信息前缀（昨天）
const std::string kKeyArenaRegon("{Arena}Region");	//竞技场排名前缀
const std::string kKeyArenaRegonYestorday("ArenaRegionYestorday_");	//昨日竞技场排名前缀
const std::string kKeyArenaDefInfo("ArenaDef_");	//竞技场防守阵容
const std::string kKeyArenaSimpleInfo("ArenaSimple_");	//竞技场防守攻击阵容概要
const std::string kKeyArenaReplay("ArenaReplay_");	//竞技场战报
const std::string kKeyArenaReplayList("ArenaReplayList_");	//竞技场战报id列表
const std::string kKeyArenaLock("ArenaLock");		//竞技场锁定
const std::string kKeyArenaWeekendLock("ArenaWeekendLock");		//竞技场周末锁定
const std::string kKeyArenaYesterdayLock("ArenaYesterdayLock");		//竞技场更新昨日段位锁定
const std::string kKeyArenaWeekendAward("ArenaWeekendAward");		//竞技场奖励
const std::string kKeyArenaWeekendRankAward("ArenaWeekendRankAward");		//竞技场排名奖励
const std::string kKeyPlayerTemp("PlayerTemp_");		//用户临时数据
const std::string kKeyArenaDefTarFailCnt("ArenaTarFailCnt_");	//对手主动挑战失连续败次数
const std::string kKeyArenaLastWeekendTime("ArenaLastWeekend");	//上次发放周末奖励时间
const std::string kKeyArenaUpdateYesterdayTime("ArenaUpdateYesterdayTime");	//上次更新昨日段位时间
const std::string kKeyArenaTop30Flag("ArenaTop30");	//竞技场前30名记录标记
const std::string kKeyArenaBtlReplayLock("ArenaBtlReplayLock");		//竞技场回放锁

extern const std::string kKeyBehaviourRankFlag("BehaviourRankFlag");	//行为排名锁
extern const std::string kKeyBehaviourRankAward("BehaviourRankAward");	//行为排名奖励
extern const std::string kKeyBehaviourRank("BehaviourRank");			//行为排前缀

const std::string kRankingAwardWorking("RankingAward");
const std::string kRankingAwardDone("RankingAwardDone");
const std::string kRankingAwardLastTime("RankingAwardLastTime");
const std::string kRankingAwardSvrId("RankingAwardSvrId");
const std::string kGSOnlineInfo("GSOnlineInfo");		//game svr 在线用户信息

//战队相关
const std::string kKeyCorpsNameIndex("CPSINDEX");
const std::string kKeyCorpsGuid("CPSGUID");

// 小队数据
const std::string kKeyCorpsUniKey("CPSDATA");
const std::string kKeyPrefixGarment("Garment"); // 玩家时装数据前缀

// 时装数据
const std::string kKeyGar_Uid("gar_auto_uid");		// 时装uid
const std::string kKeyGar_SuitUse("gar_using_suit_id");	// 正在使用的套装id
const std::string kKeyGar_Garments("gar_garments");	// 时装信息
const std::string kKeyGar_GarSuits("gar_suits");	// 时装套装信息
const std::string kKeyGar_GarPics("gar_pics");	// 时装图鉴信息

// 资源占用
const std::string kKeyCapResBasicInfo("CRSBInfo");
const std::string kKeyCapResLastStoreInfo("CRSLastStore");

//const std::string kKeyCapAIList("CapAIList");

const std::string kKeyCapAITime("CapAITime");
const std::string kKeyCapAINum("CapAINum");
const std::string kKeyCapAICurNum("CapAICurNum");
const std::string kKeyCapAINumLefted("CapAINumLefted");
const std::string kKeyCapAICapRes("CapAICapRes");
const std::string kKeyCapResRegionTotal("CapResRegionTotal");
const std::string kKeyCapResPlayerCurRes("CapResPlayerCurRes");
//const std::string kKeyCapRegionLock("CapRegionLock");
//const std::string kKeyCandidateResPref("CandidateRes_");
//const std::string kKeyProtectRes("ProtectRes");

//const std::string kKeyResDis("ResDis");
//const std::string kResRegionHash("ResRegionHash");

const std::string kKeyAcTask("AcTaskInfo");

const std::string kKeyMaintenanceState("MaintenanceState");	// 维护状态

const std::string kKeyGmActOpenState("GmActivityOpenState"); // gm活动开关状态

const std::string kKeyRandFemaleName("RandFemaleName");
const std::string kKeyRandMaleName("RandMaleName");

const std::string kKeyRecruitInviteCode("RecruitInviteCode");

const std::string kKeyPrefixCacheInfo("CacheInfo");
const std::string kCacheFieldCollectionCnt("CollectionCnt");
const std::string kCacheFieldCollectionPoint("CollectionPoint");

const std::string kKeyPlayerDataTouchInfo("PDTouch");

const std::string kKeyLockCorpPos("LockCorpPos");		// 公会位置信息锁
const std::string kKeyArMonDailyTask("ArMonDailyTask");		// ar扫脸每日每宠扫到次数

const std::string kCustomStageLog("custom_stage_log");
const std::string kKeyActivityGlobalData("act_global_data"); // 活动公共数据
const std::string kKeyPavilionLogPrefix("pavilion_log");
const std::string kKeyPavilionExtraDataPrefix("pavilion_extra");


const std::string kKeyChannelGiftUnKey("kKeyChannelGiftUnKey");
const std::string kKeyChannelUserIdKey("kKeyChannelUserIdKey");
const std::string kKeyChannelGiftInfo("kKeyChannelGiftInfo");

std::string TransIP(uint32_t ip)
{
	struct in_addr addr;
	addr.s_addr = ip;
	return inet_ntoa(addr);
}
