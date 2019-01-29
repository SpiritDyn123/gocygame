#pragma once

#include "../core/dbtable.h"

class TblLoginRecord
	: public DBTable<100, 100>
{
public:
	using DBTable::DBTable;
	// 插入玩家上线记录
	int AddLoginRecord(std::string ip);
	// 拉取玩家上线记录
	int GetLoginRecords(uint32_t index, uint32_t limit, const std::function<void(std::string ip, uint32_t time_stamp)>& fn);
};

