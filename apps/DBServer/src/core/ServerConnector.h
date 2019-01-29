/*
 * ServerConnector.h
 *
 *  Created on: 2017年2月15日
 *      Author: antigloss
 */

#ifndef DIGIMON_SRC_SERVERCONNECTOR_H_
#define DIGIMON_SRC_SERVERCONNECTOR_H_

#include <cassert>
#include <string>
#include <unordered_map>
#include <unistd.h>
#include <google/protobuf/message.h>
#include <libant/inet/tcp.h>

class ServerConnector {
public:
	ServerConnector(const std::string& svrName, const std::string& host);
	~ServerConnector();

	// 禁止复制
	ServerConnector(const ServerConnector&) = delete;
	ServerConnector& operator=(const ServerConnector&) = delete;

	bool Connect()
	{
		close();
		return connect();
	}

	// 判断是否已经建立连接
	bool Connected() const
	{
		return sockfd_ >= 0;
	}

	// 给后端机器发请求
	bool SendMsg(uint16_t protoID, uint32_t playerID, uint32_t targetID);
	bool SendMsg(uint16_t protoID, uint32_t playerID, uint32_t targetID, const google::protobuf::Message& msg);

	bool SendMsgAndDropReply(uint16_t protoID, uint32_t playerID, uint32_t targetID)
	{
		if (SendMsg(protoID, playerID, targetID)) {
			close();
			return true;
		}
		return false;
	}

	bool SendMsgAndDropReply(uint16_t protoID, uint32_t playerID, uint32_t targetID, const google::protobuf::Message& msg)
	{
		if (SendMsg(protoID, playerID, targetID, msg)) {
			close();
			return true;
		}
		return false;
	}

private:
	bool connect()
	{
		sockfd_ = ant::safe_tcp_connect(ip_.c_str(), port_, 1);
		if (sockfd_ >= 0) {
			return true;
		}
		return false;
	}

	void close()
	{
		if (sockfd_ >= 0) {
			::close(sockfd_);
			sockfd_ = -1;
		}
	}

private:
	std::string		svrName_;
	std::string		buf_;
    std::string		ip_;
    in_addr_t		port_;
    int				sockfd_;
};

#endif /* DIGIMON_SRC_SERVERCONNECTOR_H_ */
