/*===============================================================
* @Author: car
* @Created Time : 2018年06月12日 星期二 10时40分54秒
*
* @File Name: tbl_order.cc
* @Description:
*
================================================================*/

#include "tbl_order.h"
#include "../log.h"

//TblOrder::TblOrder(SQLConnection& conn, const char* db, const char* tbl)
//	: DBTable(conn, db, tbl)
//{
//	
//}



int TblOrder::Add(const std::string& cp_trade_no, double total_fee,
	const std::string& currency_type, const std::string& product_id,
	const std::string& create_time, const std::string& trade_time,
	const std::string& is_test_order, const std::string& channel_trade_no,
	const std::string& channel, const std::string& gems)
{
	char sql[1024];
	sprintf(sql, "insert into %s(`cp_trade_no`, `player_id`, `total_fee`,`currency_type` ,`product_id` ,`create_time` ,`trade_time`,`is_test_order` ,`channel_trade_no` , `channel` , `gems`) values ( '%s', %u, %f ,'%s' , '%s' ,'%s'  \
				,'%s' ,'%s'	\
				,'%s' ,'%s'	\
				,'%s')", get_table(m_uid).c_str(), cp_trade_no.c_str(), m_uid,
								total_fee, currency_type.c_str(), product_id.c_str()
								,create_time.c_str() ,trade_time.c_str()
								,is_test_order.c_str() ,channel_trade_no.c_str()
								,channel.c_str() ,gems.c_str());
	auto query = get_query(sql);
	//query << sql;
	DEBUG_LOG("tbl:%s, plid:%u sql:%s", get_table(m_uid).c_str(), m_uid, sql);
	execute(query.str());
	/*
	if (!query.execute()) {
		DEBUG_LOG("%s", query.error());
	}*/
	return 0;

}
