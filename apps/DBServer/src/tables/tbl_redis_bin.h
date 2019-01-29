#pragma once

#include "../core/dbtable.h"

class TblRedisBin
	: public DBTable<100, 100>
{
public:
	using DBTable::DBTable;
	int Get(std::unordered_map<std::string, std::string>& fields);
};

