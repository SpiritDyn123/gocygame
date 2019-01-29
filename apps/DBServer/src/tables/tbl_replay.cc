#include "tbl_replay.h"
#include <sstream>
#include "../log.h"

//TblReplay::TblReplay(SQLConnection& conn, const char* db, const char* tbl)
//	: DBTable(conn, db, tbl)
//{
//
//}

int TblReplay::Add(const std::string& key, const std::string& ver, const char* data, uint32_t len, uint32_t refCnt /* = 1 */)
{
	char sql[1024];
	sprintf(sql, "insert into %s(`uid`, `key`, `refCnt`, `ver`, `data`) values ( %u, '%s', %u, '%s', '"
		, get_table(m_uid).c_str(), m_uid, key.c_str(), refCnt, ver.c_str());
	//auto& con = get_con(uid);
	//std::stringstream query;
	auto query = get_query();
	query << sql << mysqlpp::escape << std::string(data, len) << "')";
	DEBUG_LOG("add tbl:%s, key: %s, uid:%u", get_table(m_uid).c_str(), key.c_str(), m_uid);
	execute(query.str());
	/*if (!query.execute()) {
		DEBUG_LOG("%s", query.error());
	}*/
	return 0;
}

int TblReplay::Add(const std::string& key, const std::string& ver, const std::string& data, uint32_t refCnt /* = 1 */)
{
	char sql[1024];
	sprintf(sql, "insert into %s(`uid`, `key`, `refCnt`, `ver`, `data`) values ( %u, '%s', %u, '%s', '"
		, get_table(m_uid).c_str(), m_uid, key.c_str(), refCnt, ver.c_str());
	//auto& con = get_con(uid);
	auto query = get_query();
	//std::stringstream query;
	query << sql << mysqlpp::escape << data << "')";
	DEBUG_LOG("add tbl:%s, key: %s, uid:%u", get_table(m_uid).c_str(), key.c_str(), m_uid);
	execute(query.str());
	/*if (!query.execute()) {
		DEBUG_LOG("%s", query.error());
	}*/
	return 0;
}

int TblReplay::Get(const std::string& key, const std::string& ver, std::string& data, bool ignoreVer)
{
	char sql[1024];
	sprintf(sql, "select data, ver from %s where `key` = '%s' limit 1", get_table(m_uid).c_str(), key.c_str());
	//auto& con = get_con(uid);
	auto ret = store(sql);
	if (ret.num_rows() != 0) {
		auto& info = ret[0][0];
		if (!ignoreVer) {
			auto& rVer = ret[0][1];
			if (rVer.compare(ver) != 0) {
				return -1;
			}
		}
		//DEBUG_LOG("get key:%s data len: %zu", key.c_str(), info.size());
		data.assign(info.data(), info.size());
	}
	return 0;
}

int TblReplay::Del(const std::string& key)
{
	DEBUG_LOG("del key:%s", key.c_str());
	char sql[1024];
	sprintf(sql, "select `refCnt` from %s where `key` = '%s' limit 1", get_table(m_uid).c_str(), key.c_str());
	//auto& con = get_con(uid);
	auto ret = store(sql);
	if (ret.num_rows() != 0) {
		auto& info = ret[0][0];
		auto cnt = atoi(info.c_str());
		if (cnt <= 1) {
			sprintf(sql, "delete from %s where `key` = '%s' limit 1", get_table(m_uid).c_str(), key.c_str());
			execute(sql);
		}
		else {
			sprintf(sql, "update %s set `refCnt` = `refCnt` - 1 where `key` = '%s' limit 1", get_table(m_uid).c_str(), key.c_str());
			execute(sql);
		}
	}
	return 0;
}

