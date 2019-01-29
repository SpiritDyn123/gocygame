/*===============================================================
* @Author: car
* @Created Time : 2017年04月21日 星期五 18时27分34秒
*
* @File Name: MailData.cc
* @Description:
*
================================================================*/

#include <sstream>
#include <string>
#include "../proto/SvrProtoID.pb.h"
#include "../proto/db.pb.h"
#include "../proto/CSCoreMsg.pb.h"
#include "../core/dispatcher.h"
#include "../core/redis_client.h"

#include "KeyPrefixDef.h"
#include "MailData.h"
#include "Arena.h"
#include "BehaviourRank.h"
#include "PlayerData.h"

using namespace std;

const std::string MailData::kFieldMailUIDKey(kKeyPrefixMailData + "UID");
const std::string MailData::kFieldMailGUIDKey(kKeyPrefixMailData + "GUID");
const std::string MailData::kFieldGMailGUIDKey(kKeyGlobalMailData + "UID");


void MailAddAttach(::google::protobuf::RepeatedPtrField< ::cs::RewardItem >* rewards, const std::vector<int>& item)
{
	if (item.size() < 3) {
		return;
	}
	auto attach = rewards->Add();
	attach->set_type((cs::PlayerResourceType)item[0]);
	attach->set_id(item[1]);
	attach->set_cnt(item[2]);
	for (size_t i = 3; i < item.size(); ++i) {
		attach->add_extra_datas(item[i]);
	}
}

MailData gMailData;


MailData::MailData()
{
	gMsgDispatcher.RegisterHandler(DBProtoAddMail, *this, &MailData::addMail, new db::AddMailReq, new db::AddMailRsp);
	gMsgDispatcher.RegisterHandler(DBProtoUpdateMail, *this, &MailData::updateMail, new db::UpdateMailReq, nullptr);
	gMsgDispatcher.RegisterHandler(DBProtoListMailBox, *this, &MailData::listMail, new cs::CSUint32Req, new db::ListMailBoxRsp);
	gMsgDispatcher.RegisterHandler(DBProtoAddGMMail, *this, &MailData::addGMMail, new db::AddMailGMReq, new db::AddMailGMRsp);
}


