/*===============================================================
* @Author: car
* @Created Time : 2018年03月06日 星期二 14时43分27秒
*
* @File Name: handlers/Questionnaire.cc
* @Description:
*
================================================================*/
#include <string>
#include <sstream>
#include "Questionnaire.h"
#include "../core/dispatcher.h"
#include "../core/redis_client.h"
#include "../core/MysqlProxy.h"
#include "../proto/SvrProtoID.pb.h"
#include "../proto/db.pb.h"
#include <StringUtils.h>
#include "../tables/tbl_questionnaire.h"
#include "../core/MysqlProxy.h"
#include <serverbench/benchapi.hpp>
#include "../global.h"


static Questionnaire gQuestionnaire;

Questionnaire::Questionnaire()
{
	gMsgDispatcher.RegisterHandler(DBProtoQuestionnaireAdd, *this, &Questionnaire::questionnaireAdd, new db::QuestionnaireInfo);
}


void Questionnaire::Init()
{
	dbPrefix_ = MysqlProxy::Instance().GetKv("globalDB");
	tblPrefix_ = "qa_info";
}

ErrCodeType Questionnaire::questionnaireAdd(const SSProtoHead & h, google::protobuf::Message * inMsg, google::protobuf::Message * outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::QuestionnaireInfo, req);
	TblQuestionnaire tblQue(req.plid(), dbPrefix_.c_str(), tblPrefix_.c_str());
	for (int i = 0; i < req.ans_size(); i++) {
		tblQue.Add(req.qa_id(), req.ans(i), i + 1);
	}
	tblQue.UpInfo(req.name());

	return ErrCodeType::ErrCodeSucc;
}
