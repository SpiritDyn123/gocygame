/*===============================================================
* @Author: car
* @Created Time : 2018年06月12日 星期二 10时40分54秒
*
* @File Name: tbl_corp_nick.cc
* @Description:
*
================================================================*/

#include "tbl_corp_nick.h"
#include "../log.h"
#include <crypt/base64.h>

#define	DB_NAME		"gnick"
#define	TBL_NAME	"corp_nick"

TblCorpNick::TblCorpNick(uint32_t cid)
	: DBTable(cid, DB_NAME, TBL_NAME)
{

}

int TblCorpNick::CheckNick(const std::string& nick, bool& exist)
{
	uint32_t corpId = 0;
	return CheckNick(nick, corpId, exist);
}

int TblCorpNick::CheckNick(const std::string& nick, uint32_t& corpId, bool& exist)
{
	corpId = 0;
	char sql[1024];
	std::string nickBase64 = ant::Base64Encode(nick);
	sprintf(sql, "select corp_id from %s where `nick` = '%s'", get_table(m_uid).c_str(), nickBase64.c_str());

	int retCode = ErrCodeSucc;
	auto res = store2(sql, retCode);
	if (retCode != ErrCodeSucc) {
		return ErrCodeDB;
	}
	if (res.num_rows() != 0) {
		corpId = atoi(res[0][0].c_str());
	}

	exist = (corpId > 0);
	return 0;
}

int TblCorpNick::InsertNick(const std::string& nick)
{
	char sql[1024];
	std::string nickBase64 = ant::Base64Encode(nick);
	sprintf(sql, "insert into %s values('%s', %u)", get_table(m_uid).c_str(), nickBase64.c_str(), m_uid);

	DEBUG_LOG("tbl:%s, plid:%u sql:%s", get_table(m_uid).c_str(), m_uid, sql);
	return insert(sql);
}

int TblCorpNick::DeleteNick(const std::string& nick)
{
	char sql[1024];
	std::string nickBase64 = ant::Base64Encode(nick);
	sprintf(sql, "delete from %s where `nick` = '%s'", get_table(m_uid).c_str(), nickBase64.c_str());

	DEBUG_LOG("tbl:%s, plid:%u sql:%s", get_table(m_uid).c_str(), m_uid, sql);
	execute(sql);

	return 0;
}
