/*===============================================================
* @Author: car
* @Created Time : 2018年03月06日 星期二 14时33分34秒
*
* @File Name: handlers/Questionnaire.h
* @Description:
*
================================================================*/

#ifndef _QUESIONNAIRE_H_
#define _QUESIONNAIRE_H_

#include <Proto/Proto.h>
#include "../proto/ErrCode.pb.h"
#include "../core/sqlconnection.h"
#include "MsgHandler.h"

class Questionnaire
	: public MsgHandler
{
public:
	Questionnaire();
	void Init() override;
private:
	ErrCodeType questionnaireAdd(const SSProtoHead & h, google::protobuf::Message * inMsg, google::protobuf::Message * outMsg);

private:
	std::string dbPrefix_;
	std::string tblPrefix_;
};


#endif  // _QUESIONNAIRE_H_
