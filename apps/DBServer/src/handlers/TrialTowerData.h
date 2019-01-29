/*
 * GroupData.h
 *
 *  Created on: 2017年4月11日
 *      Author: zhangxuan
 */

#ifndef DIGIMON_SRC_HANDLERS_TRIAL_TOWER_DATA_H_
#define DIGIMON_SRC_HANDLERS_TRIAL_TOWER_DATA_H_

class TrialTowerData {
public:
	TrialTowerData();
private:
	ErrCodeType pullBestPassRecords(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType tryUpdateRecords(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);

private:


private:
	static const std::string kTrialTowerDataKeyPrefix;
	static const std::string kTrialTowerDataKey;
};
#endif /* DIGIMON_SRC_HANDLERS_TRIAL_TOWER_DATA_H_ */
