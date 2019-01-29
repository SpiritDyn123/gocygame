#include "global.h"
#include <serverbench/config.hpp>

//SQLConnection* gSqlcon = nullptr;
//
//void InitSqlCon()
//{
//	auto dbAddr = config_get_strval("mysql_addr");
//	uint32_t dbPort = config_get_intval("mysql_port");
//	auto dbUser = config_get_strval("mysql_user");
//	auto dbPwd = config_get_strval("mysql_pwd");
//	gSqlcon = new SQLConnection(dbUser, dbPwd, false, dbAddr, dbPort);
//}
//
//