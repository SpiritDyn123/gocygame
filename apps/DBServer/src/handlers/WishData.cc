/*
 * WishData.cc
 *
 *  Created on: 2017年4月17日
 *      Author: antigloss
 */

#include <string>
#include "../proto/SvrProtoID.pb.h"
#include "../core/dispatcher.h"
#include "../core/redis_client.h"

#include "KeyPrefixDef.h"
#include "PlayerData.h"
#include "WishData.h"
#include "../TimeUtils.h"

using namespace std;

const string WishData::kFieldWishData("data");
const string WishData::kFieldLeftCnt("cnt");
const string WishData::kFieldTotalGivenCnt("total");
const string WishData::kFieldLastWishTime("last_wish_time");
const string WishData::kFieldWishCnt("wish_cnt");
const string WishData::kFieldPlayedWishGrid("played_grid");
const string WishData::kFieldRecordFlag("rec_flag");

const string WishData::kScriptGiveWishItem =
			"local r = redis.call('HMGET', KEYS[1], '" + kFieldTotalGivenCnt + "', ARGV[1])\n"
			"if r[1] == false then\n"
			"    return 1\n"
			"end\n"
			"local plus = tonumber(ARGV[2])\n"
			"if tonumber(r[1]) + plus > tonumber(ARGV[4]) then\n"
			"    return 1\n"
			"end\n"
			"local cur = 0\n"
			"if r[2] ~= false then\n"
			"    cur = tonumber(r[2])\n"
			"end\n"
			"if cur + plus > tonumber(ARGV[3]) then\n"
			"    return 2\n"
			"end\n"
			"redis.call('HINCRBY', KEYS[1], '" + kFieldTotalGivenCnt + "', ARGV[2])\n"
			"redis.call('HINCRBY', KEYS[1], '" + kFieldLeftCnt + "', ARGV[2])\n"
			"redis.call('HINCRBY', KEYS[1], ARGV[1], ARGV[2])\n"
			"return 0";

WishData wishData;

WishData::WishData()
{
	gMsgDispatcher.RegisterHandler(DBProtoCreateWish, *this, &WishData::createWish,	new db::WishData, nullptr);
	gMsgDispatcher.RegisterHandler(DBProtoGetMyWish, *this, &WishData::getMyWish, nullptr, new db::GetMyWishRsp);
	gMsgDispatcher.RegisterHandler(DBProtoGetWishes, *this, &WishData::getWishes, new db::RepeatedUint32Req, new db::GetWishesRsp);
	gMsgDispatcher.RegisterHandler(DBProtoGiveWishingItem, *this, &WishData::giveWishingItem, new db::GiveWishItemReq, new db::GiveWishItemRsp);
	gMsgDispatcher.RegisterHandler(DBProtoDecLeftWishItemCnt, *this, &WishData::decLeftWishItemCnt,	new db::Uint32Req, nullptr);
	gMsgDispatcher.RegisterHandler(DBProtoDelWish, *this, &WishData::delWish, nullptr, nullptr);

	gMsgDispatcher.RegisterHandler(DBProtoGetWishInfo_V1, *this, &WishData::getWish_V1, new db::BoolReq, new cs::WishDataWithRank);
	gMsgDispatcher.RegisterHandler(DBProtoSetWish_V1, *this, &WishData::setWish_V1, new cs::WishData_V1, nullptr);
	gMsgDispatcher.RegisterHandler(DBProtoDelWish_V1, *this, &WishData::delWish_V1, new db::RepeatedUint32Req, nullptr);
	gMsgDispatcher.RegisterHandler(DBProtoAddWishRecord, *this, &WishData::addWishRecord, new cs::WishRecord, nullptr);
	gMsgDispatcher.RegisterHandler(DBProtoGetWishRecord, *this, &WishData::getWishRecord, nullptr, new cs::WishRecordData);
	gMsgDispatcher.RegisterHandler(DBProtoAddWishPopularRank, *this, &WishData::addWishPopular, new db::WishPopularReq, new db::WishPopularRsp);
}

