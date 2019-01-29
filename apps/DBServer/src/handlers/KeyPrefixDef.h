/*
 * KeyDef.h
 *
 *  Created on: 2017年5月3日
 *      Author: antigloss
 */

#ifndef DIGIMON_SRC_HANDLERS_KEYPREFIXDEF_H_
#define DIGIMON_SRC_HANDLERS_KEYPREFIXDEF_H_

#include <ctime>
#include <string>
#include "../TimeUtils.h"

extern const std::string kKeyPrefixPlayerData; // PlayerData前缀
extern const std::string kKeyPrefixNickname; // 用于保证昵称唯一性的数据前缀
extern const std::string kKeyPrefixOfflineMsg; // 离线消息前缀
extern const std::string kKeyPrefixTodayLoginedPlayers; // 当日登录玩家前缀(过了新手引导的)
extern const std::string kKeyPrefixAllTodayLoginedPlayers; // 当日登录玩家前缀
extern const std::string kKeyPrefixAllTodayLoginedPlayersSnapshot; // 快照当日登陆玩家临时前缀
extern const std::string kKeyPrefixLogin;
extern const std::string kKeyPLayerGUID;

extern const std::string kKeyPrefixFollowing; // 关注列表前缀
extern const std::string kKeyPrefixFavFollowing; // 特别关注列表前缀
extern const std::string kKeyPrefixFollower; // 粉丝列表前缀
extern const std::string kKeyPrefixBlacklist; // 黑名单前缀
extern const std::string kKeyPrefixIntersection; // 互相关注前缀
extern const std::string kKeyPrefixFans; // 粉丝列表前缀

extern const std::string kKeyPrefixHatchData; // 孵化数据前缀
extern const std::string kKeyPrefixHatchMsg; // 孵化消息前缀

extern const std::string kKeyPrefixHomeDecorateData; // 家园装扮数据前缀
extern const std::string kKeyPrefixHomeResFactory;	// 家园资源生产
extern const std::string kKeyPrefixHomeResHire;	// 家园资源租赁

extern const std::string kKeyPrefixMailData; // 玩家邮件数据前缀
extern const std::string kKeyGlobalMailData; // 全局邮件数据key

extern const std::string kKeyPrefixWishData; // 心愿数据前缀
extern const std::string kKeyPrefixWishData_V1; // 心愿数据前缀V1
extern const std::string kKeyPrefixWishRecord; // 心愿数据前缀V1

extern const std::string kKeyPrefixActivityData; // 活动数据前缀
extern const std::string kKeyActivtiyDailyDeadTMData; // 活动每日失效时间

extern const std::string kKeyTrialTowerRecords;	// 修炼之塔最佳纪录 

// Fields
extern const std::string kBEDataFieldBanLogin; // BasicExtraData中的封停登录字段
extern const std::string kBEDataFieldBanChat; // BasicExtraData中的封停聊天字段

extern const std::string kKeyGsMember; // gs成员信息 用于房间置顶
extern const std::string kKeyGsRecRoom; // gs推荐房间信息
extern const std::string kKeyChannelSwitch; // 渠道开关
extern const std::string kMiscFieldBanLoginDevice; // 封禁的设备号
extern const std::string kMiscFieldBanChatDevice; // 封禁的设备号
extern const std::string kMiscFieldBanLoginIP; // 封禁的ip
extern const std::string kMiscFieldBanChatIP; // 封禁的ip
extern const std::string kKeyPrefixPlayerOfDevice; // 设备下账号
extern const std::string kKeyPrefixPlayerOfIP; // IP下账号
extern const std::string kMiscFieldDubiousDevice; // 建号数超标的设备
extern const std::string kMiscFieldDubiousIP; // 建号数超标的IP

extern const std::string kMiscFieldBanChannelPay; // 封禁充值的渠道
extern const std::string kMiscFieldBanChannelLogin; // 封禁登录的渠道
extern const std::string kMiscFieldBanChannelCreate; // 封禁登录的渠道

extern const std::string kMiscFieldOperationVer; // 运营数据版本号

