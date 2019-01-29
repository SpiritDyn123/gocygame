#include "tbl_lottery.h"
#include <sstream>
#include "../log.h"

int TblLotteryRecord::AddLotteryRecord(uint32_t type, std::string& data)
{
	char sql[1024];
	sprintf(sql, "insert into %s(`pid`, `type`, `time`, `data`) values ( %u, %d, %d, '"
		, get_table(m_uid).c_str(), m_uid, type, uint32_t(time(0)));
	auto query = get_query();
	query << sql << mysqlpp::escape << data << "')";
	execute(query.str());
	return 0;
}

int TblLotteryRecord::GetLotteryRecords(uint32_t type, uint32_t index, uint32_t limit, uint32_t time_beg, uint32_t time_end,
	const std::function<void(std::string& data)>& fn)
{
	int total = 0;
	char sql[1024];
	uint32_t indexBeg = index * limit;
	uint32_t indexEnd = indexBeg + limit;
	sprintf(sql, "select count(*) from %s where `pid` = %u and `type` = %d and `time` >= %u and `time` <= %u",
		get_table(m_uid).c_str(), m_uid, type, time_beg, time_end);
	auto ret = store(sql);
	if (ret.num_rows() != 0) {
		auto& info = ret[0][0];
		total = atoi(info.c_str());
	}

	memset(sql, 0, sizeof(char) * 1024);
	
	sprintf(sql, "select data from %s where `pid` = %u and `type` = %d and `time` >= %u and `time` <= %u order by `time` desc limit %d, %d", 
		get_table(m_uid).c_str(), m_uid, type, time_beg, time_end, indexBeg, indexEnd);
	auto ret1 = store(sql);
	for (auto& it : ret1) {
		auto& info = it[0];
		std::string data(info.data(), info.size());
		fn(data);
	}
	return total;
}