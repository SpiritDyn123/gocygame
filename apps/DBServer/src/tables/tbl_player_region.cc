/*===============================================================
* @Author: car
* @Created Time : 2018年06月12日 星期二 10时40分54秒
*
* @File Name: tbl_player_region.cc
* @Description:
*
================================================================*/

#include "tbl_player_region.h"
#include "../log.h"

#define	DB_NAME		"account"
#define	TBL_NAME	"player_region"

TblPlayerRegion::TblPlayerRegion(uint32_t uid)
	: DBTable(uid, DB_NAME, TBL_NAME)
{

}

int TblPlayerRegion::GetPlayerRegionInfo(const std::string& channel, const std::string& channelId, google::protobuf::RepeatedPtrField<db::PlayerRegionInfo>* pInfos)
{
	char sql[1024];
	sprintf(sql, "select region, player_id, last_login_tm from %s where channel = '%s' and channel_id = '%s'", 
		get_table(m_uid).c_str(), channel.c_str(), channelId.c_str());

	int retCode = ErrCodeSucc;
	auto res = store2(sql, retCode);
	if (retCode != ErrCodeSucc) {
		return ErrCodeDB;
	}

	for (uint32_t i = 0; i < res.num_rows(); i++) {
		auto pInfo = pInfos->Add();
		pInfo->set_region(atoi(res[i][0].c_str()));
		pInfo->set_player_id(atoi(res[i][1].c_str()));
		pInfo->set_last_login(atoi(res[i][2].c_str()));
	}

	return 0;
}

int TblPlayerRegion::PlayerCreate(const std::string& channel, const std::string& channelId, uint32_t plid, uint32_t region)
{
	uint32_t nowTm = time(0);
	char sql[1024];
	sprintf(sql, "insert into %s values('%s', '%s', %u, %u, %u, %u)", 
		get_table(m_uid).c_str(), 
		channel.c_str(), channelId.c_str(), 
		region, plid, nowTm, nowTm);

	DEBUG_LOG("tbl:%s, plid:%u sql:%s", get_table(m_uid).c_str(), m_uid, sql);
	return insert(sql);
}

int TblPlayerRegion::PlayerLogin(const std::string& channel, const std::string& channelId, uint32_t plid, uint32_t region)
{
	uint32_t nowTm = time(0);
	char sql[1024];
	sprintf(sql, "update %s set last_login_tm = %u where channel = '%s' and channel_id = '%s' and region = %u",
		get_table(m_uid).c_str(), nowTm, channel.c_str(), channelId.c_str(), region);

	DEBUG_LOG("tbl:%s, plid:%u sql:%s", get_table(m_uid).c_str(), m_uid, sql);
	return update_one(sql);
}
