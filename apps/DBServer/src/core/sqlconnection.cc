/**
 * @file sqlconnection.cc
 * @author andy@doogga.com
 *
 * @section DESCRIPTION
 *
 * 创建日期：2014-3-3
 *
 */

#include <serverbench/benchapi.hpp>

#include "sqlconnection.h"

using namespace mysqlpp;

SQLConnection* g_sql;


SQLConnection::SQLConnection(const std::string& user, const std::string& password
	, bool use_transaction /*= true*/, const std::string& ip /*= "localhost"*/, unsigned int port /*= 3306*/)
	: Connection(false)
	, m_query(query())
	, m_user(user)
	, m_password(password)
	, m_ip(ip)
{
	m_port = port;
	m_use_transaction = use_transaction;
	connect_to_db();
}

SimpleResult SQLConnection::execute(const std::string& sql)
{
	//DEBUG_LOG("%s", sql.c_str());

	SimpleResult res = m_query.execute(sql);
	if (res) {
		// DEBUG_LOG("SQL END: %s", sql.c_str());
		return res;
	}

	if ((m_query.errnum() == CR_SERVER_GONE_ERROR || m_query.errnum() == CR_SERVER_LOST) && connect_to_db()) {
		res = m_query.execute(sql);
	}

	if (m_query.errnum()) {
		DEBUG_LOG("execute() failed: err=[%d:%s]\n sql=[%s] ", m_query.errnum(), m_query.error(), sql.c_str());
	}

	// DEBUG_LOG("SQL END: %s", sql.c_str());
	return res;
}

StoreQueryResult SQLConnection::store(const std::string& sql)
{
	//DEBUG_LOG("%s", sql.c_str());

	StoreQueryResult res = m_query.store(sql);
	if (res) {
		// DEBUG_LOG("SQL END: %s", sql.c_str());
		return res;
	}

	if ((m_query.errnum() == CR_SERVER_GONE_ERROR || m_query.errnum() == CR_SERVER_LOST) && connect_to_db()) {
		res = m_query.store(sql);
	}

	if (m_query.errnum()) {
		DEBUG_LOG("store() failed: err=[%d:%s]\n sql=[%s]", m_query.errnum(), m_query.error(), sql.c_str());
	}

	// DEBUG_LOG("SQL END: %s", sql.c_str());
	return res;
}

UseQueryResult SQLConnection::use(const std::string& sql)
{
	//DEBUG_LOG("%s", sql.c_str());

	UseQueryResult res = m_query.use(sql);
	if (res) {
		// DEBUG_LOG("SQL END: %s", sql.c_str());
		return res;
	}

	if ((m_query.errnum() == CR_SERVER_GONE_ERROR || m_query.errnum() == CR_SERVER_LOST) && connect_to_db()) {
		res = m_query.use(sql);
	}

	if (m_query.errnum()) {
		DEBUG_LOG("use() failed: err=[%d:%s]\n sql=[%s]", m_query.errnum(), m_query.error(), sql.c_str());
	}

	// DEBUG_LOG("SQL END: %s", sql.c_str());
	return res;
}

//---------------------------------------
// Private Methods
//
bool SQLConnection::connect_to_db()
{
	// TODO: set_option应该可以放到构造函数里，无需每次connect都重设
	set_option(new mysqlpp::FoundRowsOption(true));
	set_option(new mysqlpp::SetCharsetNameOption("utf8"));

	bool r = connect("", m_ip.c_str(), m_user.c_str(), m_password.c_str(), m_port);
	if (r) {
		m_query = query();
		// 如果SET autocommit=0失败，基本肯定是数据库连接断开，那后续通过这个SQLConnection对象操作DB肯定会失败，
		// 然后触发connect重新连接，同时重新SET autocommit=0，故此这里无须判断execute返回值
		if (m_use_transaction) {
			execute("SET autocommit=0");
		}
	}
	return r;
}
