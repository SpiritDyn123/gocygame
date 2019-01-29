/*===============================================================
* @Author: car
* @Created Time : 2019年01月04日 星期二 10时40分51秒
*
* @File Name: tbl_corp_nick.h
* @Description:
*
================================================================*/
#pragma once

#include "../core/dbtable.h"

class TblCorpNick
	: public DBTable<1, 1> 
{
public:
	using DBTable::DBTable;
	TblCorpNick(uint32_t cid);

	int CheckNick(const std::string& nick, bool& exist);
	int CheckNick(const std::string& nick, uint32_t& corpId, bool& exist);

	int InsertNick(const std::string& nick); 
	int DeleteNick(const std::string& nick);
};

