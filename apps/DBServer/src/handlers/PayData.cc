#include <vector>
#include <sstream>
#include <libant/utils/StringUtils.h>

#include "../core/dispatcher.h"
#include "../core/redis_client.h"
#include "../proto/SvrProtoID.pb.h"
#include "../proto/GmMsg.pb.h"

#include "KeyPrefixDef.h"
#include "PayData.h"
#include "../TimeUtils.h"
#include "../tables/tbl_order.h"
#include "../global.h"
#include "../core/MysqlProxy.h"
#include "../CSV/CSVRMBShop.h"

using namespace std;

PayData gPayData;

PayData::PayData()
{
	// 支付订单
	gMsgDispatcher.RegisterHandler(DBProtoGetPayOrder, *this, &PayData::GetPayOrder, nullptr, new db::PayOrder);
	gMsgDispatcher.RegisterHandler(DBProtoFinishPayOrder, *this, &PayData::finishPayOrder, new db::RepeatedStrReq, new db::PayOrder);
	gMsgDispatcher.RegisterHandler(DBProtoGenPayOrder, *this, &PayData::genPayOrder, new db::GenOrderReq, new db::StrRsp);
	gMsgDispatcher.RegisterHandler(DBProtoGenTestPayOrder, *this, &PayData::genPayOrder, new db::GenOrderReq, new db::StrRsp);
	gMsgDispatcher.RegisterHandler(DBProtoSetPaidOrder, *this, &PayData::setPaidOrder, new db::OrderInfo, new db::Uint32Rsp);
	// 拉取累计充值彩钻
	gMsgDispatcher.RegisterHandler(DBProtoGetRebateInfo, *this, &PayData::getRebateGemsInfo, nullptr, new db::AccountBoughtGems);
	gMsgDispatcher.RegisterHandler(DBProtoGetRebateReward, *this, &PayData::getRebateGemsReward, nullptr, new db::AccountBoughtGems);
	gMsgDispatcher.RegisterHandler(DBProtoGetAllRebateGems, *this, &PayData::allRebateGemsInfo, nullptr, new db::AllAccountBoughtGems);
	//gm模拟充值，玩家不在线时
	gMsgDispatcher.RegisterHandler(DBProtoGmPay, *this, &PayData::onGmPay, new GmMsg::GmPayReq, new GmMsg::GmPayReq);
}

