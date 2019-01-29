/*===============================================================
* @Author: car
* @Created Time : 2019年01月04日 星期二 10时40分51秒
*
* @File Name: tbl_player_nick.h
* @Description:
*
================================================================*/
#pragma once

#include "../core/dbtable.h"

class TblPlayerNick
	: public DBTable<1, 1> 
{
public:
	using DBTable::DBTable;
	//TblOrder(SQLConnection& conn, const char* db, const char* tbl);
	TblPlayerNick(uint32_t uid);

	int CheckNick(const std::string& nick, bool& exist);
	int CheckNick(const std::string& nick, uint32_t& plid, bool& exist);
	int CheckNick(const std::string& nick, uint32_t& plid, uint32_t& expireTm, bool& exist);

	int InsertNick(const std::string& nick, uint32_t expireTm = 0); 
	int DeleteNick(const std::string& nick);
	int UpdateExpireTm(const std::string& nick, uint32_t expireTm);

private:
	int ClearExpireNick();
};

