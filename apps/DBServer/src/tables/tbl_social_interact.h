#pragma once

#include "../core/dbtable.h"
#include <functional>

class TblSocialInteract
	: public DBTable<100, 100>
{
public:
	using DBTable::DBTable;
	//TblSocialInteract(SQLConnection& conn, const char* db, const char* tbl);
	int AddInteract(uint32_t interact_pid, uint32_t type, uint32_t sub_type);
	int GetCntByInteractType(uint32_t type);
	int GetInteractList(uint32_t type, int32_t from_pos, int32_t end_pos,
		const std::function<void(uint32_t interact_pid, uint32_t sub_type, uint32_t time_stamp)>& fn);
};

