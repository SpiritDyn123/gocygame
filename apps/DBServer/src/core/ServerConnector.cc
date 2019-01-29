/*
 * ServerConnector.cc
 *
 *  Created on: 2017年2月15日
 *      Author: antigloss
 */

#include <stdexcept>
#include <serverbench/benchapi.hpp>

#include <Proto/Proto.h>
#include <StringUtils.h>
#include "ServerConnector.h"

using namespace std;

//----------------------------------------------------------
// Public Methods
//----------------------------------------------------------
ServerConnector::ServerConnector(const string& svrName, const string& host)
	: svrName_(svrName)
{
	buf_.reserve(1024 * 1024);

	auto ipPort = Split(host, ':');
	if (ipPort.size() != 2) {
		throw invalid_argument("Invalid host: " + host);
	}
	ip_ = ipPort[0];
	port_ = atoi(ipPort[1].c_str());
	sockfd_ = -1;
}

ServerConnector::~ServerConnector()
{
	close();
}

bool ServerConnector::SendMsg(uint16_t protoID, uint32_t playerID, uint32_t targetID)
{
	if (!Connected()) { // 尝试重连
		connect();
	}
	if (!Connected()) {
		DEBUG_LOG("failed to connect to %s at %s:%d", svrName_.c_str(), ip_.c_str(), port_);
		return false;
	}

	SSProtoHead h;
	h.ProtoLen = sizeof(h);
	h.ProtoID = protoID;
	h.PlayerID = playerID;
	h.SeqNo = 0;
	h.OrigProtoID = 0;
	h.TargetID = targetID;
	h.ErrCode = 0;

	int totalLen = h.ProtoLen;
	if (ant::safe_tcp_send_n(sockfd_, &h, totalLen) == totalLen) {
		return true;
	}

	DEBUG_LOG("failed to send to %s at %s:%d", svrName_.c_str(), ip_.c_str(), port_);
	close();
	return false;
}

bool ServerConnector::SendMsg(uint16_t protoID, uint32_t playerID, uint32_t targetID, const google::protobuf::Message& msg)
{
	if (!Connected()) { // 尝试重连
		connect();
	}
	if (!Connected()) {
		DEBUG_LOG("failed to connect to %s at %s:%d", svrName_.c_str(), ip_.c_str(), port_);
		return false;
	}

	SSProtoHead h;
	h.ProtoLen = sizeof(h) + msg.ByteSize();
	h.ProtoID = protoID;
	h.PlayerID = playerID;
	h.SeqNo = 0;
	h.OrigProtoID = 0;
	h.TargetID = targetID;
	h.ErrCode = 0;

	buf_.resize(h.ProtoLen);
	buf_.replace(0, sizeof(h), reinterpret_cast<char*>(&h), sizeof(h));
	msg.SerializeWithCachedSizesToArray(reinterpret_cast<uint8_t*>(&(buf_[sizeof(h)])));

	int totalLen = h.ProtoLen;
	if (ant::safe_tcp_send_n(sockfd_, buf_.data(), totalLen) == totalLen) {
		return true;
	}

	DEBUG_LOG("failed to send to %s at %s:%d", svrName_.c_str(), ip_.c_str(), port_);
	close();
	return false;
}

//----------------------------------------------------------
// Private Methods
//----------------------------------------------------------
