/*
 * HatchData.h
 *
 *  Created on: 2017年3月2日
 *      Author: antigloss
 */

#ifndef DIGIMON_SRC_HANDLERS_HATCHDATA_H_
#define DIGIMON_SRC_HANDLERS_HATCHDATA_H_

#include <sstream>
#include "../proto/ErrCode.pb.h"

// 孵化相关功能
class HatchData {
public:
	HatchData();

private:
	ErrCodeType createHatchData(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType setHatchData(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType getHatchData(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType delHatchData(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	// 以上已作废
	ErrCodeType createHatchInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType setHatchInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType loadHatchData(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType delHatchInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType addHatchMsg(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType getHatchMsg(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType SetRedPacketMsg(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType SetBuildingLogMsg(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);




private:
	static std::string makeHatchKey(uint64_t guid)
	{
		std::ostringstream oss;
		oss << kKeyPrefixHatchData << guid;
		return oss.str();
	}

	static std::string makeHatchFiled(uint32_t egg_guid)
	{
		std::ostringstream oss;
		oss << egg_guid;
		return oss.str();
	}

	static void doGetHatchData(const SSProtoHead& h, uint64_t guid, google::protobuf::RepeatedPtrField<std::string>* data);
};

#endif /* DIGIMON_SRC_HANDLERS_HATCHDATA_H_ */
