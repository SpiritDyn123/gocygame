/*
* HomeDecorateData.h
*
*  Created on: 2017年7月15日
*      Author: antigloss
*/

#ifndef DIGIMON_SRC_HANDLERS_DECORATEDATA_H_
#define DIGIMON_SRC_HANDLERS_DECORATEDATA_H_

#include <sstream>
#include "../proto/ErrCode.pb.h"

//static std::string kDecorateSaveFiled("save_flag");
//static std::string kDecorateSkinFiled("skin");
//static std::string kDecorateLevelFiled("level");
//static std::string kDecorateMapInfoFiled("map_info");
//static std::string kDecorateBuildingInfoFiled("building_info");
static std::string kDecorateBaseInfoFiled("0");

// 家园装扮相关功能
class HomeDecorateData {
public:
	HomeDecorateData();
	int getHomeLevel(uint32_t TargetID);

private:
	ErrCodeType setHomeDecorateData(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType getHomeDecorateData(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);


public:
	static std::string makeDecorateKey(uint32_t playerId)
	{
		std::ostringstream oss;
		oss << kKeyPrefixHomeDecorateData << playerId;
		return oss.str();
	}
};
extern HomeDecorateData homeDecorateData;
#endif /* DIGIMON_SRC_HANDLERS_DECORATEDATA_H_ */
