/*
 * ProxyConnector.cc
 *
 *  Created on: 2017年2月15日
 *      Author: antigloss
 */

#include <cstdlib>
#include <algorithm>
#include <stdexcept>

#include <StringUtils.h>
#include "ProxyConnector.h"

using namespace std;

ProxyConnector* gProxyConn;

//----------------------------------------------------------
// Public Methods
//----------------------------------------------------------
ProxyConnector::ProxyConnector(const std::string& hosts)
{
	auto ipPorts = Split(hosts, ';');
	if (ipPorts.empty()) {
		throw invalid_argument("Invalid hosts: " + hosts);
	}

	for (const auto& ipPort : ipPorts) {
		connectors_.emplace_back(new ServerConnector("Proxy", ipPort));
	}
	random_shuffle(connectors_.begin(), connectors_.end());
}

ProxyConnector::~ProxyConnector()
{
	for (auto conn : connectors_) {
		delete conn;
	}
}

bool ProxyConnector::SendMsgAndDropReply(uint16_t protoID, uint32_t playerID, uint32_t targetID, const google::protobuf::Message& msg)
{
	for (auto conn : connectors_) {
		if (conn->SendMsgAndDropReply(protoID, playerID, targetID, msg)) {
			return true;
		}
	}

	return false;
}

//----------------------------------------------------------
// Private Methods
//----------------------------------------------------------