extern const std::string kGameMGuide; // 男性游戏引导员
extern const std::string kGameFGuide; // 女性游戏引导员

extern const std::string kKeyPrefixPendingOrder; 	// 未完成订单
extern const std::string kKeyPrefixPaidOrder; 		// 已支付订单
extern const std::string kKeyPrefixFinishedOrder; 	// 已完成订单
extern const std::string kKeyPrefixDayPayOrder; 	// 今日生成订单自增序号
extern const std::string kKeyPrefixDayPaidOrderQueue; 	// 今日已支付订单队列
extern const std::string kMiscFieldBoughtGems; // 玩家购买的宝石数
extern const std::string kRebateBoughtGems; // 玩家充值返利购买的宝石数量
extern const std::string kKeyOrderGotBoughtGems; // 玩家是否已领充值返利宝石

extern const std::string kKeyPrefixRedPoint; // 红点提醒

extern const std::string kKeyDelPlayers; // 已经失效的号

extern const std::string kKeyPrefixOfflineReq; // 离线操作
extern const std::string kKeyPrefixSocialData; // 社交数据
extern const std::string kSocialFieldPopular; // 人气
extern const std::string kSocialFieldMomoPopular; // momo内嵌人气
extern const std::string kSocialFieldPopularLv; // 人气等级
extern const std::string kSocialFieldLove; // 爱心值
extern const std::string kSocialFieldWealth; // 财富值
extern const std::string kSocialFieldWeekWealth; // 周财富值
extern const std::string kSocialFieldDailyWealth; // 日财富值
extern const std::string kSocialFieldWealthLv; // 财富值等级
extern const std::string kSocialFieldMomoLove; // momo爱心值
extern const std::string kSocialFieldWeekPopular; // 周人气
extern const std::string kSocialFieldWeekMomoPopular; // 周momo内嵌人气
extern const std::string kSocialFieldDailyPopular; // 日人气
extern const std::string kSocialFieldWeekLove; // 周爱心值
extern const std::string kSocialFieldWeekMomoLove; // 周momo爱心值
extern const std::string kSocialFieldResetWeekTime; // 上次重置周数据时间
extern const std::string kSocialFieldResetDailyTime; // 上次重置日数据时间
extern const std::string kSocialFieldLikedCnt; // 被点赞次数
extern const std::string kSocialFieldGsMember; // GsMember信息 用于房间置顶
extern const std::string kSocialFieldStrangerCnt; // 社交交互陌生人访问次数
extern const std::string kSocialFieldLookedCnt; // 社交交互次数
extern const std::string kSocialFieldVisitedCnt; // 社交交互家园访问次数
extern const std::string kKeyPrefixMomoFollowing; // 我对关注的人的momo友好度
extern const std::string kKeyPrefixMomoFollower; // 粉丝对我的momo友好度
extern const std::string kKeyPrefixWorship; // 膜拜前缀
extern const std::string kKeyPrefixStranger; // 陌生人首次拜访时间
extern const std::string kKeyPrefixInteractTime; // 最后一次互动时间
extern const std::string kSocialFieldCheckInteractTime; // 最后查看互动记录时间
extern const std::string kKeyPrefixInteractInfo; // 最后一次互动行为记录

extern const std::string kKeyPrefixFailedSyncMomoItem; // 内嵌端同步失败请求前缀

extern const std::string kKeyPrefixClearRankProgress; // 榜单结算进度
extern const std::string kRankFieldArenaRank; // Arena榜的名字
extern const std::string kRankFieldPavilionRank; // 道馆榜的名字

extern const std::string kSocialFieldTodayPopular; // 人气
extern const std::string kSocialFieldYesterdayPopular; // 人气
extern const std::string kSocialFieldLastPopularTime; // 人气

extern const std::string kKeyPlayerRecruitData; // 招募
extern const std::string kRecruitFieldCode;
extern const std::string kRecruitFieldMyRecruitPlid;
extern const std::string kRecruitFieldMyRecruitNick;
extern const std::string kRecruitFieldLoginScore;
extern const std::string kRecruitFieldAdventureScore;
extern const std::string kRecruitFieldBtlGrpScore;
extern const std::string kRecruitFieldLevelScore;

