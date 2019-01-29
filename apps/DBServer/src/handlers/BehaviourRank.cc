#include "BehaviourRank.h"
#include <sstream>
#include "KeyPrefixDef.h"
#include <libant/utils/StringUtils.h>

#include "MailData.h"
#include "../core/dispatcher.h"
#include "../core/redis_client.h"
#include "../proto/db.pb.h"
#include "../proto/BehaviourRank.pb.h"
#include "../proto/CSCoreMsg.pb.h"
#include "../proto/SvrProtoID.pb.h"
#include "../CSV/CSVRangkingAward.h"
#include "../CSV/CSVRankingInfo.h"
#include "CorpsData.h"
#include "PlayerData.h"
#include "../TimeUtils.h"
#include "../core/ScriptMgr.h"

BehaviourRank gBehaviourRank;

BehaviourRank::BehaviourRank()
{
	gMsgDispatcher.RegisterHandler(DBProtoBRankUpdate, *this, &BehaviourRank::onUpdate, new cs::BehaviourUpdateArr, nullptr);
	gMsgDispatcher.RegisterHandler(DBProtoBRankSendAward, *this, &BehaviourRank::onTrySendAward, new cs::TrySendAward, new cs::SCRepeatedUint32Rsp);
	gMsgDispatcher.RegisterHandler(DBProtoBRankGet, *this, &BehaviourRank::onGetList, new cs::BehaviourRankListReq, new cs::BehaviourRankInfo);
}

void BehaviourRank::Init()
{
	for (auto& it : gCSVRangkingAward.AllItems) {
		auto dict = it.second;
		auto& aInfo = awards_[dict->rankId];
		aInfo.mailId_ = dict->mailId;
		aInfo.award_.push_back(Award());
		auto& award = aInfo.award_.back();
		award.begin_ = dict->begin;
		award.end_ = dict->end;
		if (award.end_ > aInfo.showMax_) {
			aInfo.showMax_ = award.end_;
		}
		for (auto& str : dict->award) {
			auto arr = SplitAndToIntVec(str, '#');
			if (arr.size() < 3) {
				continue;
			}
			award.award_.push_back(arr);
		}
	}
}

ErrCodeType BehaviourRank::onUpdate(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, cs::BehaviourUpdateArr, req);
	for (auto& info : req.info()) {
		// 在gm后台生效的gs角色，在玫瑰榜、助战榜中不计入排名
		if (gPlayerData.IsGsMember(info.tid())) {
			DEBUG_LOG("BehaviourRank::onUpdate IsGsMember. plid=%u", info.tid());
			return ErrCodeType::ErrCodeSucc;
		}

		std::string key = GetRankKey(info.bid(), info.game_region());
		if (info.inc()) {
			const auto& script = ScriptMgr::Instance().GetScriptStr("script/behaviour_rank_inc.lua", false);
			if (script != ScriptMgr::Instance().Empty()) {
				std::vector<std::string> keys;
				std::vector<std::string> args;

				keys.push_back(key);

				args.push_back(std::to_string(info.val()));
				args.push_back(std::to_string(info.tid()));
				args.push_back(std::to_string(time(nullptr)));
				auto reply = gRedis->eval(script, &keys, &args);
				switch (reply->type)
				{
				case REDIS_REPLY_INTEGER:
					break;
				case REDIS_REPLY_ERROR:
					WARN_LOG("Redis error: %s! plid=%u", reply->str, h.PlayerID);
					break;
				default:
					WARN_LOG("Redis error: unexpected reply type %d! plid=%u", reply->type, h.PlayerID);
					break;
				}
			}
		}
		else {
			std::vector<std::string> kv = { { std::to_string((double)info.val() + GetTimeTail()), std::to_string(info.tid()) } };
			gRedis->zadd(key, kv);
		}
	}
	return ErrCodeType::ErrCodeSucc;
}

