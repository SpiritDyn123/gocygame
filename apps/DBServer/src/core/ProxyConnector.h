/*
 * ProxyConnector.h
 *
 *  Created on: 2017年2月15日
 *      Author: antigloss
 */

#ifndef DIGIMON_SRC_PROXYCONNECTOR_H_
#define DIGIMON_SRC_PROXYCONNECTOR_H_

#include <vector>
#include "ServerConnector.h"

/**
 * @brief 封装和Proxy的通讯
 */
class ProxyConnector {
public:
	/**
	 * @brief 构造函数
	 */
	ProxyConnector(const std::string& hosts);
	/**
	 * @brief 析构函数
	 */
	~ProxyConnector();

	// 给后端机器发请求。
	bool SendMsgAndDropReply(uint16_t protoID, uint32_t playerID, uint32_t targetID, const google::protobuf::Message& msg);

private:
	std::vector<ServerConnector*>	connectors_;
};

extern ProxyConnector* gProxyConn;

#endif /* DIGIMON_SRC_PROXYCONNECTOR_H_ */
