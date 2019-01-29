/*===============================================================
* @Author: car
* @Created Time : 2017年04月21日 星期五 18时27分26秒
*
* @File Name: MailData.h
* @Description:
*
================================================================*/
#ifndef _DIGIMON_MAILDATA_H_
#define _DIGIMON_MAILDATA_H_

#include "../proto/ErrCode.pb.h"
#include "../proto/db.pb.h"

class MailData {
private:
	enum {
		kMaxMailLength = 1000,
		kLimitGMailLen = 2000,
	};
public:
	MailData();

public:
	ErrCodeType PackMail(const SSProtoHead& h, uint32_t reg_tm, google::protobuf::Message* out, uint32_t maxId = 0);
	ErrCodeType AddMail(db::AddMailReq& req, db::AddMailRsp* resp = nullptr);
private:
	ErrCodeType addMail(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType updateMail(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType listMail(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType addGMMail(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);

private:
	ErrCodeType getPlayerMailUID(std::string& key, uint32_t& uID);
	ErrCodeType getPlayerMailGUID(std::string& key, uint32_t& gUID);
	ErrCodeType getGlobalMailGUID(uint32_t& gUID);
	ErrCodeType doAddMail(std::string& key, std::string keyFieldUID, uint32_t targetID, google::protobuf::Message* inMsg, google::protobuf::Message* ret);
	void CleanGlobalMail();

private:
	static const std::string kFieldMailUIDKey;
	static const std::string kFieldMailGUIDKey;
	static const std::string kFieldGMailGUIDKey;
};

void MailAddAttach(::google::protobuf::RepeatedPtrField< ::cs::RewardItem >* rewards, const std::vector<int>& item);
extern MailData gMailData;

#endif /* DIGIMON_MAILDATA_H_  */