extern const std::string kKeyPrefixGiftItem;			// 收到的礼物列表
extern const std::string kGiftItemFieldGemValue;		// 收到的礼物可兑换彩钻的价值
extern const std::string kGiftItemFieldGemHistroyValue;	// 收到的礼物可兑换彩钻的价值(历史总和)
extern const std::string kKeyPrefixGiftGivenRecord;		// 收到的礼物价值列表
extern const std::string kKeyPrefixGiftGivenRank;		// 收到的礼物价值排行

// 地理位置
extern const std::string kKeyPrefixGeoCountry; // 国家key前缀
extern const std::string kKeyPrefixGeoProvince; // 省key前缀
extern const std::string kKeyPrefixGeoCity; // 市key前缀
extern const std::string kKeyPrefixGeoDistrict; // 区key前缀

extern const std::string kKeyPrefixGeoOnlineCountry; // 在线国家key前缀
extern const std::string kKeyPrefixGeoOnlineProvince; // 在线省key前缀
extern const std::string kKeyPrefixGeoOnlineCity; // 在线市key前缀
extern const std::string kKeyPrefixGeoOnlineDistrict; // 在线区key前缀
extern const std::string kKeyPrefixExtrPhotos; // 额外的照片

extern const std::string kKeyPrefixPlayerAcInfo;	// 活动额外数据

extern const std::string kKeyCorpsBasicInfo;
extern const std::string kKeyCorpsNameIndex;
extern const std::string kKeyCorpsGuid;

extern const std::string kKeyArenaInfo;		//竞技场信息前缀
extern const std::string kKeyArenaInfoLast;		//竞技场信息前缀（上期）
extern const std::string kKeyArenaInfoYesterday;	//竞技场信息前缀（昨天）
extern const std::string kKeyArenaRegon;	//竞技场排名分区前缀
extern const std::string kKeyArenaRegonYestorday;	//昨日竞技场排名前缀
extern const std::string kKeyArenaDefInfo;	//竞技场防守阵容
extern const std::string kKeyArenaSimpleInfo;	//竞技场防守攻击阵容概要
extern const std::string kKeyArenaReplay;	//竞技场战报
extern const std::string kKeyArenaReplayList;	//竞技场战报id列表
extern const std::string kKeyArenaLock;		//竞技场锁定
extern const std::string kKeyArenaWeekendLock;		//竞技场周末锁定
extern const std::string kKeyArenaYesterdayLock;		//竞技场更新昨日段位锁定
extern const std::string kKeyArenaWeekendAward;			//竞技场奖励
extern const std::string kKeyArenaWeekendRankAward;		//竞技场排名奖励
extern const std::string kKeyPlayerTemp;		//用户临时数据
extern const std::string kKeyArenaDefTarFailCnt;	//对手主动挑战失连续败次数
extern const std::string kKeyArenaLastWeekendTime;	//上次发放周末奖励时间
extern const std::string kKeyArenaUpdateYesterdayTime;	//上次更新昨日段位时间
extern const std::string kKeyArenaTop30Flag;	//竞技场前30名记录标记
extern const std::string kKeyArenaBtlReplayLock;		//竞技场回放锁

extern const std::string kKeyBehaviourRankFlag;		//行为排名锁
extern const std::string kKeyBehaviourRankAward;	//行为排名奖励
extern const std::string kKeyBehaviourRank;			//行为排前缀

//周排行榜
extern const std::string kRankingAwardWorking;		//奖励发放中标记
extern const std::string kRankingAwardDone;			//已发放奖励的排行榜名列表
extern const std::string kRankingAwardLastTime;		//上次发奖时间
extern const std::string kRankingAwardSvrId;		//触发发奖的GSID
extern const std::string kGSOnlineInfo;				//game svr 在线用户信息

// 小队数据
extern const std::string kKeyCorpsUniKey;
extern const std::string kKeyPrefixGarment;			// 玩家时装数据前缀

