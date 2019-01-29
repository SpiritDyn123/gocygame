/*
 * GroupData.h
 *
 *  Created on: 2017年4月11日
 *      Author: zhangxuan
 */

#ifndef DIGIMON_SRC_HANDLERS_GROUPDATA_H_
#define DIGIMON_SRC_HANDLERS_GROUPDATA_H_

class GroupData {
public:
	GroupData();
private:
	ErrCodeType getGroupData(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType setGroupData(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
private:
	static std::string makeGroupKey(uint64_t guid);

private:
	static const std::string kGroupDataKeyPrefix;
	static const std::string kGroupDataKey;
};
#endif /* DIGIMON_SRC_HANDLERS_GROUPDATA_H_ */
