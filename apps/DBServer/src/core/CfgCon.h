#pragma once
#include <string>
#include <time.h>
#include <vector>
#include <google/protobuf/message.h>

class CfgCon
{
public:
	~CfgCon();
	static CfgCon& GetInstacne();
	void Connect(const char* addr, short port);
	void SendHeartBeat();
	void CommitSvrInfo();
	void CommitCfgMd5();
private:
	CfgCon();
	int Send(uint32_t fd, uint32_t protoId, ::google::protobuf::Message* msg = nullptr);
private:
	std::string Addr;
	short Port;

	std::vector<int> Fds;
	time_t UpTime;
	bool Valid;
	bool IsConnect;
};

