#include "tbl_social_stranger.h"
#include <sstream>
#include "../log.h"

//TblSocialStranger::TblSocialStranger(SQLConnection& conn, const char* db, const char* tbl)
//	: DBTable(conn, db, tbl)
//{
//
//}

int TblSocialStranger::AddStranger(uint32_t stranger_pid)
{
	char sql[1024];
	sprintf(sql, "select * from %s where `pid` = %u and `stranger_pid` = %u", 
		get_table(m_uid).c_str(), m_uid, stranger_pid);
	auto ret = store(sql);
	if (ret.num_rows() != 0) {
		return 0;
	}
	//DEBUG_LOG("sql:%s", sql);
	sprintf(sql, "insert into %s(`pid`, `stranger_pid`, `time`) values ( %u, %u, %u)",
		get_table(m_uid).c_str(), m_uid, stranger_pid, uint32_t(time(nullptr)));
	execute(sql);
	return 0;
}

int TblSocialStranger::GetStrangerCnt()
{
	char sql[1024];
	sprintf(sql, "select count(*) from %s where `pid` = %u and `stranger_pid` != %u and `time` >= %u",
		get_table(m_uid).c_str(), m_uid, m_uid, GetCheckTime());
	//DEBUG_LOG("sql:%s", sql);
	auto ret = store(sql);
	if (ret.num_rows() != 0) {
		auto& info = ret[0][0];
		auto cnt = atoi(info.c_str());
		return cnt;
	}
	return 0;
}

int TblSocialStranger::UpdateCheckTime()
{
	char sql[1024];
	sprintf(sql, "replace into %s values( %u, %u, %u)", 
		get_table(m_uid).c_str(), m_uid, m_uid, uint32_t(time(nullptr)));
	//DEBUG_LOG("sql:%s", sql);
	execute(sql);
	return 0;
}

int TblSocialStranger::GetCheckTime()
{
	char sql[1024];
	sprintf(sql, "select `time` from %s where `pid` = %u and `stranger_pid` = %u",
		get_table(m_uid).c_str(), m_uid, m_uid);
	//DEBUG_LOG("sql:%s", sql);
	auto ret = store(sql);
	if (ret.num_rows() != 0) {
		auto& info = ret[0][0];
		auto cnt = atoi(info.c_str());
		return cnt;
	}
	return 0;
}
