/*
 * WishData.h
 *
 *  Created on: 2017年4月17日
 *      Author: antigloss
 */

#ifndef DIGIMON_SRC_HANDLERS_WISHDATA_H_
#define DIGIMON_SRC_HANDLERS_WISHDATA_H_

#include <string>
#include <unordered_map>

#include "../proto/ErrCode.pb.h"
#include "../proto/db.pb.h"
#include "RankData.h"

// 孵化相关功能
class WishData {
public:
	WishData();

public:
	void GetWishData(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);

private:
	ErrCodeType createWish(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType getMyWish(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType getWishes(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType giveWishingItem(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType decLeftWishItemCnt(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType delWish(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);

	ErrCodeType getWish_V1(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType setWish_V1(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType delWish_V1(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType addWishRecord(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType getWishRecord(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType addWishPopular(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType getWishPopular(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);

private:
	static void setGivenCntAndHistory(const std::unordered_map<std::string, std::string>& fields, cs::WishInfo* wishInfo);

public:
	static std::string GetWishRankKey(const std::string& rankName, uint32_t playerID);

public:
	static const std::string kFieldWishData; // 心愿数据
	static const std::string kFieldLeftCnt; // 剩余未领取数量
	static const std::string kFieldTotalGivenCnt; // 总共获赠数量
	static const std::string kFieldLastWishTime;
	static const std::string kFieldWishCnt;
	static const std::string kFieldPlayedWishGrid;
	static const std::string kFieldRecordFlag;

	static const std::string kScriptGiveWishItem;
	static const std::string kScriptTrimWishRecord;
};

extern WishData wishData;

#endif /* DIGIMON_SRC_HANDLERS_WISHDATA_H_ */
