#include "tbl_login_record.h"
#include <sstream>
#include "../log.h"

int TblLoginRecord::AddLoginRecord(std::string ip)
{
	char sql[1024];
	sprintf(sql, "insert into %s(`pid`, `ip`, `time`) values ( %u, '%s', %u)",
		get_table(m_uid).c_str(), m_uid, ip.c_str(), uint32_t(time(0)));
	execute(sql);
	return 0;
}

int TblLoginRecord::GetLoginRecords(uint32_t index, uint32_t limit,
	const std::function<void(std::string ip, uint32_t time_stamp)>& fn)
{
	// 30天之内的记录
	int total = 0;
	uint32_t timeBeg = time(0) - 30 * 24 * 60 * 60;
	uint32_t timeEnd = time(0);
	uint32_t indexBeg = index * limit;
	uint32_t indexEnd = indexBeg + limit;
	char sql[1024];
	sprintf(sql, "select count(*) from %s where `pid` = %u and `time` >= %u and `time` <= %u",
		get_table(m_uid).c_str(), m_uid, timeBeg, timeEnd);
	auto ret = store(sql);
	if (ret.num_rows() != 0) {
		auto& info = ret[0][0];
		total = atoi(info.c_str());
	}

	memset(sql, 0, sizeof(char) * 1024);

	sprintf(sql, "select `ip`, `time` from %s where `pid` = %u and `time` >= %u and `time` <= %u order by `time` desc limit %d, %d",
		get_table(m_uid).c_str(), m_uid, timeBeg, timeEnd, indexBeg, indexEnd);
	auto ret1 = store(sql);
	for (auto& it : ret1) {
		fn(it[0].c_str(), atoi(it[1].c_str()));
	}
	return total;
}