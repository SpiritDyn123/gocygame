/*
 * GeoData.h
 *
 *  Created on: 2017年7月21日
 */

#ifndef DIGIMON_SRC_HANDLERS_GEODATA_H_
#define DIGIMON_SRC_HANDLERS_GEODATA_H_

#include <string>
#include <unordered_map>

#include "RankData.h"
#include "../proto/ErrCode.pb.h"
#include "../proto/Core.pb.h"
#include "../proto/db.pb.h"

extern const int kGeoCountryChina;

// 地理位置相关
class GeoData {
public:
	GeoData();

private:
	ErrCodeType updatePos(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType rmOnlinePos(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType getNeighborPos(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);

public:
	//static void GetGeoRankKeys(cs::GeoPos& geo, const std::string& rankName, std::vector<std::string>& keys, bool worldCommon, bool worldWeek);
	static void GetGeoRankKeys(RankData::RankType type, const cs::GeoPos& geo, const std::string& rankName, std::vector<std::string>& keys);
	static ErrCodeType GetGeoData(uint32_t plid, cs::GeoPos& geo);
	static std::string GetGeoRankStr(const cs::GeoPos& geo);

private:
	static std::string GetGeoWorldStr(const cs::GeoPos& geo, const std::string& rankName);
	static std::string GetGeoCountryStr(const cs::GeoPos& geo, const std::string& rankName);
	static std::string GetGeoProvinceStr(const cs::GeoPos& geo, const std::string& rankName);
	static std::string GetGeoCityStr(const cs::GeoPos& geo, const std::string& rankName);
	static std::string GetGeoDistrictStr(const cs::GeoPos& geo, const std::string& rankName);
};

extern GeoData gGeoData;

#endif /* DIGIMON_SRC_HANDLERS_GEODATA_H_ */
