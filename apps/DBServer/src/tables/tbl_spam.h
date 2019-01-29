#pragma once

#include "../core/dbtable.h"
class TblSpam
	: public DBTable<1, 1>
{
public:
	using DBTable::DBTable;
	// 插入spam记录
	int AddSpamRecord(const std::string& name, uint32_t pid, const std::string& channel, uint32_t ban_time, 
		const std::string& content, uint32_t result, uint32_t ban_type);
	// 更新spam记录状态
	int UpdateSpamRecord(uint32_t id, uint32_t state);
	// 拉取玩家抽卡记录
	int GetSpamRecords(uint32_t index, uint32_t limit,
		const std::function<void(uint32_t id, uint32_t read, uint32_t state, std::string name, uint32_t pid, std::string channel, 
			uint32_t ban_time, std::string content, uint32_t result, uint32_t ban_type)>& fn);
};