ErrCodeType BehaviourRank::onTrySendAward(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, cs::TrySendAward, req);
	REAL_PROTOBUF_MSG(outMsg, cs::SCRepeatedUint32Rsp, rsp);
	
	time_t now = time(nullptr);

	DEBUG_LOG("onTrySendAward");
	const std::string& lkey = GetFlagKey(req.game_region());

	for (auto bid : req.bids()) {
		auto dict = gCSVRankingInfo.GetItem(bid);
		if (dict->invalid != 0) {
			continue;
		}
		long long ret;
		gRedis->hincrby(lkey, std::to_string(bid), 1, &ret);
		if (ret != 1) {
			ERROR_LOG("onTrySendAward bid:%u is in Sending", bid);
			continue;
		}
		auto it = awards_.find(bid);
		if (it == awards_.end()) {
			ERROR_LOG("onTrySendAward bid:%u award not define", bid);
			continue;
		}

		DEBUG_LOG("onTrySendAward bid:%u", bid);

		std::stringstream ss;
		auto key = GetRankKey(bid, req.game_region());
		auto& info = it->second;
		for (auto& it : info.award_) {
			std::vector<std::string> fields;
			gRedis->zrevrange(key, it.begin_ - 1, it.end_ - 1, fields);
			int idxBegin = it.begin_;
			// 个人排名
			if (!dict->isGuild) {
				for (auto& id : fields) {
					std::unordered_map<std::string, std::string> info;
					auto& val = info[kKeyBehaviourRankAward];
					ss.str("");
					ss << kKeyPlayerTemp << id;
					auto keyPlayerTmp = ss.str();
					gRedis->hget(keyPlayerTmp, info);
					ss.str("");
					ss << now << "|" << bid << "|" << idxBegin << ",";
					val.append(ss.str());
					gRedis->hset(keyPlayerTmp, info);
					++idxBegin;
					rsp.add_u32(atoi(id.c_str()));
				}
			}
			// 公会排名
			else {
				for (auto& id : fields) {
					std::vector<std::string> players;
					gRedis->hkeys(makeCorpsPlayersKey(id), players);
					// 公会解散
					if (players.size() == 0) {
						continue;
					}
					for (auto& p : players) {
						std::unordered_map<std::string, std::string> info;
						auto& val = info[kKeyBehaviourRankAward];
						ss.str("");
						ss << kKeyPlayerTemp << p;
						auto keyPlayerTmp = ss.str();
						gRedis->hget(keyPlayerTmp, info);
						ss.str("");
						ss << now << "|" << bid << "|" << idxBegin << ",";
						val.append(ss.str());
						gRedis->hset(keyPlayerTmp, info);
						rsp.add_u32(atoi(p.c_str()));
					}
					++idxBegin;
				}
			}
			if (fields.size() < it.end_ - it.begin_) {
				break;
			}
		}
		//发完奖励以后清除排行榜
		//运营要求转存到其他key中
		if(true) {
			uint32_t cursor = 0;
			std::vector<std::string> fields;
			ss.str("");
			ss << key << "_" << GetTodayStr();
			auto historyKey = ss.str();
			while (true) {
				fields.clear();
				gRedis->zscan(key, cursor, fields);
				for (uint32_t i = 0; i < fields.size() / 2; ++i) {
					auto tmp = fields[i * 2];
					fields[i * 2] = fields[i * 2 + 1];
					fields[i * 2 + 1] = tmp;
				}
				gRedis->zadd(historyKey, fields);

				if (cursor == 0) {
					break;
				}
			}
		}
		gRedis->del({ key });

		gRedis->hdel(lkey, { std::to_string(bid) });
	}
	return ErrCodeType::ErrCodeSucc;
}

