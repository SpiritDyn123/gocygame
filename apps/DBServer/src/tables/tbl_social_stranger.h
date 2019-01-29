#pragma once

#include "../core/dbtable.h"

class TblSocialStranger
	: public DBTable<100, 100>
{
public:
	using DBTable::DBTable;
	//TblSocialStranger(SQLConnection& conn, const char* db, const char* tbl);
	// 记录陌生人访问最早时间
	int AddStranger(uint32_t stranger_pid);
	// 获取指定时间段内的陌生人访问人数
	int GetStrangerCnt();
	// 记录自己最新的查询时间
	int UpdateCheckTime();
	// 获取自己最新的查询时间
	int GetCheckTime();
};