// 时装数据
extern const std::string kKeyGar_Uid;		// 时装uid
extern const std::string kKeyGar_SuitUse;	// 正在使用的套装id
extern const std::string kKeyGar_Garments;	// 时装信息
extern const std::string kKeyGar_GarSuits;	// 时装套装信息
extern const std::string kKeyGar_GarPics;	// 时装图鉴信息

// 资源占用
extern const std::string kKeyCapResBasicInfo;
extern const std::string kKeyCapResLastStoreInfo;

//extern const std::string kKeyCapAIList;			// list

extern const std::string kKeyCapAITime;			// 段位刷新时间 
//extern const std::string kKeyCapAINum;			// 今日总ai数 
extern const std::string kKeyCapAICurNum;		// 已经生成ai数
extern const std::string kKeyCapAINumLefted;    // 剩余ai数
extern const std::string kKeyCapAICapRes;		// 当前ai可分配资源
extern const std::string kKeyCapResRegionTotal;		// 今日预流通的资源数
extern const std::string kKeyCapResPlayerCurRes;	// 玩家所占的使用资源数
//extern const std::string kKeyCapRegionLock;		// 资源锁
//extern const std::string kKeyCandidateResPref;	// 候选资源
//extern const std::string kKeyProtectRes;		// 受保护的列表
//extern const std::string kKeyResDis;			// 资源分布列表
//extern const std::string kResRegionHash;		// 所在的资源分布段位位子

extern const std::string kKeyAcTask;			// 玩家活动任务数据

extern const std::string kKeyMaintenanceState;	// 维护状态

extern const std::string kKeyGmActOpenState;	// gm活动开关状态

extern const std::string kKeyRandFemaleName;	// 随机女名字
extern const std::string kKeyRandMaleName;		// 随机男名字

extern const std::string kKeyRecruitInviteCode;

// cache
extern const std::string kKeyPrefixCacheInfo;
extern const std::string kCacheFieldCollectionCnt;
extern const std::string kCacheFieldCollectionPoint;

// 记录玩家数据某个key什么时候被访问啊之类的，只为统计
extern const std::string kKeyPlayerDataTouchInfo;

// lockkey
extern const std::string kKeyLockCorpPos;
extern const std::string kKeyArMonDailyTask;

extern const std::string kCustomStageLog;
extern const std::string kKeyActivityGlobalData;
extern const std::string kKeyPavilionLogPrefix;
extern const std::string kKeyPavilionExtraDataPrefix;


// 渠道礼包唯一key
extern const std::string kKeyChannelGiftUnKey;
extern const std::string kKeyChannelUserIdKey;
extern const std::string kKeyChannelGiftInfo;

//----non-prefix key define--------------
const std::string kEggHashKeyPrefix("_E"); // 这两个HashKey的内容不能改，如果要改，必须同步修改toPlayerData！！
const std::string kMonHashKeyPrefix("_M");
const std::string kAccountInfo("account_info");
const std::string kOnlineStatInfo("online_stat_info");
const std::string kPlayerAttr("attr_info");
const std::string kMonsterBag("monster_bag");
const std::string kRegInfo("reg_info");
const std::string kMonsterFormation("monster_formation");
const std::string kInHomeItems("in_home_items");
const std::string kGroupInfo("group_info");
const std::string kHatchInfo("hatch_info");
const std::string kAdventureInfo("adventure_info");
const std::string kLoginSplit("#");
const std::string kActiveSocialInfo("active_social_info");
const std::string kLazySocialInfo("lazy_social_info");
const std::string kHomeBuffInfo("homebuff_info");
const std::string kCollectionInfo("collection_info");
const std::string kPlayerDigivice("digivice_info");
const std::string kPlayerKvInfo("kv_info");
const std::string kPlayerArFace("ar_face");
const std::string kGarWearing("gar_wearing"); // 玩家身上新的穿着时装信息，都从这里取
const std::string kArenaInfo("arena_info");
const std::string kPlayerSettings("player_settings");
const std::string kHomeBuildings("home_buildings");
const std::string kTrialTower("trial_tower_data");
const std::string kPlayerGameRegion("game_region");

