#include "../core/dispatcher.h"
#include "../core/redis_client.h"
#include "../proto/db.pb.h"
#include "../proto/SvrProtoID.pb.h"
#include "../CSV/CSVRankRewardSocial.h"
#include "../CSV/CSVMailStyle.h"
#include "../TimeUtils.h"
#include "../log.h"
#include <libant/utils/StringUtils.h>
#include <libant/time/time_utils.h>
#include "MailData.h"
#include "RankData.h"
#include "RankingAward.h"
#include "KeyPrefixDef.h"

RankingAward::RankingAward()
{
	gMsgDispatcher.RegisterHandler(DBProtoRankSendAward, *this, &RankingAward::TrySendAward, new db::RankingAwardReq, new db::RankingAwardResp);
}

void RankingAward::Init()
{

}

ErrCodeType RankingAward::TrySendAward(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::RankingAwardReq, req);
	REAL_PROTOBUF_MSG(outMsg, db::RankingAwardResp, rsp);

	time_t now = time(NULL);

	//先判断是否发过奖励
	bool exists = false;
	std::string strLastWeekend;
	tm tmNow;
	localtime_r(&now, &tmNow);
	int dd = (tmNow.tm_wday - 1) % 7;
	if (dd < 0) {
		dd += 7;
	}
	time_t curWeekend = ant::today_begin_time(now) - dd * 24 * 3600;

	gRedis->get(kRankingAwardLastTime, strLastWeekend, &exists);
	if (exists) {
		time_t lastTime = atoi(strLastWeekend.c_str());
		if (DayDiff(curWeekend, lastTime) < 7) {
			if (!req.gm()) {
				DEBUG_LOG("Award was send at:%ld cur:%ld now:%ld", lastTime, curWeekend, now);
				rsp.set_ret(db::RAR_DONE);
				return ErrCodeType::ErrCodeSucc;
			}
		}
	}

	if (req.op_code() == db::RAOC_INIT) {
		long long cnt;
		gRedis->incrby(kRankingAwardWorking, 1, &cnt);
		if (cnt != 1) {
			uint32_t svrId = 0;
			std::string strSvrId;
			gRedis->get(kRankingAwardSvrId, strSvrId);
			if (!strSvrId.empty()) {
				svrId = atoi(strSvrId.c_str());
			}
			if (svrId != req.svr_id()) {
				//如果有别的GameSvr触发了发奖流程,则不再重复触发
				rsp.set_ret(db::RAR_IN_WORKING);
				return ErrCodeType::ErrCodeSucc;
			}
			else {
				//没发完全部奖励前GS挂了
			}
		}
		else {
			//清空上次发奖记录
			gRedis->del(std::vector<std::string>{kRankingAwardDone});
			gRedis->set(kRankingAwardSvrId, std::to_string(req.svr_id()));
			DEBUG_LOG("Begin Send Weakend Ranking Award");
		}
	}
	std::vector<std::string> result;
	db::RankKey rk;
	rk.set_rank_type(RankData::kWeek);
	rk.set_rank_deadtm(LastMonday());
	std::stringstream ss;
	switch (req.type())
	{
	case 1:
		ss << kSocialFieldWeekPopular;
		break;
	case 2:
		ss << kSocialFieldWeekLove;
		break;
	default:
		return ErrCodeType::ErrCodeSucc;
	}
	auto kGeoSplit = "_";

	uint32_t key = req.ranking_key();
	if (req.region() > 0) {
		ss << kGeoSplit << (key / 10000 * 10000);
	}
	if (req.region() > 1) {
		ss << kGeoSplit << (key / 100 * 100);
	}
	if (req.region() > 2) {
		ss << kGeoSplit << key;
	}

	rk.set_rank_name(ss.str());
	const auto& keyRanking = RankData::MakeRankKey(rk);

	//发过了直接返回
	bool isMember;
	gRedis->sismember(kRankingAwardDone, keyRanking, isMember);
	if (isMember) {
		return ErrCodeType::ErrCodeSucc;
	}

	uint32_t regionId = 1;
	uint32_t mailType = 903;
	auto mailDict = gCSVMailStyle.GetItem(mailType);
	if (!mailDict) {
		ERROR_LOG("Mail Not Define Id:%d", mailType);
		return ErrCodeType::ErrCodeSucc;
	}

	DEBUG_LOG("Ranking Award : %s", keyRanking.c_str());

	while (true) {
		uint32_t dictId = (req.type() * 100 + req.region()) * 100 + regionId;
		auto dict = gCSVRankRewardSocial.GetItem(dictId);
		if (!dict) {
			break;
		}
		cs::RewardItems rewards;
		for (auto& it : dict->Reward) {
			//类型#ID#数量#宠物进化树
			auto arr = SplitAndToIntVec(it, '#');
			if (arr.size() < 3) {
				continue;
			}
			MailAddAttach(rewards.mutable_items(), arr);
		}
		int begin = dict->RankBegin;
		int end = dict->RankEnd;
		int rBegin = begin;
		int rEnd = end;
		const int onceLimit = 100;
		if (rEnd - begin >= onceLimit) {
			rEnd = begin + onceLimit - 1;
		}
		while (true) {
			gRedis->zrevrange(keyRanking, rBegin, rEnd, result);
			int rId = 0;
			for (auto& strPid : result) {
				int rank = rBegin + rId++;
				db::AddMailReq mailReq;
				uint32_t pid = atoi(strPid.c_str());
				mailReq.set_player_id(pid);
				auto mail = mailReq.add_mails();
				mail->set_mail_type(mailType);
				mail->mutable_attach()->CopyFrom(rewards.items());
				mail->set_send_time(now);
				mail->set_life_time(mailDict->DeadTime);
				mail->add_args(std::to_string(req.type()));
				mail->add_args(std::to_string(req.region()));
				mail->add_args(std::to_string(rank));
				gMailData.AddMail(mailReq);
				rsp.add_affect_pids(pid);
			}
			if (rEnd == end) {
				break;
			}
			rBegin = rEnd + 1;
			rEnd += onceLimit;
			if (rEnd >= end) {
				rEnd = end;
			}
		}
		++regionId;
	}
	if (rsp.affect_pids_size() > 0) {
		DEBUG_LOG("valid Ranking Award : %s", keyRanking.c_str());
	}
	gRedis->sadd(kRankingAwardDone, std::vector<std::string>{req.ranking_key()});
	if (req.op_code() == db::RAOC_END) {
		//奖励发完了记录发奖时间，并清空正在发奖标记
		gRedis->del(std::vector<std::string>{kRankingAwardWorking, kRankingAwardSvrId});
		gRedis->set(kRankingAwardLastTime, std::to_string(curWeekend));
	}
	return ErrCodeType::ErrCodeSucc;
}
