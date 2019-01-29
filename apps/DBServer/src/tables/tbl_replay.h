#pragma once

#include "../core/dbtable.h"

class TblReplay
	: public DBTable<100, 100>
{
public:
	using DBTable::DBTable;
//	TblReplay(SQLConnection& conn, const char* db, const char* tbl);
	int Add(const std::string& key, const std::string& ver, const char* data, uint32_t len, uint32_t refCnt = 1);
	int Add(const std::string& key, const std::string& ver, const std::string& data, uint32_t refCnt = 1);
	int Get(const std::string& key, const std::string& ver, std::string& data, bool ignoreVer);
	int Del(const std::string& key);
};