void WishData::GetWishData(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	db::BoolReq dbReq;
	dbReq.set_flag(false);
	getWish_V1(h, &dbReq, outMsg);
}

//----------------------------------------------------------------------
// Private Methods
//----------------------------------------------------------------------
ErrCodeType WishData::getWish_V1(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	//return ErrCodeDB;
	REAL_PROTOBUF_MSG(inMsg, db::BoolReq, req);
	string strPlayerID = to_string(h.TargetID);
	string key(kKeyPrefixWishData_V1 + strPlayerID);
	unordered_map<string, string> fields;
	if (!gRedis->hgetall(key, fields)) {
		WARN_LOG("hgetall failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}
	REAL_PROTOBUF_MSG(outMsg, cs::WishDataWithRank, rsp);

	for (auto it : fields) {
		if (it.first == kFieldWishCnt) {
			rsp.mutable_wish_data()->set_wish_cnt(atoi(it.second.c_str()));
		} else if (it.first == kFieldPlayedWishGrid) {
			if (req.flag()) {
				rsp.mutable_wish_data()->set_played_grid(atoi(it.second.c_str()));
			}
		} else if (it.first == kFieldLastWishTime) {
			if (req.flag()) {
				rsp.mutable_wish_data()->set_last_wish_time(atoi(it.second.c_str()));
			}
		} else if (it.first == kFieldRecordFlag) {
			if(req.flag()) {
				rsp.mutable_wish_data()->set_record_flag(atoi(it.second.c_str()));
			}
		} else {
			auto wish = rsp.mutable_wish_data()->add_wishes();
			wish->ParseFromString(it.second);
		}
	}
	// 是否是家园来拉信息
	if (req.flag()) {
		// 取玩家人气、爱心值 TODO
		/*long long totalPopular, totalLove, weekPopular, weekLove;
		ErrCodeType errCodePopLove = PlayerData::GetPopAndLove(strPlayerID, &totalPopular, &totalLove, &weekPopular, &weekLove);
		if (errCodePopLove) {
			return errCodePopLove;
		}*/
		db::SimpleSocialInfo ssInfo;
		PlayerData::GetSocialSimpleInfo(strPlayerID, ssInfo);
		rsp.mutable_wish_data()->set_popular(ssInfo.total_popular());

		// 拉取心愿榜前3
		string rankKey = GetWishRankKey(kKeyPrefixWishData, h.TargetID);
		std::vector<std::string> result;
		if (!gRedis->zrevrange(rankKey, 0, 2, result, true)) {
			WARN_LOG("zrevrank failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
			return ErrCodeDB;
		}

		auto iter = result.begin();
		for(; iter != result.end(); ++iter) {
			auto rankData = rsp.add_rank_data();
			rankData->set_plid(atoi(iter->c_str()));
			iter++;
			rankData->set_score(atoi(iter->c_str()));
		}
	}
	return ErrCodeSucc;
}

ErrCodeType WishData::setWish_V1(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, cs::WishData_V1, req);
	string key = kKeyPrefixWishData_V1 + to_string(h.TargetID);
	unordered_map<string, string> fields;

	if (req.has_wish_cnt()) {
		fields[kFieldWishCnt] = to_string(req.wish_cnt());
	}
	if (req.has_played_grid()) {
		fields[kFieldPlayedWishGrid] = to_string(req.played_grid());
	}
	if (req.has_last_wish_time()) {
		fields[kFieldLastWishTime] = to_string(req.last_wish_time());
	}
	if (req.has_record_flag()) {
		fields[kFieldRecordFlag] = to_string(req.record_flag());
	}

	for (auto wish : req.wishes()) {
		wish.SerializeToString(&fields[to_string(wish.grid())]);
	}

	if (!gRedis->hset(key, fields)) {
		WARN_LOG("hset failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}

	DEBUG_LOG("Wish created. plid=%u wish_cnt=%d", h.TargetID, req.wishes_size());
	return ErrCodeSucc;
}

ErrCodeType WishData::delWish_V1(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::RepeatedUint32Req, req);
	string key = kKeyPrefixWishData_V1 + to_string(h.TargetID);
	vector<string> fields;
	for (auto u32 : req.u32()) {
		fields.emplace_back(to_string(u32));
	}

	if (!gRedis->hdel(key, fields)) {
		WARN_LOG("hdel failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}

	for (auto u32 : req.u32()) {
		DEBUG_LOG("Wish deleted. plid=%u wish_grid=%u", h.TargetID, u32);
	}
	return ErrCodeSucc;
}

ErrCodeType WishData::addWishRecord(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, cs::WishRecord, req);
	string key = kKeyPrefixWishRecord + to_string(h.TargetID);
	string serializedStr;
	req.SerializeToString(&serializedStr);
	vector<string> vals = { serializedStr };

	if (!gRedis->lpush(key, vals)) {
		WARN_LOG("lpush failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}

	return ErrCodeSucc;
}

const string WishData::kScriptTrimWishRecord =
			"if redis.call('LLEN', KEYS[1]) > 300 then\n"
				"redis.call('LTRIM', KEYS[1], 0, 49)\n"
				"return 1\n"
			"end\n"
			"return 0";

ErrCodeType WishData::getWishRecord(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(outMsg, cs::WishRecordData, rsp);
	string key = kKeyPrefixWishRecord + to_string(h.TargetID);
	vector<string> vals;

	vector<string> scriptKeys = { key };
	ScopedReplyPointer replySet = gRedis->eval(kScriptTrimWishRecord, &scriptKeys);
	CHECK_REPLY_EC(replySet, REDIS_REPLY_INTEGER, h.TargetID);

	if (!gRedis->lrange(key, 0, 49, vals)) {
		WARN_LOG("lrange failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}
	for (auto data : vals) {
		rsp.add_records()->ParseFromString(data);
	}

	return ErrCodeSucc;
}

ErrCodeType WishData::addWishPopular(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::WishPopularReq, req);
	string rankKey = GetWishRankKey(kKeyPrefixWishData, h.TargetID);

	if (!gRedis->zincrby(rankKey, to_string(req.player_id()), req.popular())) {
		WARN_LOG("zincrby failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}
	if (!gRedis->expire_at(rankKey, NextMonday())) {
		WARN_LOG("expire_at failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}

	REAL_PROTOBUF_MSG(outMsg, db::WishPopularRsp, rsp);

	// 返回排名前3的
	std::vector<std::string> result;
	if (!gRedis->zrevrange(rankKey, 0, 2, result, true)) {
		WARN_LOG("zrevrank failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}

	auto iter = result.begin();
	for(; iter != result.end(); ++iter) {
		auto rankData = rsp.mutable_rank_list()->add_datas();
		rankData->set_plid(atoi(iter->c_str()));
		iter++;
		rankData->set_score(atoi(iter->c_str()));
	}
	rsp.mutable_wish_give()->Swap(req.mutable_wish_give());

	return ErrCodeSucc;
}
//---------------------------------------------------------------------------
ErrCodeType WishData::createWish(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	string key = kKeyPrefixWishData + to_string(h.TargetID);
	unordered_map<string, string> fields;
	auto& dataStr = fields[kFieldWishData];
	auto& leftCntStr = fields[kFieldLeftCnt];
	if (!gRedis->hget(key, fields)) {
		WARN_LOG("hget failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}

	while (dataStr.size()) {
		if (atoi(leftCntStr.c_str()) > 0) {
			DEBUG_LOG("Not yet get all wishing items! plid=%u left=%s", h.TargetID, leftCntStr.c_str());
			return ErrCodeHaventGotAllWishItem;
		}

		db::WishData wishData;
		if (wishData.ParseFromString(dataStr)) {
			time_t tNow = time(0);
			int today = localtime(&tNow)->tm_yday;

			time_t createdTime = wishData.created_time();
			int createdDay = localtime(&createdTime)->tm_yday;

			if (today == createdDay) {
				DEBUG_LOG("One wish per day! plid=%u", h.TargetID);
				return ErrCodeWishAlreadyCreated;
			}
		}

		if (!gRedis->del({key})) {
			WARN_LOG("del failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
			return ErrCodeDB;
		}

		break;
	}

	REAL_PROTOBUF_MSG(inMsg, db::WishData, req);
	req.set_created_time(time(0));

	fields.erase(kFieldLeftCnt);
	if (!req.SerializeToString(&dataStr)) {
		DEBUG_LOG("Failed to serialize wish data! plid=%u", h.TargetID);
		return ErrCodeInvalidPacket;
	}
	fields[kFieldTotalGivenCnt] = "0";

	if (!gRedis->hset(key, fields)) {
		WARN_LOG("hset failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}

	DEBUG_LOG("Wish created. plid=%u type=%d itemID=%d", h.TargetID, req.item_type(), req.item_id());
	return ErrCodeSucc;
}

ErrCodeType WishData::getMyWish(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	string key = kKeyPrefixWishData + to_string(h.TargetID);
	unordered_map<string, string> fields;
	if (!gRedis->hgetall(key, fields)) {
		WARN_LOG("hgetall failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}
	if (fields.empty()) {
		DEBUG_LOG("Haven't make a wish. plid=%u", h.TargetID);
		return ErrCodeHaventMadeWish;
	}

	REAL_PROTOBUF_MSG(outMsg, db::GetMyWishRsp, rsp);
	for (const auto& v : fields) {
		if (v.first == kFieldWishData) {
			rsp.mutable_my_wish()->ParseFromString(v.second);
		} else if (v.first == kFieldLeftCnt) {
			rsp.set_item_cnt(atoi(v.second.c_str()));
		} else if (v.first == kFieldTotalGivenCnt) {
			rsp.set_given_cnt(atoi(v.second.c_str()));
		} else {
			auto giver = rsp.add_history();
			giver->set_player_id(atoi(v.first.c_str()));
			giver->set_cnt(atoi(v.second.c_str()));
		}
	}

	return ErrCodeSucc;
}

ErrCodeType WishData::getWishes(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::RepeatedUint32Req, req);
	REAL_PROTOBUF_MSG(outMsg, db::GetWishesRsp, rsp);

	db::WishData wishData;
	unordered_map<string, string> fields;
	for (int i = 0; i < req.u32_size(); ++i) {
		uint32_t plid = req.u32(i);
		string wishKey = kKeyPrefixWishData + to_string(plid);

		fields.clear();
		if (!gRedis->hgetall(wishKey, fields)) {
			WARN_LOG("hgetall failed: %s! plid=%u follow=%u", gRedis->last_error_cstr(), h.TargetID, plid);
			continue;
		}
		if (fields.empty()) {
			continue;
		}

		const auto& wishDataStr = fields[kFieldWishData];
		if (wishDataStr.empty()) {
			continue;
		}

		wishData.ParseFromString(wishDataStr);
		if (time(0) - wishData.created_time() > 86200) { // 留一些冗余时间
			continue;
		}

		auto wishInfo = rsp.add_wishes();
		wishInfo->set_player_id(plid);
		wishInfo->set_item_type(wishData.item_type());
		wishInfo->set_item_id(wishData.item_id());
		wishInfo->set_create_time(wishData.created_time());
		wishInfo->set_max_given_cnt(wishData.max_given_cnt());

		setGivenCntAndHistory(fields, wishInfo);
	}

	return ErrCodeSucc;
}

ErrCodeType WishData::giveWishingItem(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::GiveWishItemReq, req);

	DEBUG_LOG("Give wishing item. plid=%u target=%u type=%d itemID=%u giveCnt=%d max=%d",
			  h.PlayerID, h.TargetID, req.item_type(), req.item_id(), req.item_cnt(), req.max_given_cnt());

	string strPlayerID = to_string(h.PlayerID);
	string key = kKeyPrefixWishData + to_string(h.TargetID);
	unordered_map<string, string> fields;
	auto& dataStr = fields[kFieldWishData];
	if (!gRedis->hget(key, fields)) {
		WARN_LOG("hget failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}

	ErrCodeType errCode = ErrCodeSucc;
	db::WishData wishData;
	wishData.ParseFromString(dataStr);
	if (wishData.created_time() == req.create_time()) {
		errCode = gPlayerData.CheckIfInBlacklist(h.TargetID, h.PlayerID);
		if (errCode != ErrCodeSucc) {
			return errCode;
		}

		vector<string> keys = { key };
		vector<string> args = { strPlayerID, to_string(req.item_cnt()),
								to_string(req.max_give_cnt()), to_string(req.max_given_cnt()) };
		ScopedReplyPointer reply = gRedis->eval(kScriptGiveWishItem, &keys, &args);
		CHECK_REPLY_EC(reply, REDIS_REPLY_INTEGER, h.PlayerID);
		if (reply->integer == 1) {
			DEBUG_LOG("Wish item given limit! plid=%u target=%u", h.PlayerID, h.TargetID);
			errCode = ErrCodeWishItemGivenLimit;
		} else if (reply->integer == 2) {
			DEBUG_LOG("Wish item giving limit! plid=%u target=%u", h.PlayerID, h.TargetID);
			errCode = ErrCodeWishItemGivingLimit;
		}
	} else {
		DEBUG_LOG("Wish already changed! plid=%u target=%u", h.PlayerID, h.TargetID);
		errCode = ErrCodeWishItemGivenLimit;
	}

	REAL_PROTOBUF_MSG(outMsg, db::GiveWishItemRsp, rsp);
	if (errCode) {
		rsp.set_errcode(errCode);
	} else {
		rsp.set_item_type(req.item_type());
		rsp.set_item_id(req.item_id());
		rsp.set_item_cnt(req.item_cnt());
		rsp.set_friendship(req.friendship());
	}
	auto wishInfo = rsp.mutable_wish();
	wishInfo->set_player_id(h.TargetID);
	wishInfo->set_item_type(wishData.item_type());
	wishInfo->set_item_id(wishData.item_id());
	wishInfo->set_create_time(wishData.created_time());
	wishInfo->set_max_given_cnt(wishData.max_given_cnt());

	fields.clear();
	gRedis->hgetall(key, fields);
	setGivenCntAndHistory(fields, wishInfo);

	return ErrCodeSucc;
}

ErrCodeType WishData::decLeftWishItemCnt(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::Uint32Req, req);
	DEBUG_LOG("Dec wish item cnt. plid=%u", h.TargetID);

	string key = kKeyPrefixWishData + to_string(h.TargetID);
	bool exist;
	if (!gRedis->exists(key, exist)) {
		WARN_LOG("exists failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}

	if (exist) {
		long long inc = req.u32();
		gRedis->hincrby(key, kFieldLeftCnt, -inc);
	}

	return ErrCodeSucc;
}

ErrCodeType WishData::delWish(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	DEBUG_LOG("Del wish item cnt. plid=%u", h.TargetID);
	gRedis->del({kKeyPrefixWishData + to_string(h.TargetID)});
	return ErrCodeSucc;
}

//----------------------------------------------------
// Static helpers
//----------------------------------------------------
void WishData::setGivenCntAndHistory(const std::unordered_map<std::string, std::string>& fields, cs::WishInfo* wishInfo)
{
	for (const auto& v : fields) {
		if (v.first == kFieldWishData) {
			// Nothing to do
		} else if (v.first == kFieldLeftCnt) {
			// Nothing to do
		} else if (v.first == kFieldTotalGivenCnt) {
			wishInfo->set_given_cnt(atoi(v.second.c_str()));
		} else {
			auto giver = wishInfo->add_history();
			giver->set_player_id(atoi(v.first.c_str()));
			giver->set_cnt(atoi(v.second.c_str()));
		}
	}
}

string WishData::GetWishRankKey(const std::string& rankName, uint32_t playerID)
{
	db::RankKey rankObj;

	rankObj.set_rank_type(RankData::kWeek);
	rankObj.set_rank_index(playerID);
	rankObj.set_rank_name(rankName);
	rankObj.set_rank_deadtm(NextMonday());
	return RankData::MakeRankKey(rankObj);
}
