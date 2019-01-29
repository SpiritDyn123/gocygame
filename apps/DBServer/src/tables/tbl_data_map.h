/**
 * @file tbl_data_map.h
 * @author andy@doogga.com
 *
 * @section DESCRIPTION
 *
 * 创建日期：2014-3-5
 *
 */

#ifndef DOOGGA_DBSVR_TBL_DATA_MAP_H_
#define DOOGGA_DBSVR_TBL_DATA_MAP_H_

#include "../core/dbtable.h"

///**
// * @brief 操作t_data_map表
// */
//class TblDataMap : public DBTable<1000, 10> {
//public:
//	/**
//	 * @brief 构造函数
//	 */
//	TblDataMap(SQLConnection& conn)
//		: DBTable<1000, 10>(conn, "db_dg_repo", "t_data_map")
//	{
//	}
//
//	/**
//	 * @brief 获取dataid
//	 */
//	int get_dataid(uint32_t hash, const DbGetDataIdIn& in, uint32_t& dataid);
//	/**
//	 * @brief 新建dataid
//	 */
//	int create_dataid(uint32_t hash, const DbGetDataIdIn& in, uint32_t& dataid);
//};
//
//extern TblDataMap* g_datamap;

#endif /* DOOGGA_DBSVR_TBL_DATA_MAP_H_ */
