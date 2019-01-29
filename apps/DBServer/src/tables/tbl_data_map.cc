/**
 * @file tbl_data_map.cc
 * @author andy@doogga.com
 *
 * @section DESCRIPTION
 *
 * 创建日期：2014-3-5
 *
 */

#include "tbl_data_map.h"

using namespace std;

//TblDataMap* g_datamap;
//
//int TblDataMap::get_dataid(uint32_t hash, const DbGetDataIdIn& in, uint32_t& dataid)
//{
//	m_sql.str("");
//	m_sql << "SELECT id FROM " << get_table(hash) << " WHERE prod_id = " << in.appid() << " AND report_id = " << in.reportid()
//	      << " AND aux_values = '"
//	      << escape_string(in.auxvals()) << "' AND last_value = '" << escape_string(in.lastval()) << "'";
//
//	return select_one(m_sql.str(), dataid);
//}
//
//int TblDataMap::create_dataid(uint32_t hash, const DbGetDataIdIn& in, uint32_t& dataid)
//{
//	m_sql.str("");
//	m_sql << "INSERT INTO " << get_table(hash) << " (prod_id, report_id, aux_values, last_value) VALUES ("
//	      << in.appid()
//	      << ", " << in.reportid() << ", '" << in.auxvals() << "', '" << in.lastval() << "')";
//	return insert(m_sql.str(), dataid);
//}
