#include "tbl_social_interact.h"
#include <sstream>
#include "../log.h"

//TblSocialInteract::TblSocialInteract(SQLConnection& conn, const char* db, const char* tbl)
//	: DBTable(conn, db, tbl)
//{
//
//}

int TblSocialInteract::AddInteract(uint32_t interact_pid, uint32_t type, uint32_t sub_type)
{
	char sql[1024];
	sprintf(sql, "replace into %s values( %u, %u, %u, %u, %u)",
		get_table(m_uid).c_str(), m_uid, interact_pid, type, sub_type, uint32_t(time(nullptr)));
	//DEBUG_LOG("sql:%s", sql);
	execute(sql);
	return 0;
}

int TblSocialInteract::GetCntByInteractType(uint32_t type)
{
	char sql[1024];
	sprintf(sql, "select count(*) from %s where `pid` = %u and `type` = %u",
		get_table(m_uid).c_str(), m_uid, type);
	//DEBUG_LOG("sql:%s", sql);
	auto ret = store(sql);
	if (ret.num_rows() != 0) {
		auto& info = ret[0][0];
		auto cnt = atoi(info.c_str());
		return cnt;
	}
	return 0;
}

int TblSocialInteract::GetInteractList(uint32_t type, int32_t from_pos, int32_t end_pos,
	const std::function<void(uint32_t interact_pid, uint32_t sub_type, uint32_t time_stamp)>& fn)
{
	char sql[1024];
	sprintf(sql, "select `interact_pid`, `sub_type`, `time` from %s where `pid` = %u and `type` = %u order by `time` desc limit %d, %d",
		get_table(m_uid).c_str(), m_uid, type, from_pos, end_pos - from_pos);
	auto ret = store(sql);
	for (auto& it : ret) {
		fn(atoi(it[0].c_str()), atoi(it[1].c_str()), atoi(it[2].c_str()));
	}
	return 0;
}