//===============================================================================
// Public methods
//===============================================================================
ErrCodeType PayData::GetPayOrder(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(outMsg, db::PayOrder, rsp);
	// 订单信息
	unordered_map<string, string> pendingOrders;
	if (!gRedis->hgetall(makePendingOrderKey(h.TargetID), pendingOrders)) {
		WARN_LOG("hgetall failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}
	unordered_map<string, string> paidOrders;
	if (!gRedis->hgetall(makePaidOrderKey(h.TargetID), paidOrders)) {
		WARN_LOG("hgetall failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}
	unordered_map<string, string> finishedOrders;
	if (!gRedis->hgetall(makeFinishedOrderKey(h.TargetID), finishedOrders)) {
		WARN_LOG("hgetall failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}
	toOrdersData(pendingOrders, rsp.mutable_pending_orders());
	toOrdersData(paidOrders, rsp.mutable_paid_orders());
	toOrdersData(finishedOrders, rsp.mutable_finished_orders());

	return ErrCodeSucc;
}


void PayData::Init()
{
	dbPrefix_ = MysqlProxy::Instance().GetKv("globalDB");
	tblPrefix_ = "order_info";
}

//===============================================================================
// Private methods
//===============================================================================
ErrCodeType PayData::genPayOrder(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::GenOrderReq, req);
	REAL_PROTOBUF_MSG(outMsg, db::StrRsp, dbRsp);

	// 检查充值渠道是否被ban
	unordered_map<string, string> m;
	auto& strChannelBanTime = m[req.channel()];
	if (!gRedis->hget(kMiscFieldBanChannelPay, m)) {
		WARN_LOG("hget baned pay channel failed! channel=%s", req.channel().c_str());
		return ErrCodeDB;
	}
	time_t channelBanTime = atoi(strChannelBanTime.c_str());
	time_t tNow = time(0);
	if (tNow < channelBanTime) {
		return ErrCodeSucc;
	}
	tm* tmNow = localtime(&tNow);
	tmNow->tm_hour = 0;
	tmNow->tm_min = 0;
	tmNow->tm_sec = 0;
	time_t today = mktime(tmNow);
	long long orderNo;
	if (!gRedis->incrby(makeDayPayNoKey(today), 1, &orderNo)) {
		WARN_LOG("incrby failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}
	string strOrderNo = to_string(orderNo);
	if (strOrderNo.size() < 8) {
		strOrderNo.insert(0, 8 - strOrderNo.size(), '0');
	}
	// 订单格式：playerID_productID_price_gems_nowstamp_todayOrderNo
	stringstream ss;
	ss << h.PlayerID << "_" << req.product_id() << "_" << req.price() << "_" << req.gems() << "_" << tNow << "_" << strOrderNo<<"_"<<req.zone();
	string orderID = ss.str();
	bool success;
	db::OrderInfo info;
	info.set_cp_trade_no(orderID);
	info.set_player_id(h.PlayerID);
	info.set_create_time(to_string(tNow));
	string serializedOrderInfo;
	info.SerializeToString(&serializedOrderInfo);
	if (!gRedis->hsetnx(makePendingOrderKey(h.PlayerID), orderID, serializedOrderInfo, success)) {
		WARN_LOG("hsetnx failed: %s! plid=%u", gRedis->last_error_cstr(), h.PlayerID);
		return ErrCodeDB;
	}
	DEBUG_LOG("plid=%u orderID=%s success=%u", h.PlayerID, orderID.c_str(), success);
	if (success) {
		dbRsp.set_str(orderID);
	}

	return ErrCodeSucc;
}

ErrCodeType PayData::finishPayOrder(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::RepeatedStrReq, req);

	db::OrderInfo tmpOrder;
	unordered_map<string, string> orders;
	vector<string> paidOrdersToDel;

	bool exits = false;
    for (int i = 0; i < req.strs_size(); i++) {
        if (!gRedis->hexists(makePaidOrderKey(h.TargetID), req.strs(i), exits)) {
            WARN_LOG("hget failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
            return ErrCodeDB;
        }
        if (exits){
            orders[req.strs(i)];
        } else {
            WARN_LOG("Pay Err plid=%u not exits Paid Order %s", h.TargetID, req.strs(i).c_str());
        }
    }

    if (orders.size()) {
        if (!gRedis->hget(makePaidOrderKey(h.TargetID), orders)) {
            WARN_LOG("hget failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
            return ErrCodeDB;
        }

        for (auto& v : orders) {
            tmpOrder.ParseFromString(v.second);
            tmpOrder.set_create_time(to_string(time(NULL)));
            tmpOrder.SerializeToString(&(v.second));
            paidOrdersToDel.emplace_back(v.first);
            TblOrder tblOrder(h.TargetID, dbPrefix_.c_str(), tblPrefix_.c_str());
            tblOrder.Add(tmpOrder.cp_trade_no(), tmpOrder.total_fee(),
                    tmpOrder.currency_type(), tmpOrder.product_id(),
                    tmpOrder.create_time(), tmpOrder.trade_time(),
                    tmpOrder.is_test_order(), tmpOrder.channel_trade_no(),
                    tmpOrder.channel(), tmpOrder.gems());

        }

        if (!gRedis->hset(makeFinishedOrderKey(h.TargetID), orders)) {
            WARN_LOG("hset failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
            return ErrCodeDB;
        }

        if (!gRedis->hdel(makePaidOrderKey(h.TargetID), paidOrdersToDel)) {
            WARN_LOG("hdel failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
            return ErrCodeDB;
        }
        if (!gRedis->hdel(makePendingOrderKey(h.TargetID), paidOrdersToDel)) {
            WARN_LOG("hdel failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
            return ErrCodeDB;
        }
    }

#if 0
	for (int i = 0; i < req.strs_size(); i++) {
		string& info = orders[req.strs(i)];
		if (!gRedis->hget(makePaidOrderKey(h.TargetID), orders)) {
			WARN_LOG("hget failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
			return ErrCodeDB;
		}
		tmpOrder.ParseFromString(info);
		tmpOrder.set_create_time(to_string(time(NULL)));
		tmpOrder.SerializeToString(&orders[req.strs(i)]);
		if (!gRedis->hset(makeFinishedOrderKey(h.TargetID), orders)) {
			WARN_LOG("hset failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
			return ErrCodeDB;
		}
		paidOrdersToDel.emplace_back(req.strs(i));
		TblOrder tblOrder(h.TargetID, dbPrefix_.c_str(), tblPrefix_.c_str());
		tblOrder.Add(tmpOrder.cp_trade_no(), tmpOrder.total_fee(),
			tmpOrder.currency_type(), tmpOrder.product_id(),
			tmpOrder.create_time(), tmpOrder.trade_time(),
			tmpOrder.is_test_order(), tmpOrder.channel_trade_no(),
			tmpOrder.channel(), tmpOrder.gems());
	}
	if (!gRedis->hdel(makePaidOrderKey(h.TargetID), paidOrdersToDel)) {
		WARN_LOG("hdel failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}
	if (!gRedis->hdel(makePendingOrderKey(h.TargetID), paidOrdersToDel)) {
		WARN_LOG("hdel failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}
#endif 
	GetPayOrder(h, nullptr, outMsg);
	return ErrCodeSucc;
}

static const string kScriptSetPaidOrder = 
	"if redis.call('HEXISTS', KEYS[2], ARGV[1]) == 1 then\n" // 检查FinishedOrder
    "  return 2\n"
    "end\n"
    "if redis.call('HEXISTS', KEYS[1], ARGV[1]) == 0 then\n" // 检查PendingOrder
    "  return 1\n"
    "end\n"
    "if redis.call('HSETNX', KEYS[3], ARGV[1], ARGV[2]) == 0 then\n" // 设置PaidOrder
    "  return 3\n"
    "end\n"
    "redis.call('HINCRBY', KEYS[4], ARGV[3], tonumber(ARGV[4]))\n" // 增加玩家购买的宝石数
    "redis.call('RPUSH', KEYS[5], ARGV[1])\n" // 加入今日已支付订单队列
    "return 0";

ErrCodeType PayData::setPaidOrder(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::OrderInfo, req);
	REAL_PROTOBUF_MSG(outMsg, db::Uint32Rsp, rsp);

	string serializedData;
	req.SerializeToString(&serializedData);
	unordered_map<string, string> m;
	auto& regInfoStr = m[kRegInfo];
	if (!gRedis->hget(kKeyPrefixPlayerData + to_string(h.TargetID), m)) {
		WARN_LOG("hget failed: %s! plid=%u", gRedis->last_error_message().c_str(), h.TargetID);
		return ErrCodeDB;
	}
	db::RegInfo regInfo;
	regInfo.ParseFromString(regInfoStr);
	string account = makeLoginKey(regInfo.account().account_type(), regInfo.account().account_id());
	vector<string> payKeys = { makePendingOrderKey(h.TargetID), makeFinishedOrderKey(h.TargetID), makePaidOrderKey(h.TargetID), 
		kMiscFieldBoughtGems, makeDayPaidOrderQueueKey(GetTodayStr()) };
	vector<string> payArgs = { req.cp_trade_no(), serializedData, account, req.gems() };

	ScopedReplyPointer reply = gRedis->eval(kScriptSetPaidOrder, &payKeys, &payArgs);
	CHECK_REPLY_EC(reply, REDIS_REPLY_INTEGER, h.TargetID);
	if (reply->integer != 0) {
		DEBUG_LOG("Paid order set failed! plid=%u, oid=%s", h.TargetID, req.cp_trade_no().c_str());
	}
	rsp.set_u32(reply->integer);
	return ErrCodeSucc;
}

ErrCodeType PayData::allRebateGemsInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(outMsg, db::AllAccountBoughtGems, rsp);

	unordered_map<string, string> m;
	if (!gRedis->hgetall(kMiscFieldBoughtGems, m)) {
		DEBUG_LOG("hgetall rebate gems failed! err=%s", gRedis->last_error_cstr());
		return ErrCodeDB;
	}
	for (const auto& v : m) {
		auto itm = rsp.add_info();
		itm->set_account(v.first);
		itm->set_gems(atoi(v.second.c_str()));
	}
	return ErrCodeSucc;
}

ErrCodeType PayData::getRebateGemsInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(outMsg, db::AccountBoughtGems, rsp);

	unordered_map<string, string> m;
	auto& regInfoStr = m[kRegInfo];
	if (!gRedis->hget(kKeyPrefixPlayerData + to_string(h.TargetID), m)) {
		WARN_LOG("hget failed: %s! plid=%u", gRedis->last_error_message().c_str(), h.TargetID);
		return ErrCodeDB;
	}
	db::RegInfo regInfo;
	regInfo.ParseFromString(regInfoStr);
	string account = makeLoginKey(regInfo.account().account_type(), regInfo.account().account_id());
	rsp.set_account(account);

	m.clear();
	m[account];
	if (!gRedis->hget(kKeyOrderGotBoughtGems, m)) {
		DEBUG_LOG("hget has got rebate gems failed! err=%s plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}
	bool has_got = atoi(m[account].c_str()) > 0 ? true : false;

	m.clear();
	m[account];
	if (!gRedis->hget(kRebateBoughtGems, m)) {
		DEBUG_LOG("hget rebate bought gems failed! err=%s plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}
	uint32_t bought_gems = atoi(m[account].c_str());

	rsp.set_account(account);
	rsp.set_has_got(has_got);
	rsp.set_gems(bought_gems);
	return ErrCodeSucc;
}

ErrCodeType PayData::getRebateGemsReward(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(outMsg, db::AccountBoughtGems, rsp);

	unordered_map<string, string> m;
	auto& regInfoStr = m[kRegInfo];
	if (!gRedis->hget(kKeyPrefixPlayerData + to_string(h.TargetID), m)) {
		WARN_LOG("hget failed: %s! plid=%u", gRedis->last_error_message().c_str(), h.TargetID);
		return ErrCodeDB;
	}
	db::RegInfo regInfo;
	regInfo.ParseFromString(regInfoStr);
	string account = makeLoginKey(regInfo.account().account_type(), regInfo.account().account_id());
	rsp.set_account(account);

	m.clear();
	m[account];
	if (!gRedis->hget(kKeyOrderGotBoughtGems, m)) {
		DEBUG_LOG("hget has got rebate gems failed! err=%s plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}
	bool has_got = atoi(m[account].c_str()) > 0 ? true : false;
	if (has_got) {
		DEBUG_LOG("has got rebate gems! plid=%u", h.TargetID);
	} else {
		m[account] = "1";
		if (!gRedis->hset(kKeyOrderGotBoughtGems, m)) {
			DEBUG_LOG("hset has got rebate gems failed! err=%s plid=%u", gRedis->last_error_cstr(), h.TargetID);
			return ErrCodeDB;
		}
	}

	m.clear();
	m[account];
	if (!gRedis->hget(kRebateBoughtGems, m)) {
		DEBUG_LOG("hget bought gems failed! err=%s plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}
	uint32_t bought_gems = atoi(m[account].c_str());

	rsp.set_account(account);
	rsp.set_has_got(has_got);
	rsp.set_gems(bought_gems);
	return ErrCodeSucc;
}

ErrCodeType PayData::onGmPay(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, GmMsg::GmPayReq, req);
	REAL_PROTOBUF_MSG(outMsg, GmMsg::GmPayReq, rsp);
	DEBUG_LOG("onGmPayPid:%u, productid:%d, times:%d", req.player_id(), req.product_id(), req.times());

	rsp = req;
	db::GenOrderReq tmpReq;
	db::StrRsp tmpRsp;
	db::OrderInfo tmpReq1;
	db::Uint32Rsp tmpRsp1;
	SSProtoHead tmpHead = h;
	tmpHead.PlayerID = tmpHead.TargetID = req.player_id();
	auto itm = gCSVRMBShop.GetItem(req.product_id());
	if (itm == nullptr) {
		DEBUG_LOG("onGmPayPid:%u, config not find, itemid:%d", req.player_id(), req.product_id());
		return ErrCodeDB;
	}
	tmpReq.set_product_id(req.product_id());
	tmpReq.set_price(itm->Price);
	tmpReq.set_channel("A1");
	tmpReq.set_gems(itm->Gems);
	for (uint32_t i = 0; i < req.times(); i++) {
		tmpRsp.Clear();
		tmpReq1.Clear();
		tmpRsp1.Clear();
		genPayOrder(tmpHead, &tmpReq, &tmpRsp);

		// 订单格式：playerID_productID_price_gems_nowstamp_todayOrderNo
		tmpReq1.set_cp_trade_no(tmpRsp.str());
		tmpReq1.set_player_id(h.PlayerID);
		auto paramList = Split(tmpRsp.str(), '_');
		tmpReq1.set_total_fee(atof(paramList[2].c_str()));
		tmpReq1.set_product_id(paramList[1]);
		tmpReq1.set_create_time(to_string(time(nullptr)));
		tmpReq1.set_trade_time(to_string(time(nullptr)));
		tmpReq1.set_is_test_order("1");
		tmpReq1.set_channel_type("200");
		tmpReq1.set_channel_trade_no("test_" + paramList[0] + paramList[4] + paramList[5]);
		tmpReq1.set_channel("momo");
		tmpReq1.set_gems(paramList[3]);
		setPaidOrder(tmpHead, &tmpReq1, &tmpRsp1);
	}
	
	return ErrCodeSucc;
}

//===============================================================================
// Private static methods
//===============================================================================
void PayData::toOrdersData(unordered_map<string, string>& m, google::protobuf::RepeatedPtrField<db::OrderInfo>* orders)
{
	for (const auto& v : m) {
		auto itm = orders->Add();
		itm->ParseFromString(v.second);
	}
}
