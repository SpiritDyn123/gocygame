/*
 * GeoData.cc
 *
 *  Created on: 2017年7月21日
 */

#include <string>
#include "../proto/SvrProtoID.pb.h"
#include "../core/dispatcher.h"
#include "../core/redis_client.h"

#include "KeyPrefixDef.h"
#include "Arena.h"
#include "GeoData.h"
#include "RankData.h"
#include "PlayerData.h"
#include "../TimeUtils.h"

using namespace std;

GeoData gGeoData;

// const string kGeoCountryChina("中国");
// const string kGeoCountryOther("Other");
// const string kNoWhere("NoWhere");
const int kGeoCountryChina(100000);
const int kGeoCountryOther(900000);
// const int kNoWhere(0);
const string kGeoSplit("_");
const int kGeoNeighborCnt(100);

GeoData::GeoData()
{
	gMsgDispatcher.RegisterHandler(DBProtoUpdatePos, *this, &GeoData::updatePos, new cs::GeoPos, nullptr);
	gMsgDispatcher.RegisterHandler(DBProtoDelOnlinePos, *this, &GeoData::rmOnlinePos, new cs::GeoPos, nullptr);
	gMsgDispatcher.RegisterHandler(DBProtoGetNeighborPos, *this, &GeoData::getNeighborPos, new cs::CSGetNeighborPlayer, new cs::SCGeoDistList);
}

static const string kScriptSetGeoPos =
		"for i = 1, #KEYS do\n"
		"  redis.call('GEOADD', KEYS[i], ARGV[1], ARGV[2], ARGV[3])\n"
		"end\n"
		"return 0";

static const string kScriptRemoveGeoPos =
		"for i = 1, #KEYS do\n"
		"  redis.call('ZREM', KEYS[i], ARGV[1])\n"
		"end\n"
		"return 0";

ErrCodeType GeoData::updatePos(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, cs::GeoPos, req);
	string strPlayerID = to_string(h.TargetID);

	// 取玩家原地理位置
	cs::GeoPos orgGeo;
	ErrCodeType errCodeGeo = GetGeoData(h.TargetID, orgGeo);
	if (errCodeGeo) {
		return errCodeGeo;
	}
	// 取玩家人气、爱心值
	/*long long totalPopular, totalLove, weekPopular, weekLove;
	ErrCodeType errCodePopLove = PlayerData::GetPopAndLove(strPlayerID, &totalPopular, &totalLove, &weekPopular, &weekLove);
	if (errCodePopLove) {
		return errCodePopLove;
	}*/
	db::SimpleSocialInfo sInfo;
	ErrCodeType retCode = PlayerData::GetSocialSimpleInfo(strPlayerID, sInfo);
	if (retCode) {
		return retCode;
	}

	// 删除原位置
	if (orgGeo.has_country_code()) { // 原来有位置
		vector<string> orgGeoKeys;
		vector<string> orgGeoArgs = { strPlayerID };
		orgGeoKeys.emplace_back(makeGeoCountryKey(orgGeo.country_code()));
		if (orgGeo.country_code() == kGeoCountryChina) {
			orgGeoKeys.emplace_back(makeGeoProvinceKey(orgGeo.province_code()));
			orgGeoKeys.emplace_back(makeGeoCityKey(orgGeo.province_code(), orgGeo.city_code()));
			orgGeoKeys.emplace_back(makeGeoDistrictKey(orgGeo.province_code(), orgGeo.city_code(), orgGeo.district_code()));
		}

		ScopedReplyPointer replyDel = gRedis->eval(kScriptRemoveGeoPos, &orgGeoKeys, &orgGeoArgs);
		CHECK_REPLY_EC(replyDel, REDIS_REPLY_INTEGER, atoi(strPlayerID.c_str()));
	}

	// 设置新位置
	vector<string> newGeoKeys;
	vector<string> newGeoArgs = { to_string(req.longitude()), to_string(req.latitude()), strPlayerID };
	newGeoKeys.emplace_back(makeGeoCountryKey(req.country_code()));
	// newGeoKeys.emplace_back(makeGeoOnlineCountryKey(req.country_code()));
	if (req.country_code() == kGeoCountryChina) {
		newGeoKeys.emplace_back(makeGeoProvinceKey(req.province_code()));
		newGeoKeys.emplace_back(makeGeoCityKey(req.province_code(), req.city_code()));
		newGeoKeys.emplace_back(makeGeoDistrictKey(req.province_code(), req.city_code(), req.district_code()));
	}

	ScopedReplyPointer replySet = gRedis->eval(kScriptSetGeoPos, &newGeoKeys, &newGeoArgs);
	CHECK_REPLY_EC(replySet, REDIS_REPLY_INTEGER, atoi(strPlayerID.c_str()));

	// 从原位置榜单中删除并设置新位置榜
	double timeTail = GetTimeTail();
	if (sInfo.total_popular() > 0 && !gPlayerData.IsGsMember(h.TargetID)) {
		RankData::DoDelGeoRank(strPlayerID, orgGeo, kSocialFieldPopular);
		RankData::DoSetGeoRank(strPlayerID, (double)sInfo.total_popular() + timeTail, (double)sInfo.week_popular() + timeTail, 
			(double)sInfo.daily_popular() + timeTail, req, kSocialFieldPopular);
	}
	if (sInfo.total_wealth() > 0) {
		RankData::DoDelGeoRank(strPlayerID, orgGeo, kSocialFieldWealth);
		RankData::DoSetGeoRank(strPlayerID, (double)sInfo.total_wealth() + timeTail, (double)sInfo.week_wealth() + timeTail,
			(double)sInfo.daily_wealth() + timeTail, req, kSocialFieldWealth);
	}

	RankData::DoDelGeoRank(strPlayerID, orgGeo, kRankFieldArenaRank, false, true, false);

	// cs::ArenaInfo arena;
	// gArena.GetInfo(h, nullptr, &arena);
	ErrCodeType errCodeUpdateRank = gArena.UpdateArenaRank(h.TargetID, 0, req.game_region());
	if (errCodeUpdateRank) {
		return errCodeUpdateRank;
	}
	return ErrCodeSucc;
}