const std::string kChannelNameMomo("momo");
const std::string kDailyTask("daily_task_info");
const std::string kPavilion("pavilion_data");

inline std::string makeLoginKey(const std::string& accountType, const std::string& accountId)
{
	return kKeyPrefixLogin + accountType + kLoginSplit + accountId;
}

inline std::string makeTodayLoginedPlayersKey(int gameRegion)
{
	return kKeyPrefixTodayLoginedPlayers + "_rg" + std::to_string(gameRegion) + '_' + std::to_string(time(0) / 86400);
}

inline std::string makeAllTodayLoginedPlayersKey(int gameRegion)
{
	return kKeyPrefixAllTodayLoginedPlayers + "_rg" + std::to_string(gameRegion) + '_' + GetTodayStr();
}

inline std::string makeAllYesterdayLoginedPlayersKey(int gameRegion)
{
	return kKeyPrefixAllTodayLoginedPlayers + "_rg" + std::to_string(gameRegion) + '_' + GetYesterdayStr();
}

inline std::string makeFailedSyncMomoItemKey(uint32_t plid)
{
	return kKeyPrefixFailedSyncMomoItem + std::to_string(plid) + '}';
}

inline std::string makeFollowingKey(uint32_t plid)
{
	return kKeyPrefixFollowing + std::to_string(plid) + '}';
}

inline std::string makeFavFollowingKey(uint32_t plid)
{
	return kKeyPrefixFavFollowing + std::to_string(plid) + '}';
}

inline std::string makeFollowerKey(uint32_t plid)
{
	return kKeyPrefixFollower + std::to_string(plid) + '}';
}

inline std::string makeBlacklistKey(uint32_t plid)
{
	return kKeyPrefixBlacklist + std::to_string(plid) + '}';
}

inline std::string makeIntersectionKey(uint32_t plid)
{
	return kKeyPrefixIntersection + std::to_string(plid) + '}';
}

inline std::string makeFansKey(uint32_t plid)
{
	return kKeyPrefixFans + std::to_string(plid) + '}';
}

inline std::string makeWorshipKey(uint32_t plid)
{
	return kKeyPrefixWorship + std::to_string(plid) + '}';
}

inline std::string makeStrangerKey(uint32_t plid)
{
	return kKeyPrefixStranger + std::to_string(plid) + '}';
}

inline std::string makeInteractTimeKey(int type, uint32_t plid)
{
	return kKeyPrefixInteractTime + std::to_string(type) + '_' + std::to_string(plid) + '}';
}

inline std::string makeInteractInfoKey(int type, uint32_t plid)
{
	return kKeyPrefixInteractInfo + std::to_string(type) + '_' + std::to_string(plid) + '}';
}

inline std::string makeBasicExtraDataKey(const std::string& playerID)
{
	return "BEData{" + playerID + "}";
}

inline std::string makeBuildingLogKey(uint32_t plid)
{
	return "BDLOGData{" + std::to_string(plid) + "}";
}

inline std::string makeRedPacketKey(uint32_t plid)
{
	return "RedPacket{" + std::to_string(plid) + "}";
}


inline std::string makeRedptKey(uint32_t plid)
{
	return kKeyPrefixRedPoint + std::to_string(plid);
}

inline std::string makePlayerOfDeviceKey(const std::string& device)
{
	return kKeyPrefixPlayerOfDevice + device;
}

inline std::string makePlayerOfIPKey(const std::string& device)
{
	return kKeyPrefixPlayerOfIP + device;
}

inline std::string makePendingOrderKey(uint32_t plid)
{
	return kKeyPrefixPendingOrder + std::to_string(plid);
}

inline std::string makePaidOrderKey(uint32_t plid)
{
	return kKeyPrefixPaidOrder + std::to_string(plid);
}

inline std::string makeFinishedOrderKey(uint32_t plid)
{
	return kKeyPrefixFinishedOrder + std::to_string(plid);
}

inline std::string makeDayPayNoKey(time_t today)
{
	return kKeyPrefixDayPayOrder + std::to_string(today);
}

