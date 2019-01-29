#pragma once
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <stdint.h>
#include <vector>
#include <libant/utils/Singleton.h>
#include <libant/utils/StringMap.h>
#include <rapidjson/document.h>

class SQLConnection;
class MysqlProxy
	: public Singleton<MysqlProxy>
{
public:
	MysqlProxy();
	~MysqlProxy();
	void LoadMysqlCfg(const std::string& cfg);
	SQLConnection* GetSqlConById(const std::string& dbName, uint32_t id);
	SQLConnection* GetDefaultSqlCon();
	bool IsValidPrefix(const std::string& prefix);
	std::string GetOfflineKey();
	bool IsLoad() const { return mLoad; }
	const char* GetKv(const std::string& k);
	bool Redis2mysqlEnable() const;
private:
	//uint32_t GetStrHash(const char* str);
private:
	struct ConInfo {
		void Load(const rapidjson::Value& v);
		uint32_t id;
		std::string user;
		std::string pwd;
		std::string ip;
		uint32_t port;
		SQLConnection* con;
	};
	struct DBLocation {
		void Load(const rapidjson::Value& v);
		int GetDBId(uint32_t idx) const;
		std::string name;
		bool single;
		uint32_t singleLocation;
		std::unordered_map<uint32_t, uint32_t> location;
	};
	struct Redis2mysql {
		void Load(const rapidjson::Value& v);
		bool WithPrefix(const std::string& prefix) const;
		std::unordered_set<std::string> validPrefix;
		bool enable;
	};

	bool mLoad;
	std::unordered_map<uint32_t, ConInfo*> mConDict;
	std::vector<uint32_t> mDbIdList;
	Redis2mysql mRedis2mysql;
	std::string mOfflineKey;
	std::unordered_map<std::string, std::string> mKvs;
	StringMap<DBLocation> mDBLocations;
};