ErrCodeType GeoData::rmOnlinePos(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, cs::GeoPos, req);
	string strPlayerID = to_string(h.TargetID);

	vector<string> geoKeys;
	vector<string> geoArgs = { strPlayerID };

	geoKeys.emplace_back(makeGeoOnlineCountryKey(req.country_code()));
	if (req.country_code() == kGeoCountryChina) {
		geoKeys.emplace_back(makeGeoOnlineProvinceKey(req.province_code()));
		geoKeys.emplace_back(makeGeoOnlineCityKey(req.province_code(), req.city_code()));
		geoKeys.emplace_back(makeGeoOnlineDistrictKey(req.province_code(), req.city_code(), req.district_code()));
	}
	ScopedReplyPointer reply = gRedis->eval(kScriptRemoveGeoPos, &geoKeys, &geoArgs);
	CHECK_REPLY_EC(reply, REDIS_REPLY_INTEGER, atoi(strPlayerID.c_str()));

	return ErrCodeSucc;
}

ErrCodeType GeoData::getNeighborPos(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, cs::CSGetNeighborPlayer, req);
	REAL_PROTOBUF_MSG(outMsg, cs::SCGeoDistList, rsp);

	vector<vector<string>> result;

	string keyCountry;
	string keyProvince;
	string keyCity;
	string keyDistrict;
	
	if (req.online()) {
		keyCountry = makeGeoOnlineCountryKey(req.geo_pos().country_code());
		keyProvince = makeGeoOnlineProvinceKey(req.geo_pos().province_code());
		keyCity = makeGeoOnlineCityKey(req.geo_pos().province_code(), req.geo_pos().city_code());
		keyDistrict = makeGeoOnlineDistrictKey(req.geo_pos().province_code(), req.geo_pos().city_code(), req.geo_pos().district_code());
	} else {
		keyCountry = makeGeoCountryKey(req.geo_pos().country_code());
		keyProvince = makeGeoProvinceKey(req.geo_pos().province_code());
		keyCity = makeGeoCityKey(req.geo_pos().province_code(), req.geo_pos().city_code());
		keyDistrict = makeGeoDistrictKey(req.geo_pos().province_code(), req.geo_pos().city_code(), req.geo_pos().district_code());
	}

	if (req.geo_pos().country_code() == kGeoCountryChina) {
		if (!gRedis->georadius(keyDistrict, req.geo_pos().longitude(), req.geo_pos().latitude(), 5, result)) {
			WARN_LOG("georadius failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
			return ErrCodeDB;
		}
		if (result.size() < kGeoNeighborCnt) {
			if (!gRedis->georadius(keyCity, req.geo_pos().longitude(), req.geo_pos().latitude(), 25, result)) {
				WARN_LOG("georadius failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
				return ErrCodeDB;
			}
		}
		if (result.size() < kGeoNeighborCnt) {
			if (!gRedis->georadius(keyProvince, req.geo_pos().longitude(), req.geo_pos().latitude(), 100, result)) {
				WARN_LOG("georadius failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
				return ErrCodeDB;
			}
		}
		if (result.size() < kGeoNeighborCnt) {
			if (!gRedis->georadius(keyCountry, req.geo_pos().longitude(), req.geo_pos().latitude(), 300, result)) {
				WARN_LOG("georadius failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
				return ErrCodeDB;
			}
		}
	} else {
		if (!gRedis->georadius(keyCountry, req.geo_pos().longitude(), req.geo_pos().latitude(), 300, result)) {
			WARN_LOG("georadius failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
			return ErrCodeDB;
		}
	}

	for (auto ret : result) {
		auto geo = rsp.add_geo_list();
		geo->set_player_id(atoi(ret[0].c_str()));
		geo->set_dist(atof(ret[1].c_str()));
		if (rsp.geo_list_size() >= kGeoNeighborCnt) {
			break;
		}
	}

	return ErrCodeSucc;
}

//--------------------------------------------------------------
/*void GeoData::GetGeoRankKeys(cs::GeoPos& geo, const std::string& rankName, std::vector<std::string>& keys, bool worldCommon, bool worldWeek)
{
	keys.clear();
	bool inChina = geo.country_code() == kGeoCountryChina;
	int countryCode = inChina ? kGeoCountryChina : kGeoCountryOther;
	stringstream ssRankWorld, ssRankCountry, ssRankProvince, ssRankCity, ssRankDistrict;
	ssRankWorld << rankName;
	ssRankCountry << rankName << kGeoSplit << countryCode;
	ssRankProvince << rankName << kGeoSplit << countryCode;
	ssRankCity << rankName << kGeoSplit << countryCode;
	ssRankDistrict << rankName << kGeoSplit << countryCode;
	if (inChina) {
		ssRankProvince << kGeoSplit << geo.province_code();
		ssRankCity << kGeoSplit << geo.province_code() << kGeoSplit << geo.city_code();
		ssRankDistrict << kGeoSplit << geo.province_code() << kGeoSplit << geo.city_code() << kGeoSplit << geo.district_code();
	}

	db::RankKey rankWorldObj, rankCountryObj, rankProvinceObj, rankCityObj, rankDistrictObj;
	db::RankKey rankWorldWeekObj, rankCountryWeekObj, rankProvinceWeekObj, rankCityWeekObj, rankDistrictWeekObj;
	db::RankKey rankWorldDailyObj, rankCountryDailyObj, rankProvinceDailyObj, rankCityDailyObj, rankDistrictDailyObj;
	// 永久总榜Key
	if (worldCommon) {
		rankWorldObj.set_rank_type(RankData::kCommon);
		rankWorldObj.set_rank_name(ssRankWorld.str());
		keys.emplace_back(RankData::MakeRankKey(rankWorldObj));

		rankCountryObj.set_rank_type(RankData::kCommon);
		rankCountryObj.set_rank_name(ssRankCountry.str());
		keys.emplace_back(RankData::MakeRankKey(rankCountryObj));

		rankProvinceObj.set_rank_type(RankData::kCommon);
		rankProvinceObj.set_rank_name(ssRankProvince.str());
		keys.emplace_back(RankData::MakeRankKey(rankProvinceObj));

		rankCityObj.set_rank_type(RankData::kCommon);
		rankCityObj.set_rank_name(ssRankCity.str());
		keys.emplace_back(RankData::MakeRankKey(rankCityObj));

		rankDistrictObj.set_rank_type(RankData::kCommon);
		rankDistrictObj.set_rank_name(ssRankDistrict.str());
		keys.emplace_back(RankData::MakeRankKey(rankDistrictObj));
	}

	// 周总榜Key
	rankWorldWeekObj.set_rank_type(RankData::kWeek);
	rankWorldWeekObj.set_rank_deadtm(NextMonday());
	rankWorldWeekObj.set_rank_name(ssRankWorld.str());
	keys.emplace_back(RankData::MakeRankKey(rankWorldWeekObj));
	// 全国周榜Key
	rankCountryWeekObj.set_rank_type(RankData::kWeek);
	rankCountryWeekObj.set_rank_deadtm(NextMonday());
	rankCountryWeekObj.set_rank_name(ssRankCountry.str());
	keys.emplace_back(RankData::MakeRankKey(rankCountryWeekObj));
	// 省周榜Key
	rankProvinceWeekObj.set_rank_type(RankData::kWeek);
	rankProvinceWeekObj.set_rank_deadtm(NextMonday());
	rankProvinceWeekObj.set_rank_name(ssRankProvince.str());
	keys.emplace_back(RankData::MakeRankKey(rankProvinceWeekObj));
	// 市周榜Key
	rankCityWeekObj.set_rank_type(RankData::kWeek);
	rankCityWeekObj.set_rank_deadtm(NextMonday());
	rankCityWeekObj.set_rank_name(ssRankCity.str());
	keys.emplace_back(RankData::MakeRankKey(rankCityWeekObj));
	// 区周榜Key
	rankDistrictWeekObj.set_rank_type(RankData::kWeek);
	rankDistrictWeekObj.set_rank_deadtm(NextMonday());
	rankDistrictWeekObj.set_rank_name(ssRankDistrict.str());
	keys.emplace_back(RankData::MakeRankKey(rankDistrictWeekObj));

	// 日总榜Key
	rankWorldDailyObj.set_rank_type(RankData::kWeek);
	rankWorldDailyObj.set_rank_deadtm(NextDay());
	rankWorldDailyObj.set_rank_name(ssRankWorld.str());
	keys.emplace_back(RankData::MakeRankKey(rankWorldDailyObj));
	// 全国日榜Key
	rankCountryDailyObj.set_rank_type(RankData::kWeek);
	rankCountryDailyObj.set_rank_deadtm(NextDay());
	rankCountryDailyObj.set_rank_name(ssRankCountry.str());
	keys.emplace_back(RankData::MakeRankKey(rankCountryDailyObj));
	// 省日榜Key
	rankProvinceDailyObj.set_rank_type(RankData::kWeek);
	rankProvinceDailyObj.set_rank_deadtm(NextDay());
	rankProvinceDailyObj.set_rank_name(ssRankProvince.str());
	keys.emplace_back(RankData::MakeRankKey(rankProvinceDailyObj));
	// 市日榜Key
	rankCityDailyObj.set_rank_type(RankData::kWeek);
	rankCityDailyObj.set_rank_deadtm(NextDay());
	rankCityDailyObj.set_rank_name(ssRankCity.str());
	keys.emplace_back(RankData::MakeRankKey(rankCityDailyObj));
	// 区日榜Key
	rankDistrictDailyObj.set_rank_type(RankData::kWeek);
	rankDistrictDailyObj.set_rank_deadtm(NextDay());
	rankDistrictDailyObj.set_rank_name(ssRankDistrict.str());
	keys.emplace_back(RankData::MakeRankKey(rankDistrictDailyObj));

}*/

void GeoData::GetGeoRankKeys(RankData::RankType type, const cs::GeoPos& geo, const std::string& rankName, std::vector<std::string>& keys)
{
	uint32_t deadtm = 0;
	if (type == RankData::kWeek) {
		deadtm = NextMonday();
	} else if (type == RankData::kDaily) {
		deadtm = NextDay();
	}

	std::stringstream ss;
	ss << rankName << "_" << geo.game_region();
	std::string nameWithRegion = ss.str();
	db::RankKey rankWorldObj, rankCountryObj, rankProvinceObj, rankCityObj, rankDistrictObj;
	
	rankWorldObj.set_rank_type(type);
	rankWorldObj.set_rank_deadtm(deadtm);
	rankWorldObj.set_rank_name(GetGeoWorldStr(geo, nameWithRegion));
	keys.emplace_back(RankData::MakeRankKey(rankWorldObj));

	rankCountryObj.set_rank_type(type);
	rankCountryObj.set_rank_deadtm(deadtm);
	rankCountryObj.set_rank_name(GetGeoCountryStr(geo, nameWithRegion));
	keys.emplace_back(RankData::MakeRankKey(rankCountryObj));

	rankProvinceObj.set_rank_type(type);
	rankProvinceObj.set_rank_deadtm(deadtm);
	rankProvinceObj.set_rank_name(GetGeoProvinceStr(geo, nameWithRegion));
	keys.emplace_back(RankData::MakeRankKey(rankProvinceObj));

	rankCityObj.set_rank_type(type);
	rankCityObj.set_rank_deadtm(deadtm);
	rankCityObj.set_rank_name(GetGeoCityStr(geo, nameWithRegion));
	keys.emplace_back(RankData::MakeRankKey(rankCityObj));

	rankDistrictObj.set_rank_type(type);
	rankDistrictObj.set_rank_deadtm(deadtm);
	rankDistrictObj.set_rank_name(GetGeoDistrictStr(geo, nameWithRegion));
	keys.emplace_back(RankData::MakeRankKey(rankDistrictObj));
}

ErrCodeType GeoData::GetGeoData(uint32_t plid, cs::GeoPos& geo)
{
	string strPlayerID = to_string(plid);
	string key(kKeyPrefixPlayerData + strPlayerID);
	unordered_map<string, string> m;
	auto& lazySocialInfo = m[kLazySocialInfo];
	if (!gRedis->hget(key, m)) {
		WARN_LOG("hget failed: %s! plid=%u", gRedis->last_error_cstr(), plid);
		return ErrCodeDB;
	}
	db::LazySocialAttr data;
	data.ParseFromString(lazySocialInfo);
	geo.CopyFrom(data.geo_info());
	return ErrCodeSucc;
}

std::string GeoData::GetGeoRankStr(const cs::GeoPos& geo)
{
	stringstream ssRankStr;
	if (geo.has_country_code()) {
		bool inChina = geo.country_code() == kGeoCountryChina;
		ssRankStr << kGeoSplit << geo.country_code();
		if (inChina) {
			if (geo.has_province_code()) {
				ssRankStr << kGeoSplit << geo.province_code();
				if (geo.has_city_code()) {
					ssRankStr << kGeoSplit << geo.city_code();
					if (geo.has_district_code()) {
						ssRankStr << kGeoSplit << geo.district_code();
					}
				}
			}
		}
	}

	return ssRankStr.str();
}

std::string GeoData::GetGeoWorldStr(const cs::GeoPos& geo, const std::string& rankName)
{
	stringstream ssRankStr;
	ssRankStr << rankName;
	return ssRankStr.str();
}

std::string GeoData::GetGeoCountryStr(const cs::GeoPos& geo, const std::string& rankName)
{
	bool inChina = geo.country_code() == kGeoCountryChina;
	int countryCode = inChina ? kGeoCountryChina : kGeoCountryOther;
	stringstream ssRankStr;
	ssRankStr << rankName << kGeoSplit << countryCode;
	return ssRankStr.str();
}

std::string GeoData::GetGeoProvinceStr(const cs::GeoPos& geo, const std::string& rankName)
{
	bool inChina = geo.country_code() == kGeoCountryChina;
	int countryCode = inChina ? kGeoCountryChina : kGeoCountryOther;
	stringstream ssRankStr;
	ssRankStr << rankName << kGeoSplit << countryCode;
	if (inChina) {
		ssRankStr << kGeoSplit << geo.province_code();
	}
	return ssRankStr.str();
}

std::string GeoData::GetGeoCityStr(const cs::GeoPos& geo, const std::string& rankName)
{
	bool inChina = geo.country_code() == kGeoCountryChina;
	int countryCode = inChina ? kGeoCountryChina : kGeoCountryOther;
	stringstream ssRankStr;
	ssRankStr << rankName << kGeoSplit << countryCode;
	if (inChina) {
		ssRankStr << kGeoSplit << geo.province_code() << kGeoSplit << geo.city_code();
	}
	return ssRankStr.str();
}

std::string GeoData::GetGeoDistrictStr(const cs::GeoPos& geo, const std::string& rankName)
{
	bool inChina = geo.country_code() == kGeoCountryChina;
	int countryCode = inChina ? kGeoCountryChina : kGeoCountryOther;
	stringstream ssRankStr;
	ssRankStr << rankName << kGeoSplit << countryCode;
	if (inChina) {
		ssRankStr << kGeoSplit << geo.province_code() << kGeoSplit << geo.city_code() << kGeoSplit << geo.district_code();
	}
	return ssRankStr.str();
}