ErrCodeType MailData::PackMail(const SSProtoHead& h, uint32_t reg_tm, google::protobuf::Message* out, uint32_t maxId /* = 0 */)
{
	gArena.TrySendWeekendAwardMail(h.TargetID);
	gBehaviourRank.TrySendAwardMail(h.TargetID);

	uint32_t targetID = h.TargetID;
	//已读取的全局邮件最大id
	uint32_t curGMailMaxId = 0;
	//当前全局邮件最大id
	uint32_t gmailMaxId = 0;
	std::string key = kKeyPrefixMailData + to_string(targetID);
	ErrCodeType ret = getPlayerMailGUID(key, curGMailMaxId);
	if (ret != ErrCodeSucc) {
		return ret;
	}

	ret = getGlobalMailGUID(gmailMaxId);
	if (ret != ErrCodeSucc) {
		return ret;
	}
	unordered_map<string, string> fields;
	if (curGMailMaxId) {
		//有新的全局邮件
		if (curGMailMaxId < gmailMaxId) {
			//数量超上限，只取最近几个邮件
			if( gmailMaxId > (kMaxMailLength + curGMailMaxId)) {
				curGMailMaxId = gmailMaxId - kMaxMailLength;
			}
			for (uint32_t i = curGMailMaxId + 1; i <= gmailMaxId; ++i) {
				bool exists = false;
				if (!gRedis->hexists(kKeyGlobalMailData, to_string(i), exists)) {
					WARN_LOG("hget failed: %s! plid=%u", gRedis->last_error_cstr(), targetID);
					continue;
				}
				if (exists) {
					fields[to_string(i)];
				}
			}
			if (!gRedis->hget(kKeyGlobalMailData, fields)) {
				WARN_LOG("hget failed: %s! plid=%u", gRedis->last_error_cstr(), targetID);
				return ErrCodeDB;
			}
		}
	} else if (reg_tm) {
		//第一次获取的话，只拉取注册日期之后的邮件
		if (!gRedis->hgetall(kKeyGlobalMailData, fields)) {
			WARN_LOG("hgetall failed: %s! plid=%u", gRedis->last_error_cstr(), targetID);
			return ErrCodeDB;
		}
		
		auto v = fields.begin();
		while(v != fields.end()) {
			if ( v->first == kFieldGMailGUIDKey ) {
				v = fields.erase(v);
				continue;
			}
			cs::MailBase mail;
			mail.ParseFromString(v->second);
			if (mail.extra_time() < reg_tm) {
				v = fields.erase(v);
			} else {
				v++;
			}
		}
	}

	//如果有全局邮件，就把邮件复制到自己邮箱里
	if (!fields.empty()) {
		db::AddMailReq add;
		db::AddMailRsp addRsp;
		for (const auto& v : fields) {
			add.add_mails()->ParseFromString(v.second);
		}
		addMail(h, &add, &addRsp);
	}

	fields.clear();
	fields[kFieldMailGUIDKey] = to_string(gmailMaxId);
	if (!gRedis->hset(key, fields)) {
		WARN_LOG("hset failed: %s! plid=%u", gRedis->last_error_cstr(), targetID);
		return ErrCodeDB;
	}

	REAL_PROTOBUF_MSG(out, cs::MailBoxData, rsp);
	fields.clear();
	if (!gRedis->hgetall(key, fields)) {
		WARN_LOG("hgetall failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}

	for (const auto& v : fields) {
		if (v.first != kFieldMailUIDKey && v.first != kFieldMailGUIDKey) {
			cs::MailBase mail;
			mail.ParseFromString(v.second);
			//更新邮件的时候，id大于已获取的邮件id最大值判定为新邮件
			if (mail.mail_id() < maxId) {
				continue;
			}
			else {
				rsp.add_mails()->CopyFrom(mail);
			}
		}
	}

	gPlayerData.setPlayerDataTouchInfo(h.TargetID, kKeyPrefixMailData, false);
	return ErrCodeSucc;	
}

ErrCodeType MailData::AddMail(db::AddMailReq& req, db::AddMailRsp* resp /* = nullptr */)
{
	string key;
	string keyFieldUID;

	// 全局邮件
	uint32_t targetID = req.player_id();
	if (req.mail_type()) {
		key = kKeyGlobalMailData;
		keyFieldUID = kFieldGMailGUIDKey;
	}
	else { // 个人邮件
		key = kKeyPrefixMailData + to_string(targetID);
		keyFieldUID = kFieldMailUIDKey;
	}

	unordered_map<string, string> fields;
	for (int i = 0; i < req.mails_size(); ++i) {
		long long uID = 0;
		gRedis->hincrby(key, keyFieldUID, 1, &uID);

		uint32_t realUID = uID;
		if ((uID - realUID) != 0) {
			gRedis->hincrby(key, keyFieldUID, -1);
			WARN_LOG("ERROR EXCEED MAX UINT32_T LENGTH");
			return ErrCodeMail32Limit;
		}

		std::string newMailID(to_string(realUID));
		req.mutable_mails(i)->set_mail_id(realUID);
		if (!(req.mails(i).SerializeToString(&(fields[newMailID])))) {
			DEBUG_LOG("Failed to serialize mail data! plid=%u", targetID);
			return ErrCodeInvalidPacket;
		}
	}

	if (req.mails_size()) {
		if (!gRedis->hset(key, fields)) {
			WARN_LOG("hset failed: %s! plid=%u", gRedis->last_error_cstr(), targetID);
			return ErrCodeDB;
		}
	}
	if (resp) {
		resp->set_res(0);
		for (const auto& v : fields) {
			resp->add_mails()->ParseFromString(v.second);
		}
	}

	gPlayerData.setPlayerDataTouchInfo(targetID, kKeyPrefixMailData, true);
	return ErrCodeType::ErrCodeSucc;
}


ErrCodeType MailData::addMail(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::AddMailReq, req);
	REAL_PROTOBUF_MSG(outMsg, db::AddMailRsp, rsp);

	if (req.player_id() == 0) {
		req.set_player_id(h.TargetID);
	}
	return AddMail(req, &rsp);
}


ErrCodeType MailData::updateMail(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::UpdateMailReq, req);
	string key;
	string keyFieldUID;

	// 全局邮件
	uint32_t targetID = req.player_id();
	if (req.mail_type()) {
		key = kKeyGlobalMailData;
		keyFieldUID = kFieldGMailGUIDKey;
	} else { // 个人邮件
		if (!targetID) {
			targetID = h.TargetID;
		}
		key = kKeyPrefixMailData + to_string(targetID);
		keyFieldUID = kFieldMailUIDKey;
	}

	unordered_map<string, string> fields;
	std::vector<std::string> del_fields(req.del_mails_size());
	if (req.mails_size()) {
		for (int i = 0; i < req.mails_size(); ++i) {
			const cs::MailBase& base = req.mails(i);
			std::string newMailID(to_string(base.mail_id()));
			base.SerializeToString(&(fields[newMailID]));
		}

		if (!gRedis->hset(key, fields)) {
			WARN_LOG("hset failed: %s! plid=%u", gRedis->last_error_cstr(), targetID);
			return ErrCodeDB;
		}
	}

	if (req.del_mails_size()) {
		for (int i =0; i < req.del_mails_size(); ++i) {
			del_fields[i] = to_string(req.del_mails(i));
		}
		DEBUG_LOG("delete Mail %s", req.Utf8DebugString().c_str());

		if (!gRedis->hdel(key, del_fields)) {
			WARN_LOG("hset failed: %s! plid=%u", gRedis->last_error_cstr(), targetID);
			return ErrCodeDB;
		}
	}

#if 0
	REAL_PROTOBUF_MSG(outMsg, db::UpdateMailRsp, rsp);
	rsp.set_res(0);
	for (auto& v : fields) {
		rsp.add_mails()->ParseFromString(v.second);
	}

	for (auto& v : del_fields) {
		rsp.add_del_mails(atoi(v.c_str()));
	}
#endif

	gPlayerData.setPlayerDataTouchInfo(targetID, kKeyPrefixMailData, true);
	return ErrCodeSucc;
}


