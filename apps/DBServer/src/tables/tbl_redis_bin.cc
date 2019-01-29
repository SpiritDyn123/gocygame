#include "tbl_redis_bin.h"
#include "../core/redis_client.h"
#include "../log.h"
#include <libant/crypt/md5.h>

int TblRedisBin::Get(std::unordered_map<std::string, std::string>& fields)
{
	char sql[1024];
	sprintf(sql, "select `data` from %s where `id` = %d limit 1", get_table(m_uid).c_str(), m_uid);
	auto ret = store(sql);
	if (ret.size() != 0) {
		auto& info = ret[0][0];
		UnpackBinToDict(info.c_str(), info.length(), fields);
		//for test
		//for (auto& it : fields) {
		//	auto md5val = ant::GetStrMd5(it.second);
		//	DEBUG_LOG("mysql decode key:[%s], md5[%s]", it.first.c_str(), md5val.c_str());
		//}
	}
	else {
		DEBUG_LOG("sql:%s", sql);
	}
	return 0;
}

