#include "tbl_spam.h"
#include <sstream>
#include "../log.h"
 
int TblSpam::AddSpamRecord(const std::string& name, uint32_t pid, const std::string& channel, uint32_t ban_time,
	const std::string& content, uint32_t result, uint32_t ban_type)
{
	char sql[1024];
	sprintf(sql, "insert into %s(`name`, `pid`, `channel`, `ban_time`, `content`, `result`, `ban_type`) values ( '%s', %d, '%s', %d, '%s', %d, %d)"
		, get_table(m_uid).c_str(), name.c_str(), pid, channel.c_str(), ban_time, content.c_str(), result, ban_type);
	execute(sql);
	return 0;
}

int TblSpam::UpdateSpamRecord(uint32_t id, uint32_t state)
{
	char sql[1024];
	sprintf(sql, "update %s set `read` = %d, `state` = %d where `id` = %d"
		, get_table(m_uid).c_str(), 1, state, id);
	execute(sql);
	return 0;
}

int TblSpam::GetSpamRecords(uint32_t index, uint32_t limit,
	const std::function<void(uint32_t id, uint32_t read, uint32_t state, std::string name, uint32_t pid, std::string channel,
		uint32_t ban_time, std::string content, uint32_t result, uint32_t ban_type)>& fn)
{
	int total = 0;
	char sql[1024];
	sprintf(sql, "select count(*) from %s", get_table(m_uid).c_str());
	auto ret = store(sql);
	if (ret.num_rows() != 0) {
		auto& info = ret[0][0];
		total = atoi(info.c_str());
	}
	memset(sql, 0, sizeof(char) * 1024);
	uint32_t indexBeg = index * limit;
	uint32_t indexEnd = indexBeg + limit;
	sprintf(sql, "select `id`, `read`, `state`, `name`, `pid`, `channel`, `ban_time`, `content`, `result`, `ban_type` from %s limit %d, %d"
		, get_table(m_uid).c_str(), indexBeg, indexEnd);
	auto ret1 = store(sql);
	for (auto& it : ret1) {
		fn(atoi(it[0].c_str()), atoi(it[1].c_str()), atoi(it[2].c_str()), it[3].c_str(), atoi(it[4].c_str()), 
			it[5].c_str(), atoi(it[6].c_str()), it[7].c_str(), atoi(it[8].c_str()), atoi(it[9].c_str()));
	}
	return total;
}