/*
 * OfflineReq.h
 *
 *  Created on: 2017年7月15日
 */

#ifndef DIGIMON_SRC_HANDLERS_OFFLINEREQ_H_
#define DIGIMON_SRC_HANDLERS_OFFLINEREQ_H_

#include <string>
#include <unordered_map>

#include "../proto/ErrCode.pb.h"

// 离线操作处理相关功能
class OfflineReq {
public:
	OfflineReq();

private:
	ErrCodeType addOfflineReq(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType getOfflineReq(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType delOfflineReq(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);

	ErrCodeType getCarousel(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType addCarousel(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType delCarousel(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);

};

#endif /* DIGIMON_SRC_HANDLERS_OFFLINEREQ_H_ */
