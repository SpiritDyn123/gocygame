/*===============================================================
* @Author: car
* @Created Time : 2018年06月12日 星期二 10时40分54秒
*
* @File Name: tbl_player_nick.cc
* @Description:
*
================================================================*/

#include "tbl_player_nick.h"
#include "../log.h"
#include <crypt/base64.h>

#define	DB_NAME		"gnick"
#define	TBL_NAME	"player_nick"

TblPlayerNick::TblPlayerNick(uint32_t uid)
	: DBTable(uid, DB_NAME, TBL_NAME)
{

}

int TblPlayerNick::CheckNick(const std::string& nick, bool& exist)
{
	uint32_t plid = 0;
	uint32_t expireTm = 0;
	return CheckNick(nick, plid, expireTm, exist);
}

int TblPlayerNick::CheckNick(const std::string& nick, uint32_t& plid, bool& exist)
{
	uint32_t expireTm = 0;
	return CheckNick(nick, plid, expireTm, exist);
}

int TblPlayerNick::CheckNick(const std::string& nick, uint32_t& plid, uint32_t& expireTm, bool& exist)
{
	//ClearExpireNick();

	plid = 0;
	expireTm = 0;

	char sql[1024];
	std::string nickBase64 = ant::Base64Encode(nick);
	sprintf(sql, "select player_id, expire_tm from %s where `nick` = '%s'", get_table(m_uid).c_str(), nickBase64.c_str());

	int retCode = ErrCodeSucc;
	auto res = store2(sql, retCode);
	if (retCode != ErrCodeSucc) {
		return ErrCodeDB;
	}

	if (res.num_rows() != 0) {
		plid = atoi(res[0][0].c_str());
		expireTm = atoi(res[0][1].c_str());
	}

	exist = (plid > 0);

	if (exist && expireTm > 0 && expireTm < time(0)) {
		DeleteNick(nick);
		exist = false;
	}


	return 0;
}

int TblPlayerNick::InsertNick(const std::string& nick, uint32_t expireTm)
{
	//ClearExpireNick();

	char sql[1024];
	std::string nickBase64 = ant::Base64Encode(nick);
	sprintf(sql, "insert into %s values('%s', %u, %u)", get_table(m_uid).c_str(), nickBase64.c_str(), m_uid, expireTm);

	DEBUG_LOG("tbl:%s, plid:%u sql:%s", get_table(m_uid).c_str(), m_uid, sql);
	return insert(sql);
}

int TblPlayerNick::DeleteNick(const std::string& nick)
{
	char sql[1024];
	std::string nickBase64 = ant::Base64Encode(nick);
	sprintf(sql, "delete from %s where `nick` = '%s'", get_table(m_uid).c_str(), nickBase64.c_str());

	DEBUG_LOG("tbl:%s, plid:%u sql:%s", get_table(m_uid).c_str(), m_uid, sql);
	execute(sql);

	return 0;
}

int TblPlayerNick::UpdateExpireTm(const std::string& nick, uint32_t expireTm)
{
	char sql[1024];
	std::string nickBase64 = ant::Base64Encode(nick);
	sprintf(sql, "replace into %s values('%s', %u, %u)",
		get_table(m_uid).c_str(), nickBase64.c_str(), m_uid, expireTm);

	DEBUG_LOG("tbl:%s, plid:%u sql:%s", get_table(m_uid).c_str(), m_uid, sql);
	execute(sql);
	return 0;
}

int TblPlayerNick::ClearExpireNick()
{
	char sql[1024];
	uint32_t nowTm = time(0);
	sprintf(sql, "delete from %s where `expire_tm` > 0 and `expire_tm` < %u", get_table(m_uid).c_str(), nowTm);

	DEBUG_LOG("tbl:%s, plid:%u sql:%s", get_table(m_uid).c_str(), m_uid, sql);
	execute(sql);

	return 0;
}

