/*===============================================================
* @Author: car
* @Created Time : 2017年06月26日 星期一 15时01分59秒
*
* @File Name: RankData.h
* @Description:
*
================================================================*/
#ifndef _DIGIMON_RANKDATA_H_
#define _DIGIMON_RANKDATA_H_

#include <sstream>
#include <string>
#include "../proto/ErrCode.pb.h"
#include "../proto/Core.pb.h"
#include "../proto/db.pb.h"
/*
更新分数
删除分数
拉取排行
根据分数获取名次
重新恢复排行榜
*/
extern const std::string kRankSplit;

class RankData {
public:
	enum SetScoreWay {
		kAdd			= 0,	
		kSetNoForce		= 1,	
		kSetByForce		= 2,
	};

	enum RankType {
		kCommon			= 1,
		kWeek			= 2,
		kDaily			= 3,
	};

public:
	RankData();
	// TODO
	void RestoreRank(); 

private:
	ErrCodeType getRankList(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType getRankByKey(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType updateRank(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType delRank(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType delGeoRankVec(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType clearRank(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);

public:
	static std::string MakeRankKey(const db::RankKey& rankKey);
	static ErrCodeType DoSetGeoRank(std::string& strPlayerID, double totalScore, double weekScore, double dailyScore, 
		const cs::GeoPos& geo, const std::string& rankName, bool common = true, bool week = true, bool daily = true);
	static ErrCodeType DoDelGeoRank(std::string& strPlayerID, const cs::GeoPos& geo, const std::string& rankName, bool common = true, bool week = true, bool daily = true);
	static ErrCodeType DoIncGeoRank(std::string& strPlayerID, int inc, const cs::GeoPos& geo, const std::string& rankName, bool common = true, bool week = true, bool daily = true);

private:
	static std::string GetRankName(int rankName, int gameRegion);
	static ErrCodeType GetFollowingRankList(db::GetRankListReq& req, db::GetRankListRsp& rsp);

private:
	static const std::string kGRankInfoKey;
	static const std::string kRankKey;
};

extern RankData rankData;



#endif // _DIGIMON_RANKDATA_H_