inline std::string makeDayPaidOrderQueueKey(std::string strToday)
{
	return kKeyPrefixDayPaidOrderQueue + strToday;
}

inline std::string makeOfflineReqKey(uint32_t plid)
{
	return kKeyPrefixOfflineReq + std::to_string(plid);
}

inline std::string makeGeoCountryKey(int country)
{
	return kKeyPrefixGeoCountry + std::to_string(country);
}

inline std::string makeGeoProvinceKey(int province)
{
	return kKeyPrefixGeoProvince + std::to_string(province);
}

inline std::string makeGeoCityKey(int province, int city)
{
	return kKeyPrefixGeoCity + std::to_string(province) + '_' + std::to_string(city);
}

inline std::string makeGeoDistrictKey(int province, int city, int district)
{
	return kKeyPrefixGeoDistrict + std::to_string(province) + '_' + std::to_string(city) + '_' + std::to_string(district);
}

inline std::string makeGeoOnlineCountryKey(int country)
{
	return kKeyPrefixGeoOnlineCountry + std::to_string(country);
}

inline std::string makeGeoOnlineProvinceKey(int province)
{
	return kKeyPrefixGeoOnlineProvince + std::to_string(province);
}

inline std::string makeGeoOnlineCityKey(int province, int city)
{
	return kKeyPrefixGeoOnlineCity + std::to_string(province) + '_' + std::to_string(city);
}

inline std::string makeGeoOnlineDistrictKey(int province, int city, int district)
{
	return kKeyPrefixGeoOnlineDistrict + std::to_string(province) + '_' + std::to_string(city) + '_' + std::to_string(district);
}

/*
inline std::string makeCorpsJoinKey(const std::string& corpsid)
{
	return "CPSJ{" + corpsid + "}"; 
}

inline std::string makeCorpsInviteKey(const std::string& corpsid)
{
	return "CPSI{" + corpsid + "}"; 
}

*/

// 家族的基本信息
inline std::string makeCorpsBasicKey(const std::string& corpsid)
{
	return "CPSB{" + corpsid + "}"; 
}

// 家族的成员信息
inline std::string makeCorpsPlayersKey(const std::string& corpsid)
{
	return "CPSP{" + corpsid + "}"; 
}

// 家族的科技信息
inline std::string makeCorpsSkillsKey(const std::string& corpsid)
{
	return "CPSSkills{" + corpsid + "}"; 
}

// 废弃
inline std::string makeCorpsMonsKey(const std::string& corpsid)
{
	return "CPSM{" + corpsid + "}"; 
}

// 玩家家族相关key
inline std::string makePlayersKey(const std::string& plid)
{
	return "PCPSKEY{" + plid + "}"; 
}

// 玩家申请家族key
inline std::string makePlayersJoinKey(const std::string& plid)
{
	return "PCPSJKEY{" + plid + "}"; 
}

// 玩家被邀请的key
inline std::string makePlayersInvKey(const std::string& plid)
{
	return "PCPSIKEY{" + plid + "}"; 
}

// 玩家家族任务
inline std::string makePlayerCorpsTask(const std::string& plid)
{
	return "PCPSTASK{" + plid + "}";
}

inline std::string makePlayerBetaKey(const std::string& plid)
{
	return "BETA_ASSIST{" + plid + "}";
}

inline std::string makePlayerExAcTask(const std::string& plid)
{
	return "ExAcTASK{" + plid + "}";
}

inline std::string makePlayerActivityTask(const std::string& plid)
{
	return "ActivityTASK{" + plid + "}";
}

inline std::string makePlayerFExAcTask(const std::string& plid)
{
	return "ExAcFTASK{" + plid + "}";
}

inline std::string makeCorpsLogsKey(const std::string& key)
{
	return "CPSLOG{" + key + "}";
}

inline std::string makeCapResLogsKey(const std::string& key)
{
	return "CRSLOG{" + key + "}";
}

inline std::string makeCapResKey(const std::string& key)
{
	return "CRSINFO{" + key + "}";
}

inline std::string makeAICapResKey(const std::string& key)
{
	return "CRSAIINFO{" + key + "}";
}


