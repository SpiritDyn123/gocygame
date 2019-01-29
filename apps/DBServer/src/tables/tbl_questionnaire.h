/*===============================================================
* @Author: car
* @Created Time : 2018年03月06日 星期二 14时11分12秒
*
* @File Name: tbl_questionnaire.h
* @Description:
*
================================================================*/
#pragma once

#include "../core/dbtable.h"

class TblQuestionnaire
	: public DBTable<1, 1> 
{
public:
	using DBTable::DBTable;
	//TblQuestionnaire(SQLConnection& conn, const char* db, const char* tbl);
	int Add(uint32_t qaid, const std::string& ans, uint32_t seq);
	int UpInfo(const std::string& key);

};

