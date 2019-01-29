/*
 * PayData.h
 *
 *  Created on: 2017年8月29日
 */

#ifndef DIGIMON_SRC_HANDLERS_PAYDATA_H_
#define DIGIMON_SRC_HANDLERS_PAYDATA_H_

#include <string>
#include <unordered_map>

#include "../proto/ErrCode.pb.h"
#include "../proto/Core.pb.h"
#include "../proto/db.pb.h"
#include "../core/sqlconnection.h"
#include "MsgHandler.h"
// 支付相关
class PayData
	: public MsgHandler
{
public:
	PayData();
	ErrCodeType	GetPayOrder(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);

	void Init() override;

private:
	ErrCodeType	finishPayOrder(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType	genPayOrder(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType	setPaidOrder(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType	getRebateGemsInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType	allRebateGemsInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType	getRebateGemsReward(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType onGmPay(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);

private:
	static void toOrdersData(std::unordered_map<std::string, std::string>& m, google::protobuf::RepeatedPtrField<db::OrderInfo>* orders);

private:
	std::string dbPrefix_;
	std::string tblPrefix_;
};

extern PayData gPayData;

#endif /* DIGIMON_SRC_HANDLERS_PAYDATA_H_ */