inline std::string makeCorpsAllKey(const std::string& channel)
{
	if (channel == "0") {
		return "CPSLOCALALL"; 
	} else {
		return "CPSLOCALALL_" + channel;
	}
}

inline std::string makeCorpsProvinceKey(const std::string& key, const std::string& channel)
{
	if (channel == "0") {
		return "CPSLOCALPRO{" + key + "}"; 
	} else {
		return "CPSLOCALPRO{" + channel + "_" + key + "}";
	}
}

inline std::string makeCorpsCityKey(const std::string& key, const std::string& key1, const std::string& channel)
{
	if (channel == "0") {
		return "CPSLOCALCITY{" + key + "_" + key1 + "}"; 
	} else {
		return "CPSLOCALCITY{" + channel + "_" + key + "_" + key1 + "}";
	}

}

inline std::string makeTodayCandidateResKey(uint32_t game_region, uint32_t region) 
{
	if (game_region == 0) {
		return "CandidateRes_" + std::to_string(region);
	} else {
		return "CandidateRes_" + std::to_string(game_region) +"_"+ std::to_string(region); 
	}

}

inline std::string makeResRegionHash(uint32_t game_region) 
{
	if (game_region == 0) {
		return "ResRegionHash";
	} else {
		return "ResRegionHash" + std::to_string(game_region); 
	}

}


inline std::string makeCapAIHash(uint32_t game_region)
{
	if (game_region == 0) {
		return "CapAIHash";
	} else {
		return "CapAIHash" + std::to_string(game_region);
	}
}

inline std::string makeCapRegionLock(uint32_t game_region, uint32_t region)
{
	if (game_region == 0) {
		return "CapRegionLock" + std::to_string(region);
	} else {
		return "CapRegionLock" + std::to_string(game_region) + "_" + std::to_string(region);
	}
}

inline std::string makeCapRegionDis(uint32_t game_region, uint32_t region)
{
	if (game_region == 0) {
		return "ResDis" + std::to_string(region);
	} else {
		return "ResDis" + std::to_string(game_region) + "_" + std::to_string(region);
	}
}

inline std::string makeProtectRes(uint32_t game_region, uint32_t region)
{
	if (game_region == 0) {
		return "ProtectRes" + std::to_string(game_region);
	} else {
		return "ProtectRes" + std::to_string(game_region) + "_" + std::to_string(region);
	}
}


inline std::string makePlayerArFaceID()
{
	return "PlayersARFaceID";
}


inline std::string makeBetaPhoneKey()
{
	//return "DigimonBetaSubscribe";
	return "DigimonBetaSubscribeEx";
}

inline std::string makeBetaPhoneOfficialKey()
{
	return "DigimonBetaSubscribeOfficial";
}

inline std::string makePlayerHutKey(const std::string& pid)
{
	return "HutInfo{" + pid + "}";
}

inline std::string makeGarmentKey(const std::string& pid)
{
	return "GarmentNewInfo{" + pid + "}";
}

inline std::string makePlayerCacheKey(uint32_t plid)
{
	return kKeyPrefixCacheInfo + std::to_string(plid);
}

inline std::string makePlayerFaceARKey(const std::string& pid)
{
	return "PARFACE" + pid;
}

inline std::string makePlayerFaceARPersonTmKey(const std::string& person)
{
	return person + "_tm";
}

inline std::string makeTrialTowerRecordsDataKey(int gameRegion)
{
	return kKeyTrialTowerRecords+"_"+std::to_string(gameRegion);
}

inline std::string makeRecruitDataKey(uint32_t plid)
{
	return kKeyPlayerRecruitData + std::to_string(plid);
}

inline std::string makePavilionLogKey(uint32_t plid)
{
	return kKeyPavilionLogPrefix + "_" + std::to_string(plid);
}

inline std::string makePavilionExtraDataKey(uint32_t plid)
{
	return kKeyPavilionExtraDataPrefix + "_" + std::to_string(plid);
}

std::string TransIP(uint32_t ip);

#endif /* DIGIMON_SRC_HANDLERS_KEYPREFIXDEF_H_ */
