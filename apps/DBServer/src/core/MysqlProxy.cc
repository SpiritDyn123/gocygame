#include "MysqlProxy.h"
#include "sqlconnection.h"
#include <rapidjson/document.h>
#include <rapidjson/filestream.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>
#include <serverbench/benchapi.hpp>
#include <libant/hash/hash_algo.h>

MysqlProxy::MysqlProxy()
	: mLoad(false)
{
}


MysqlProxy::~MysqlProxy()
{
	mDBLocations.Clear();
	for (auto& it : mConDict) {
		delete it.second;
	}
	mConDict.clear();
}

SQLConnection* MysqlProxy::GetSqlConById(const std::string& dbName, uint32_t id)
{
	auto info = mDBLocations.GetC(dbName);
	int dbId = -1;
	if (info) {
		dbId = info->GetDBId(id);
	}
	if (dbId < 0) {
		ERROR_LOG("DB Name Not Define [%s]", dbName.c_str());
		return nullptr;
	}

	auto it = mConDict.find(dbId);
	if (it != mConDict.end()) {
		return it->second->con;
	}
	else {
		ERROR_LOG("DB Id Not Define [%d]", dbId);
		//error
		return nullptr;
	}
}


SQLConnection* MysqlProxy::GetDefaultSqlCon()
{
	return mConDict.begin()->second->con;
}


bool MysqlProxy::IsValidPrefix(const std::string & prefix)
{
	return mRedis2mysql.WithPrefix(prefix);
}

std::string MysqlProxy::GetOfflineKey()
{
	return mOfflineKey;
}


const char* MysqlProxy::GetKv(const std::string& k)
{
	auto it = mKvs.find(k);
	if (it != mKvs.end()) {
		return it->second.c_str();
	}
	else {
		return nullptr;
	}
}


bool MysqlProxy::Redis2mysqlEnable() const
{
	return mRedis2mysql.enable;
}

void MysqlProxy::LoadMysqlCfg(const std::string& cfg)
{
	if (mLoad) {
		return;
	}
	FILE *fp = fopen(cfg.c_str(), "r");
	if (!fp) {
		return;
	}
	rapidjson::FileStream is(fp);
	rapidjson::Document doc;
	doc.ParseStream<0>(is);

	if (doc.HasParseError())
	{
		ERROR_LOG("GetParseError %d", doc.GetParseError());
		return;
	}
	rapidjson::StringBuffer buffer;
	rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
	doc.Accept(writer);
	//DEBUG_LOG("%s", buffer.GetString());
	mLoad = true;

	auto& mysql = doc["mysql"];
	if (mysql.IsArray()) {
		for (size_t i = 0; i < mysql.Size(); ++i) {
			auto& item = mysql[i];
			ConInfo* info = new ConInfo();
			info->Load(item);
			if (info->con) {
				mConDict[info->id] = info;
			}
			else {
				ERROR_LOG("Connect To Mysql Failed Id:%u", info->id);
			}
		}
	}
	auto& dbs = doc["dbs"];
	if (dbs.IsArray()) {
		for (size_t i = 0; i < dbs.Size(); ++i) {
			auto& item = dbs[i];
			DBLocation* loca = new DBLocation();
			loca->Load(item["location"]);
			loca->name = item["name"].GetString();
			mDBLocations.Insert(loca->name, loca);
		}
	}
	mRedis2mysql.Load(doc["redis2mysql"]);

	if(doc.HasMember("offlineKey")) {
		mOfflineKey = doc["offlineKey"].GetString();
	}
	if (doc.HasMember("kvs")) {
		auto& kvs = doc["kvs"];
		for (auto it = kvs.MemberBegin(); it != kvs.MemberEnd(); ++it) {
			mKvs.insert(std::make_pair(it->name.GetString(), it->value.GetString()));
		}
	}
	fclose(fp);
	INFO_LOG("LoadMysqlCfg Success!");
}

//uint32_t MysqlProxy::GetStrHash(const char * str)
//{
//	return ant::murmur_hash2(str, strlen(str), 123123);
//}

void MysqlProxy::ConInfo::Load(const rapidjson::Value& v)
{
	id = v["id"].GetUint();
	user = v["user"].GetString();
	pwd = v["pwd"].GetString();
	ip = v["host"].GetString();
	port = v["port"].GetUint();
	//πÿ±’ ¬ŒÔ
	con = new SQLConnection(user, pwd, false, ip, port);
}

void MysqlProxy::DBLocation::Load(const rapidjson::Value& info)
{
	if (info.IsUint()) {
		singleLocation = info.GetUint();
		single = true;
	}
	else if (info.IsArray()) {
		single = false;
		for (auto it = info.Begin(); it != info.End(); ++it) {
			auto& v = *it;
			uint32_t begin = v["begin"].GetUint();
			uint32_t end = v["end"].GetUint();
			uint32_t dbId = v["dbId"].GetUint();
			for (uint32_t i = begin; i <= end; ++i) {
				if (location.find(i) == location.end()) {
					location[i] = dbId;
				}
				else {
					// error dumplicate define
					ERROR_LOG("DB [%s] Dumplicate Idx [%u], Already Define In DB Id [%u] ", name.c_str(), i, location[i]);
				}
			}
		}
	}
	else {
		ERROR_LOG("DB [%s] Location Type Error", name.c_str());
	}
}

int MysqlProxy::DBLocation::GetDBId(uint32_t idx) const
{
	if (single) {
		return singleLocation;
	}
	else {
		auto it = location.find(idx);
		if (it != location.end()) {
			return it->second;
		}
		else {
			//error
			ERROR_LOG("DB Idx[%d] Not Define With Name [%s]", idx, name.c_str());
			return -1;
		}
	}
}


void MysqlProxy::Redis2mysql::Load(const rapidjson::Value& v)
{
	if (!v.IsObject()) {
		return;
	}
	enable = true;
	if (v.HasMember("enable")) {
		enable = v["enable"].GetBool();
	}
	auto& prefix = v["prefix"];
	if (prefix.IsArray()) {
		for (uint32_t i = 0; i < prefix.Size(); ++i) {
			auto& item = prefix[i];
			validPrefix.insert(item.GetString());
		}
	}
}

bool MysqlProxy::Redis2mysql::WithPrefix(const std::string & prefix) const
{
	return validPrefix.find(prefix) != validPrefix.end();
}
