#include <cerrno>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <serverbench/benchapi.hpp>

#include "core/dispatcher.h"
#include "core/redis_client.h"
#include "core/sqlconnection.h"
#include "core/ProxyConnector.h"
#include "core/CfgCon.h"
#include <StringUtils.h>
#include "core/MysqlProxy.h"
#include "core/ScriptMgr.h"
//#include "tables/tbl_data_map.h"
//#include "tables/tbl_data.h"
#include "handlers/Arena.h"
#include "handlers/PlayerData.h"
#include "CSV/CSVManager.h"
#include "handlers/BtlReplay.h"
#include "handlers/CapRes.h"
#include "handlers/PayData.h"
#include "global.h"
#include "handlers/BehaviourRank.h"
#include "handlers/Questionnaire.h"
#include "handlers/MsgHandler.h"

using namespace std;
using namespace google::protobuf;

extern "C" int handle_init(int argc, char** argv, int proc_type)
{
	switch (proc_type) {
	case PROC_MAIN: // 监控进程
		return 0;
	case PROC_WORK: { // 工作进程
		srand(getpid());

		gMsgDispatcher.PrintSupportedProtos();

		// redis info
		string redisHost = config_get_strval("redis");
		string redisPasswd;
		if (config_get_strval("redis_passwd")) {
			redisPasswd = config_get_strval("redis_passwd");
		}
		gRedis = new RedisClient(redisHost, redisPasswd);

		// redis rank info
		string redisRankHost;
		string redisRankPasswd;
		const char* redisRankHostStr = config_get_strval("redis_rank");
		const char* redisRankPasswdStr = config_get_strval("redis_rank_passwd");
		redisRankHost = redisRankHostStr ? redisRankHostStr : redisHost;
		redisRankPasswd = redisRankPasswdStr ? redisRankPasswdStr : redisPasswd;
		gRedisRank = new RedisClient(redisRankHost, redisRankPasswd);

		// redis battle info
		string redisBtlHost;
		string redisBtlPasswd;
		const char* redisBtlHostStr = config_get_strval("redis_battle");
		const char* redisBtlPasswdStr = config_get_strval("redis_battle_passwd");
		redisBtlHost = redisBtlHostStr ? redisBtlHostStr : redisHost;
		redisBtlPasswd = redisBtlPasswdStr ? redisBtlPasswdStr : redisPasswd;
		gRedisBattle = new RedisClient(redisBtlHost, redisBtlPasswd);

		// mysql info
		const char* mysqlCfg = config_get_strval("mysqlCfg");
		if (!mysqlCfg) {
			ERROR_LOG("mysqlCfg not define");
			return -1;
		}

		MysqlProxy::Instance().LoadMysqlCfg(mysqlCfg);

		ScriptMgr::Instance().Load("script");
//		g_sql = new SQLConnection(config_get_strval("dbuser"), config_get_strval("dbpasswd"),
//		                          config_get_intval("use_transaction", 1), config_get_strval("dbhost"));
//		g_data = new TblData(*g_sql, "t_data");
		CSVManager::Load(config_get_strval("csv_path"));
		MsgHdlMgr::Instance().Init();

		/*
		gPlayerData.Init();
		gArena.Init();
		gBtlReplay.Init();
		gCapResData.Init();
		gPayData.Init();
		gBehaviourRank.Init();
		gQuestionnaire.Init();
		*/
#if 0 
		for (int i=0 ; i< 1000; i++) {
			gCapResData.BuildNpcResTest(500, 1);
		}
#endif


		return 0;
	}
	case PROC_CONN: // 网络进程
		{
			auto cfgAddr = config_get_strval("config_svr");
			if (cfgAddr) {
				auto addrArr = Split(cfgAddr, ':');
				if (addrArr.size() >= 2) {
					CfgCon::GetInstacne().Connect(addrArr[0].c_str(), atoi(addrArr[1].c_str()));
				}
			}
		}
		
		return 0;
	case PROC_TIME: // 定时器进程
		return 0;
	default:
		ERROR_LOG("invalid proc_type=%d", proc_type);
		return -1;
	}

	return -1;
}

extern "C" void handle_fini(int proc_type)
{
	switch (proc_type) {
	case PROC_MAIN: // 监控进程
		break;
	case PROC_WORK: // 工作进程
//		delete g_datamap;
//		delete g_data;
//		delete g_data_hour;
//		delete g_data_minute;
//		delete g_sql;
		delete gRedis;
		delete gRedisRank;
		delete gRedisBattle;
		break;
	case PROC_CONN: // 网络进程
		break;
	case PROC_TIME: // 定时器进程
		break;
	default:
		ERROR_LOG("invalid proc_type=%d", proc_type);
		break;
	}
}

extern "C" int handle_input(const char* buffer, int length, const skinfo_t* sk, int proc_type)
{
	return gMsgDispatcher.GetProtoLen(buffer, length, sk);
}

extern "C" int handle_filter_key(const char* buf, int len, uint32_t* key)
{
	*key = gMsgDispatcher.GetHashCode(buf, len);
	return 0;
}

static string output_buf;
// 返回-1直接关闭连接，0正常返回，1返回后关闭连接。一般只需要使用0和-1。
extern "C" int handle_process(char* recvbuf, int rcvlen, char** sendbuf, int* sndlen, const skinfo_t* sk)
{
	output_buf.clear();
	int ret = gMsgDispatcher.Dispatch(recvbuf, output_buf);
	if (ret >= 0) {
		*sendbuf = const_cast<char*>(output_buf.data());
		*sndlen = output_buf.size();
	}

	return ret;
}

extern "C" int handle_timer(int* intvl)
{
	*intvl = config_get_intval("statInterval", 10);
	static stat_info_t info;
	g_stat.update_info_interval();
	g_stat.get_stat_info(info);
	INFO_LOG("stat info:[s_queue:used=%u%% in=%u out=%u, r_queue:used=%u%% in=%u out=%u, filter:%u %u %u]",
		info.s_queue.used_rate, info.s_queue.push_cnt, info.s_queue.pop_cnt, 
		info.r_queue.used_rate, info.r_queue.push_cnt, info.r_queue.pop_cnt,
		info.filter.push_cnt, info.filter.pop_cnt, info.filter.cur_size);
	return 0;
}

extern "C" int handle_open(char** buf, int* len, const skinfo_t* sk)
{
	return 0;
}

extern "C" int handle_close(const skinfo_t* sk)
{
	return 0;
}

extern "C" int handle_con()
{
	CfgCon::GetInstacne().SendHeartBeat();
	return 0;
}