ErrCodeType MailData::addGMMail(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::AddMailGMReq, req);
	REAL_PROTOBUF_MSG(outMsg, db::AddMailGMRsp, rsp);
	string key;
	string keyFieldUID;

	if (req.mail_type()) {
		key = kKeyGlobalMailData;
		keyFieldUID = kFieldGMailGUIDKey;
		// do add Mail
		db::AddMailRet* ret = rsp.add_rets();
		ret->set_player_id(0);
		ret->set_ret(doAddMail(key, keyFieldUID, 0, &req, ret));
		// 每次添加的时候会触发
		CleanGlobalMail();
	} else { // 个人邮件
		for (int i = 0; i < req.player_id_size(); ++i) {
			key = kKeyPrefixMailData + to_string(req.player_id(i));
			keyFieldUID = kFieldMailUIDKey;
			// do add Mail
			db::AddMailRet* ret = rsp.add_rets();
			ret->set_player_id(req.player_id(i));
			ret->set_ret(doAddMail(key, keyFieldUID, req.player_id(i), &req, ret));
		}
	}

	return ErrCodeSucc;
}

ErrCodeType MailData::listMail(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, cs::CSUint32Req, req);
	REAL_PROTOBUF_MSG(outMsg, db::ListMailBoxRsp, rsp);
	gArena.TrySendWeekendAwardMail(h.TargetID);
	gBehaviourRank.TrySendAwardMail(h.TargetID);
	return PackMail(h, 0, rsp.mutable_box(), req.u32());
}


