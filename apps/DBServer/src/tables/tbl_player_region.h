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
#include "../proto/db.pb.h"

class TblPlayerRegion
	: public DBTable<1, 1> 
{
public:
	using DBTable::DBTable;
	//TblOrder(SQLConnection& conn, const char* db, const char* tbl);
	TblPlayerRegion(uint32_t uid);

	int GetPlayerRegionInfo(const std::string& channel, const std::string& channelId, google::protobuf::RepeatedPtrField<db::PlayerRegionInfo>* pInfos);
	int PlayerCreate(const std::string& channel, const std::string& channelId, uint32_t plid, uint32_t region);
	int PlayerLogin(const std::string& channel, const std::string& channelId, uint32_t plid, uint32_t region);

private:
};

