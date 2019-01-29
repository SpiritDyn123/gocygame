/*===============================================================
* @Author: car
* @Created Time : 2018年03月06日 星期二 14时11分15秒
*
* @File Name: tbl_questionnaire.cc
* @Description:
*
================================================================*/
#include "tbl_questionnaire.h"
#include "../log.h"



//TblQuestionnaire::TblQuestionnaire(SQLConnection& conn, const char* db, const char* tbl)
//	: DBTable(conn, db, tbl)
//{
//
//}



int TblQuestionnaire::Add(uint32_t qaid, const std::string& ans, uint32_t seq)
{
	char sql[1024];
	sprintf(sql, "insert into %s(`qaid`, `uid`, `seq`, `ans`) values ( %u, %u, %u, '%s')", get_table(m_uid).c_str(), qaid, m_uid, seq, ans.c_str());
	auto query = get_query();
	query << sql;
	DEBUG_LOG("tbl:%s, plid:%u sql:%s", get_table(qaid).c_str(), m_uid, sql);
	execute(query.str());
	/*
	if (!query.execute()) {
		DEBUG_LOG("%s", query.error());
	}
	*/
	return 0;

}


int TblQuestionnaire::UpInfo(const std::string& key)
{
	char sql[1024];
	sprintf(sql, "insert into %s.qa_info_user(`uid`, `name`) values ( %u, '%s') ON DUPLICATE KEY UPDATE uid=uid", get_dbname(0).c_str(), m_uid, key.c_str());
	auto query = get_query();
	query << sql;
	DEBUG_LOG("sql %s", sql);
	execute(query.str());
	/*
	if (!query.execute()) {
		DEBUG_LOG("%s", query.error());
	}*/
	return 0;
}
