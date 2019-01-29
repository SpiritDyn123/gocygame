/**
 * @file sqlconnection.h
 *
 * 对mysqlpp::Connection做了简单封装，保存了连接数据库用的ip、port、user和password。
 *
 */

#ifndef DIGIMON_DBSVR_SQLCONNECTION_H_
#define DIGIMON_DBSVR_SQLCONNECTION_H_

#include <string>

#include <mysql/errmsg.h>
#include <mysql/mysqld_error.h>
#include <mysql++/mysql++.h>

/**
 * @brief 对mysqlpp::Connection做了简单封装，保存了连接数据库用的ip、port、user和password。
 */
class SQLConnection : public mysqlpp::Connection {
public:
	/**
	 * @brief 构造函数
	 * @param user 用户名
	 * @param password 密码
	 * @param ip ip，默认localhost
	 * @param port 端口，默认3306
	 * @param use_transaction 是否使用事务
	 */
	SQLConnection(const std::string& user, const std::string& password,
	              bool use_transaction = true,
	              const std::string& ip = "localhost", unsigned int port = 3306);
	/**
	 * @brief 进行转义操作
	 * @param s 保存转义后的字符。如果s2 == 0，则s既是入参，也是出参
	 * @param s2 待转义字符
	 */
	void escape_string(std::string& s, const std::string* s2 = 0) const
	{
		if (s2 == 0) {
			m_query.escape_string(&s);
		} else {
			m_query.escape_string(&s, s2->c_str(), s2->size());
		}
	}

	/**
	 * @brief 执行SQL，并获取简单的返回结果，比如自增ID、影响行数等
	 */
	mysqlpp::SimpleResult execute(const std::string& sql);
	/**
	 * @brief 执行SQL，并把结果都保存到StoreQueryResult对象中。这个函数使用方便，但是如果一次返回记录数过多，会大大影响效率。
	 */
	mysqlpp::StoreQueryResult store(const std::string& sql);
	/**
	 * @brief 执行SQL，返回结果通过遍历UseQueryResult获取。如果一次返回记录数较多，建议使用这个函数。
	 */
	mysqlpp::UseQueryResult use(const std::string& sql);

	/**
	 * @brief 提交修改。如果创建SQLConnection时选择了不使用事务，则该函数不做任何操作。
	 * @return 成功返回true，失败返回false
	 */
	bool commit()
	{
		if (m_use_transaction) {
			return execute("COMMIT");
		}
		return true;
	}

	/**
	 * @brief 回退修改。如果创建SQLConnection时选择了不使用事务，则该函数不做任何操作。
	 */
	void rollback()
	{
		if (m_use_transaction) {
			execute("ROLLBACK");
		}
	}

private:
	/**
	 * @brief 和数据库建立连接。如果之前已经建立好了连接，再次调用此函数会导致之前的连接被关闭，然后重新建立一个新的连接。
	 * @return 成功返回true，失败返回false。
	 */
	bool connect_to_db();

private:
	mysqlpp::Query m_query;

	std::string m_user;
	std::string m_password;
	std::string m_ip;
	unsigned int m_port;
	bool m_use_transaction;
};

/*! 和数据库的连接，通过该连接操作数据库 */
extern SQLConnection* g_sql;

#endif /* DIGIMON_DBSVR_SQLCONNECTION_H_ */
