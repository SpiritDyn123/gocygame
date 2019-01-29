/**
 * @file tbl_data.h
 * @author andy@doogga.com
 *
 * @section DESCRIPTION
 *
 * 创建日期：2014-3-5
 *
 */

#ifndef DOOGGA_DBSVR_TBL_DATA_H_
#define DOOGGA_DBSVR_TBL_DATA_H_

#include <functional>
#include <unordered_map>

#include "../core/dbtable.h"

///**
// * @brief 操作t_data表
// */
//class TblData : public DBTable<1000, 10> {
//private:
//	/**
//	 * @brief 用于获取查询得到的t_data表的value和txid
//	 */
//	// class DataRetriever;
//
//	/**
//	 * @brief 用于获取渠道数据
//	 */
//	class ChannelIdRetriever;
//	/**
//	 * @brief 用于获取区服数据
//	 */
//	class ServersDataRetriever;
//
//public:
//	/**
//	 * @brief t_data表的key
//	 */
//	class TxKey {
//	public:
//		friend struct std::hash<TxKey>;
//		friend class DataRetriever;
//		friend class TblData;
//
//	public:
//		TxKey(uint32_t app_id, uint32_t ocsid, uint32_t datid, uint32_t t)
//		{
//			appid = app_id;
//			ocs_id = ocsid;
//			dataid = datid;
//			ts = t;
//		}
//
//		bool operator==(const TxKey& key) const
//		{
//			return (appid == key.appid) && (ocs_id == key.ocs_id) && (dataid == key.dataid) && (ts == key.ts);
//		}
//
//	private:
//		uint32_t appid;
//		uint32_t ocs_id;
//		uint32_t dataid;
//		uint32_t ts;
//	};
//	/**
//	 * @brief t_data表的value
//	 */
//	class TxVal {
//	public:
//		friend class DataRetriever;
//		friend class TblData;
//
//	public:
//		TxVal(double v = -1, bool add = false, bool sum = false)
//		{
//			val = v;
//			is_add = add;
//			sum_all_svrs = sum;
//			changed = 2;
//		}
//
//		void add_val(double v)
//		{
//			val += v;
//		}
//
//	private:
//		double val;
//		bool is_add;
//		bool sum_all_svrs;
//		char changed; // 0 无需更新，1 更新，2 插入
//	};
//
//public:
//	/**
//	 * @brief 构造函数
//	 */
//	TblData(SQLConnection& conn, const char* tblname_prefix)
//		: DBTable<1000, 10>(conn, "db_dg_repo", tblname_prefix)
//	{
//	}
//
//	/**
//	 * @brief 把keyvals里的值更新到数据库
//	 */
//	int update_data(uint32_t hash, uint32_t company_id, int64_t txid, std::unordered_map<TxKey, TxVal>& keyvals, DbUpdDataOut& out);
//
//	/**
//	 * @brief 更新数据库
//	 */
//	int update_data(uint32_t hash, const DbUpdDayDataIn& in);
//
//	/**
//	 * @brief 获取某data_id的各个渠道的数据
//	 */
//	int get_channels_data(uint32_t hash, const DbGetChannelsDataIn& in, DbGetChannelsDataOut& out);
//
//	/**
//	 * @brief 获取某data_id的各个区服的数据
//	 */
//	int get_servers_data(uint32_t hash, const DbGetServersDataIn& in, DbGetServersDataOut& out);
//
//private:
//	/**
//	 * @brief 添加一条统计数据
//	 */
//	int add_data(uint32_t hash, uint32_t company_id, int64_t txid, const TxKey& k, const TxVal& v);
//	/**
//	 * @brief 更新一条统计数据
//	 */
//	int update_data(uint32_t hash, uint32_t company_id, int64_t txid, const TxKey& k, const TxVal& v);
//
//	/**
//	 * @brief 对k所指定的所有不同区服的value做求和操作，结果更新到区服0
//	 */
//	int sum_data(uint32_t hash, uint32_t company_id, const TxKey& k);
//};
//
//namespace std {
///**
// * @brief 用于计算TxKey的哈希值
// */
//template <>
//struct hash<typename TblData::TxKey> {
//public:
//	size_t operator()(const TblData::TxKey& k) const
//	{
//		return ((hash<uint32_t>()(k.ocs_id) ^ (hash<uint32_t>()(k.dataid) << 1))
//				^ ((hash<uint32_t>()(k.ts) >> 1) ^ hash<uint32_t>()(k.appid)));
//	}
//};
//
//}
//
//extern TblData* g_data;
//extern TblData* g_data_hour;
//extern TblData* g_data_minute;

#endif /* DOOGGA_DBSVR_TBL_DATA_H_ */
