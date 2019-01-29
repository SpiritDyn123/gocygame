/**
 * @file tbl_data.cc
 * @author andy@doogga.com
 *
 * @section DESCRIPTION
 *
 * 创建日期：2014-3-5
 *
 */

#include <unordered_set>

#include "tbl_data.h"

using namespace std;
using namespace mysqlpp;

//TblData* g_data;
//TblData* g_data_hour;
//TblData* g_data_minute;
//
////class TblData::DataRetriever {
////public:
////	DataRetriever(int64_t txid, unordered_map<TxKey, TxVal>& kvs)
////		: m_kvs(kvs)
////	{
////		m_txid = txid;
////	}
////
////	void operator()(Row row)
////	{
////		TxVal& val = m_kvs[TxKey(row[0], row[1], row[2], row[3])];
////		if (m_txid != static_cast<int64_t>(row[5])) {
////			if (val.is_add) {
////				val.val += static_cast<double>(row[4]);
////			}
////			val.changed = 1;
////		} else {
////			val.changed = 0;
////		}
////	}
////
////private:
////	int64_t m_txid;
////	unordered_map<TxKey, TxVal>& m_kvs;
////};
//
//class TblData::ChannelIdRetriever {
//public:
//	ChannelIdRetriever(DbGetChannelsDataOut& out)
//		: out_(out)
//	{
//	}
//
//	void operator()(Row row)
//	{
//		ChannelPair* p = out_.add_channel_pairs();
//		p->set_channel_id(static_cast<uint32_t>(row[0]) >> 16);
//		p->set_value(static_cast<uint32_t>(row[1]));
//	}
//
//private:
//	DbGetChannelsDataOut& out_;
//};
//
//class TblData::ServersDataRetriever {
//public:
//	ServersDataRetriever(DbGetServersDataOut& out)
//		: out_(out)
//	{
//	}
//
//	void operator()(Row row)
//	{
//		ServerPair* p = out_.add_server_pairs();
//		p->set_server_id(static_cast<uint32_t>(row[0]));
//		p->set_value(static_cast<uint32_t>(row[1]));
//	}
//
//private:
//	DbGetServersDataOut& out_;
//};
//
//int TblData::update_data(uint32_t hash, uint32_t company_id, int64_t txid, unordered_map<TxKey, TxVal>& keyvals, DbUpdDataOut& out)
//{
//	unordered_map<TxKey, TxVal>::iterator it = keyvals.begin();
//
////	m_sql.str("");
////	// TODO 如果确定TransactionalDbState::multiUpdate里的返回值没啥价值，这里可以无需SELECT，直接用update和insert！
////	m_sql << "SELECT prod_id, os_chnl_svr_id, data_id, tmstamp, value, txid FROM " << get_table(hash) << " WHERE (prod_id = "
////	      << it->first.appid << " AND os_chnl_svr_id = " << it->first.ocs_id
////	      << " AND data_id = " << it->first.dataid << " AND tmstamp = " << it->first.ts << ")";
////	for (++it; it != keyvals.end(); ++it) {
////		m_sql << " OR (prod_id = " << it->first.appid << " AND os_chnl_svr_id = " << it->first.ocs_id
////				<< " AND data_id = " << it->first.dataid << " AND tmstamp = " << it->first.ts << ")";
////	}
////
////	DataRetriever dr(txid, keyvals);
////	int err = select(m_sql.str(), dr);
////	if (err != dberr_succ) {
////		return err;
////	}
//
//	int err = dberr_succ;
////	unordered_set<TxKey> keys_to_sum;
//	for (it = keyvals.begin(); it != keyvals.end(); ++it) {
//		// 无论该值是否需要更新，都强制进行求和操作
////		if (it->second.sum_all_svrs) {
////			keys_to_sum.insert(it->first);
////		}
//
////		out.add_vals(it->second.val);
////		switch (it->second.changed) {
////		case 1:
////			err = update_data(hash, txid, it->first, it->second);
////			if (err != dberr_succ) {
////				return err;
////			}
////			break;
////		case 2:
////			err = add_data(hash, company_id, txid, it->first, it->second);
////			if (err != dberr_succ) {
////				return err;
////			}
////			break;
////		default:
////			break;
////		}
//
//// update不成功说明记录不存在或者txid相等
//		const TxKey& k = it->first;
//		const TxVal& v = it->second;
//		err = update_data(hash, company_id, txid, k, v);
//		if (err == dberr_entry_not_found) {
//			// insert也不成功则表明txid相等。
//			err = add_data(hash, company_id, txid, k, v);
//		}
//		if (err == dberr_succ) {
//			if (it->second.sum_all_svrs && k.ocs_id) {
//				err = sum_data(hash, company_id, k);
//				if (err != dberr_succ) {
//					return err;
//				}
//			}
//			continue;
//		}
//
//		return err;
//	}
//
////	for (unordered_set<TxKey>::iterator it = keys_to_sum.begin(); it != keys_to_sum.end(); ++it) {
////		err = sum_data(hash, *it);
////		if (err != dberr_succ) {
////			return err;
////		}
////	}
//
//	return dberr_succ;
//}
//
//int TblData::update_data(uint32_t hash, const DbUpdDayDataIn& in)
//{
//	int err = dberr_succ;
//	for (int i = 0; i != in.data_size(); ++i) {
//		const DataElement& e = in.data(i);
//		uint32_t oschnlsvr_id = ((e.os() << 24) | (e.channel() << 16) | e.svrid());
//		m_sql.str("");
//		m_sql << "INSERT INTO " << get_table(hash) << " (company_id, prod_id, os_chnl_svr_id, data_id, tmstamp, value) VALUES("
//		      << in.company_id()
//		      << ", " << in.appid() << ", " << oschnlsvr_id
//		      << ", "
//		      << in.dataid() << ", " << in.timestamp() << ", " << e.value() << ") ON DUPLICATE KEY UPDATE value = " << e.value();
//		err = insert(m_sql.str());
//		if (err != dberr_succ) {
//			break;
//		}
//	}
//
//	return err;
//}
//
//int TblData::get_channels_data(uint32_t hash, const DbGetChannelsDataIn& in, DbGetChannelsDataOut& out)
//{
//	m_sql.str("");
//	m_sql << "SELECT os_chnl_svr_id, value FROM " << get_table(hash) << " WHERE prod_id = " << in.appid()
//			<< " AND data_id = " << in.dataid() << " AND tmstamp = " << in.timestamp()
//			<< " AND os_chnl_svr_id != 0 AND (os_chnl_svr_id & 0xFF00FFFF) = 0";
//
//	ChannelIdRetriever retriever(out);
//	return select(m_sql.str(), retriever);
//}
//
//int TblData::get_servers_data(uint32_t hash, const DbGetServersDataIn& in, DbGetServersDataOut& out)
//{
//	m_sql.str("");
//	m_sql << "SELECT os_chnl_svr_id, value FROM " << get_table(hash) << " WHERE prod_id = " << in.appid()
//			<< " AND data_id = " << in.dataid() << " AND tmstamp = " << in.timestamp()
//			<< " AND os_chnl_svr_id > 0 AND os_chnl_svr_id <= 0xFFFF";
//
//	ServersDataRetriever retriever(out);
//	return select(m_sql.str(), retriever);
//}
//
////--------------------------------------------------------------------------
//// Private Methods
////
//int TblData::add_data(uint32_t hash, uint32_t company_id, int64_t txid, const TxKey& k, const TxVal& v)
//{
//	m_sql.str("");
//	m_sql << "INSERT INTO " << get_table(hash) << " (company_id, prod_id, os_chnl_svr_id, data_id, tmstamp, value, txid) VALUES("
//	      << company_id
//	      << ", " << k.appid << ", " << k.ocs_id
//	      << ", "
//	      << k.dataid << ", " << k.ts << ", " << v.val << ", " << txid << ")";
//
//	return insert(m_sql.str(), dberr_succ); // 如果已经插入过了，也返回成功
//}
//
//int TblData::update_data(uint32_t hash, uint32_t company_id, int64_t txid, const TxKey& k, const TxVal& v)
//{
//	m_sql.str("");
////	m_sql << "UPDATE " << get_table(hash) << " SET value = " << v.val << ", txid = " << txid << " WHERE prod_id = " << k.appid
////			<< " AND os_chnl_svr_id = " << k.ocs_id << " AND data_id = " << k.dataid << " AND tmstamp = " << k.ts;
//
//	if (v.is_add) {
//		m_sql << "UPDATE " << get_table(hash) << " SET value = value + " << v.val << ", txid = " << txid
//		      << " WHERE company_id = "
//		      << company_id << " AND prod_id = "
//		      << k.appid
//		      << " AND os_chnl_svr_id = " << k.ocs_id
//		      << " AND data_id = "
//		      << k.dataid << " AND tmstamp = " << k.ts << " AND txid != " << txid;
//	} else {
//		m_sql << "UPDATE " << get_table(hash) << " SET value = " << v.val << ", txid = " << txid
//		      << " WHERE company_id = "
//		      << company_id << " AND prod_id = "
//		      << k.appid
//		      << " AND os_chnl_svr_id = " << k.ocs_id
//		      << " AND data_id = "
//		      << k.dataid << " AND tmstamp = " << k.ts << " AND txid != " << txid;
//	}
//
//	return update_one(m_sql.str());
//}
//
//int TblData::sum_data(uint32_t hash, uint32_t company_id, const TxKey& k)
//{
//	m_sql.str("");
//	m_sql << "INSERT INTO " << get_table(hash) << " (company_id, prod_id, os_chnl_svr_id, data_id, tmstamp, value) "
//	      << "SELECT company_id, prod_id, 0, data_id, tmstamp, SUM(value) FROM "
//	      << get_table(hash)
//	      << " WHERE company_id = " << company_id
//	      << " AND prod_id = "
//	      << k.appid << " AND os_chnl_svr_id != 0 AND data_id = " << k.dataid
//	      << " AND tmstamp = "
//	      << k.ts << " ON DUPLICATE KEY UPDATE value = (SELECT SUM(value) FROM "
//	      << get_table(hash)
//	      << " WHERE company_id = " << company_id
//	      << " AND prod_id = " << k.appid << " AND os_chnl_svr_id != 0 AND data_id = "
//	      << k.dataid
//	      << " AND tmstamp = " << k.ts << ")";
//
//	return insert(m_sql.str());
//}