ErrCodeType BehaviourRank::onGetList(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, cs::BehaviourRankListReq, req);
	REAL_PROTOBUF_MSG(outMsg, cs::BehaviourRankInfo, rsp);
	auto dict = gCSVRankingInfo.GetItem(req.bid());
	if (!dict) {
		return ErrCodeType::ErrCodeGameConfig;
	}
	int dispMax = dict->dispMax;
	std::string key = GetRankKey(req.bid(), req.game_region());
	std::vector<std::string> result;
	gRedis->zrevrange(key, 0, dispMax - 1, result, true);
	for (uint32_t i = 0; i < result.size() / 2; ++i) {
		int tid = atoi(result[i * 2].c_str());
		if (dict->isGuild) {
			tid = gCorpsData.GetCorpsOwnerId(tid);
			if (tid == 0) {
				gRedis->zrem(key, std::to_string(tid));
				continue;
			}
		}
		auto info = rsp.add_infos();
		info->set_id(tid);
		info->set_score(atoi(result[i * 2 + 1].c_str()));
	}
	
	int tid = h.PlayerID;
	int myId = h.PlayerID;
	if (dict->isGuild) {
		bool exist = false;
		auto cKey = makePlayersKey(std::to_string(tid));
		std::string val;
		myId = 0;
		tid = 0;
		if (gRedis->get(cKey, val, &exist)) {
			if (exist) {
				tid = atoi(val.c_str());
				if (tid != 0) {
					myId = gCorpsData.GetCorpsOwnerId(tid);
				}
			}
		}
	}
	double myScore;
	auto myInfo = rsp.mutable_myinfo();
	myInfo->set_id(myId);
	gRedis->zscore(key, std::to_string(tid), myScore);
	myInfo->set_score(myScore);

	return ErrCodeType::ErrCodeSucc;
}

static char gStrBuff[1024];

std::string BehaviourRank::GetRankKey(uint32_t bid, int gameRegion)
{
	sprintf(gStrBuff, "%s_%d_%u", kKeyBehaviourRank.c_str(), gameRegion, bid);
	//sprintf(gStrBuff, "%s_%u", kKeyBehaviourRank.c_str(), bid);
	return gStrBuff;
}


std::string BehaviourRank::GetFlagKey(int gameRegion)
{
	sprintf(gStrBuff, "%s_%d", kKeyBehaviourRankFlag.c_str(), gameRegion);
	return gStrBuff;
	//return kKeyBehaviourRankFlag;
}

void BehaviourRank::TrySendAwardMail(uint32_t uid)
{
	std::stringstream ss;
	ss << kKeyPlayerTemp << uid;
	auto keyPlayerTmp = ss.str();
	std::unordered_map<std::string, std::string> info;
	auto& award = info[kKeyBehaviourRankAward];
	gRedis->hget(keyPlayerTmp, info);
	auto mailInfo = Split(award, ',');
	award.clear();
	gRedis->hset(keyPlayerTmp, info);
	db::AddMailReq req;
	req.set_player_id(uid);
	for (auto& mi : mailInfo) {
		if (mi.empty()) {
			continue;
		}
		auto data = Split(mi, '|');
		if (data.size() != 3) {
			continue;
		}
		auto time = atoi(data[0].c_str());
		auto bid = atoi(data[1].c_str());
		uint32_t rank = atoi(data[2].c_str());
		auto it = awards_.find(bid);
		if (it == awards_.end()) {
			continue;
		}
		auto& ainfo = it->second;
		auto m = req.add_mails();
		m->set_mail_type(ainfo.mailId_);
		m->set_send_time(time);
		m->add_args(std::to_string(rank));
		for (auto& ait : ainfo.award_) {
			if (ait.begin_ <= rank && rank <= ait.end_) {
				for (auto& item : ait.award_) {
					item.AttachToMail(m);
				}
				break;
			}
		}
	}
	gMailData.AddMail(req);
}


void BehaviourRank::DelRankInfo(uint32_t uid, int gameRegion)
{
	for (auto& it : gCSVRankingInfo.AllItems) {
		DEBUG_LOG("DelRankInfo tid:%u, uid:%u", it.first, uid);
		std::string key = GetRankKey(it.first, gameRegion);
		std::string field = { std::to_string(uid) };
		gRedis->zrem(key, field);
	}
}

BehaviourRank::Award::Item::Item(const std::vector<int>& info)
{
	type_ = info[0];
	id_ = info[1];
	cnt_ = info[2];
	exData_.assign(info.begin() + 3, info.end());
}

void BehaviourRank::Award::Item::AttachToMail(cs::MailBase* mail) const
{
	auto attach = mail->add_attach();
	attach->set_type((cs::PlayerResourceType)type_);
	attach->set_id(id_);
	attach->set_cnt(cnt_);
	for (auto& v : exData_) {
		attach->add_extra_datas(v);
	}
}