ErrCodeType MailData::getGlobalMailGUID(uint32_t& gUID)
{
	bool exist = false;
	if (!gRedis->exists(kKeyGlobalMailData, exist)) {
		WARN_LOG("exists failed: %s!", gRedis->last_error_cstr());
		return ErrCodeDB;
	}

	if (exist) {
		unordered_map<string, string> fields;                                                                 
	    auto& gUIDStr = fields[kFieldGMailGUIDKey];
		if (!gRedis->hget(kKeyGlobalMailData, fields)) {
			WARN_LOG("hget failed: %s!", gRedis->last_error_cstr());
			return ErrCodeDB;
		}
		gUID =  atoi(gUIDStr.c_str());
	}
	
	return ErrCodeSucc;
}

ErrCodeType MailData::getPlayerMailUID(std::string& key, uint32_t& uID)
{
	bool exist = false;
	if (!gRedis->exists(key, exist)) {
		WARN_LOG("exists failed: %s!", gRedis->last_error_cstr());
		return ErrCodeDB;
	}

	if (exist) {
		unordered_map<string, string> fields;                                                                 
		auto& uIDStr = fields[kFieldMailUIDKey];	
		if (!gRedis->hget(key, fields)) {
			WARN_LOG("hget failed: %s!", gRedis->last_error_cstr());
			return ErrCodeDB;
		}
		uID =  atoi(uIDStr.c_str());
	}

	return ErrCodeSucc;	
}

ErrCodeType MailData::getPlayerMailGUID(std::string& key, uint32_t& gUID)
{
	bool exist = false;
	if (!gRedis->exists(key, exist)) {
		WARN_LOG("exists failed: %s!", gRedis->last_error_cstr());
		return ErrCodeDB;
	}

	if (exist) {
		unordered_map<string, string> fields;
		auto& gUIDStr = fields[kFieldMailGUIDKey];
		if (!gRedis->hget(key, fields)) {
			WARN_LOG("hget failed: %s!", gRedis->last_error_cstr());
			return ErrCodeDB;
		}
		gUID =  atoi(gUIDStr.c_str());
	}

	return ErrCodeSucc;	
}


ErrCodeType MailData::doAddMail(std::string& key, std::string keyFieldUID, uint32_t targetID, google::protobuf::Message* inMsg, google::protobuf::Message* ret)
{
	REAL_PROTOBUF_MSG(inMsg, db::AddMailGMReq, req);
	REAL_PROTOBUF_MSG(ret, db::AddMailRet, res);
	unordered_map<string, string> fields;
	for (int i = 0; i < req.mails_size(); ++i) {
		long long uID = 0;
		gRedis->hincrby(key, keyFieldUID, 1, &uID);

		uint32_t realUID = uID;
		if ((uID - realUID) != 0) {
			gRedis->hincrby(key, keyFieldUID, -1);
			WARN_LOG("ERROR EXCEED MAX UINT32_T LENGTH");
			res.set_msg("Mail limited");
			return ErrCodeMail32Limit;
		}

		std::string newMailID(to_string(realUID));
		req.mutable_mails(i)->set_mail_id(realUID);
		if (!(req.mails(i).SerializeToString(&(fields[newMailID])))) {
			DEBUG_LOG("Failed to serialize mail data! plid=%u", targetID);
			res.set_msg("Failed to serialize mail data");
			return ErrCodeInvalidPacket;
		}
	}

	if (!gRedis->hset(key, fields)) {
		WARN_LOG("hset failed: %s! plid=%u", gRedis->last_error_cstr(), targetID);
		res.set_msg(gRedis->last_error_cstr());
		return ErrCodeDB;
	}

	return ErrCodeSucc;	
}


void MailData::CleanGlobalMail()
{
	uint32_t guid = 0;
	ErrCodeType type = getGlobalMailGUID(guid);
	if (type == ErrCodeSucc) {
		if (guid > kLimitGMailLen) {
			std::vector<std::string> del_fields(guid - kMaxMailLength - 1);
			if (!gRedis->hdel(kKeyGlobalMailData, del_fields)) {
				WARN_LOG("hset failed: %s! when CleanGlobalMail", gRedis->last_error_cstr());
				return;
			}
		}

	}
}
