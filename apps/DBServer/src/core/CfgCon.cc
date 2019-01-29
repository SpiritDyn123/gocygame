#include <unistd.h>
#include <serverbench/net_client.hpp>
#include <serverbench/log.hpp>
#include <serverbench/benchapi.hpp>
#include "CfgCon.h"
#include "../proto/ConfigServer.pb.h"
#include "../proto/SvrProtoID.pb.h"
#include <Proto/Proto.h>
#include <libant/crypt/md5.h>

CfgCon::CfgCon()
	: Valid(false)
	, IsConnect(false)
{
}
int CfgCon::Send(uint32_t fd, uint32_t protoId, ::google::protobuf::Message* msg /* = nullptr */)
{
	SSProtoHead h;
	h.ProtoID = protoId;
	h.PlayerID = 0;
	h.SeqNo = 0;
	h.OrigProtoID = 0;
	h.TargetID = 0;
	h.ErrCode = 0;
	h.ProtoLen = sizeof(h);
	if (msg) {
		h.ProtoLen += msg->ByteSize();
	}
	std::string buf;
	buf.resize(h.ProtoLen);
	buf.replace(0, sizeof(h), reinterpret_cast<char*>(&h), sizeof(h));
	if (msg) {
		msg->SerializeWithCachedSizesToArray(reinterpret_cast<uint8_t*>(&(buf[sizeof(h)])));
	}
	return net_client_send(fd, buf.data(), h.ProtoLen);
}

CfgCon::~CfgCon()
{
}

CfgCon & CfgCon::GetInstacne()
{
	static CfgCon con;
	return con;
}

void CfgCon::Connect(const char * addr, short port)
{
	Addr = addr;
	Port = port;
	Valid = true;
	CommitSvrInfo();
}

void CfgCon::SendHeartBeat()
{
	if (!Valid) {
		return;
	}
	auto now = time(nullptr);
	if (now - UpTime < 5) {
		return;
	}

	if (!IsConnect) {
		CommitSvrInfo();
		return;
	}

	UpTime = now;

	for (auto fd : Fds) {
		if (Send(fd, ConfigProtoHeartBeat) < 0) {
			IsConnect = false;
			break;
		}
	}
}

void CfgCon::CommitSvrInfo()
{
	Fds.clear();
	int cnt = sevrer_get_bind_cnt();

	for (int i = 0; i < cnt; ++i) {
		auto fd = net_client_new(Addr.c_str(), Port);
		Fds.push_back(fd);
		if (fd <= 0) {
			break;
		}
		const char* addr = server_get_bind_addr(i);
		int port = server_get_bind_port(i);
		config::SvrInfoReq req;
		req.set_svr_type(cs::SvrTypeDBServer);
		req.set_svr_ip(addr);
		req.set_svr_port(port);
		req.set_svr_id(server_get_bind_id(i));
		req.set_tag(server_get_bind_tag(i));

		Send(fd, ConfigProtoSvrInfo, &req);
		DEBUG_LOG("Send Svr Info Svr: %s:%d", addr, port);
	}
	IsConnect = true;
	UpTime = time(nullptr);

	CommitCfgMd5();
}

void CfgCon::CommitCfgMd5()
{
	const char* mysqlCfg = config_get_strval("mysqlCfg");
	if (mysqlCfg) {
		auto md5 = ant::GetFileMd5(mysqlCfg);
		DEBUG_LOG("Cfg Md5: %s", md5.c_str());
		config::CheckCfgMd5 req;
		req.set_md5(md5);
		for (auto fd : Fds) {
			Send(fd, ConfigProtoCheckCfgMD5, &req);
		}
	}
}
