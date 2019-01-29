/*
 * OfflineReq.cc
 *
 *  Created on: 2017年7月15日
 */

#include <string>
#include "../proto/SvrProtoID.pb.h"
#include "../proto/CenterServer.pb.h"
#include "../proto/db.pb.h"
#include "../core/dispatcher.h"
#include "../core/redis_client.h"

#include "KeyPrefixDef.h"
#include "OfflineReq.h"

using namespace std;

OfflineReq gOfflineReq;

static const string kSystemCarousel("System_Carousel");

OfflineReq::OfflineReq()
{
	gMsgDispatcher.RegisterHandler(DBProtoAddOfflineReq, *this, &OfflineReq::addOfflineReq, new cs::OfflineInfo, nullptr);
	gMsgDispatcher.RegisterHandler(DBProtoGetOfflineReq, *this, &OfflineReq::getOfflineReq, nullptr, new cs::OfflineReqData);
	gMsgDispatcher.RegisterHandler(DBProtoDelOfflineReq, *this, &OfflineReq::delOfflineReq, nullptr, nullptr);
	// 系统跑马灯
	gMsgDispatcher.RegisterHandler(DBProtoGetCarousel, *this, &OfflineReq::getCarousel, nullptr, new center::Carousels);
	gMsgDispatcher.RegisterHandler(DBProtoAddCarousel, *this, &OfflineReq::addCarousel, new center::Carousel, nullptr);
	gMsgDispatcher.RegisterHandler(DBProtoDelCarousel, *this, &OfflineReq::delCarousel, new db::RepeatedStrReq, nullptr);
}

ErrCodeType OfflineReq::addOfflineReq(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, cs::OfflineInfo, req);

	vector<string> vals;
	string serializedReq;
	req.SerializeToString(&serializedReq);
	vals.emplace_back(serializedReq);
	if (!gRedis->rpush(makeOfflineReqKey(h.TargetID), vals)) {
		WARN_LOG("rpush failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}

	return ErrCodeSucc;
}

ErrCodeType OfflineReq::getOfflineReq(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(outMsg, cs::OfflineReqData, rsp);
	vector<string> vals;

	if (!gRedis->lrange(makeOfflineReqKey(h.TargetID), 0, -1, vals)) {
		WARN_LOG("lrange failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}
	for (auto& v : vals) {
		auto offlineInfo = rsp.add_to_handle();
		offlineInfo->ParseFromString(v);
	}

	return ErrCodeSucc;
}

ErrCodeType OfflineReq::delOfflineReq(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	if (!gRedis->ltrim(makeOfflineReqKey(h.TargetID), 1, 0)) {
		WARN_LOG("ltrim failed: %s! plid=%u", gRedis->last_error_cstr(), h.TargetID);
		return ErrCodeDB;
	}

	return ErrCodeSucc;
}

ErrCodeType OfflineReq::getCarousel(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(outMsg, center::Carousels, rsp);
	unordered_map<string, string> m;
	if (!gRedis->hgetall(kSystemCarousel, m)) {
		WARN_LOG("hgetall failed: %s!", gRedis->last_error_cstr());
		return ErrCodeDB;
	}
	for (auto& v : m) {
		auto carousel = rsp.add_carousels();
		carousel->ParseFromString(v.second);
	}

	return ErrCodeSucc;
}

ErrCodeType OfflineReq::addCarousel(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, center::Carousel, req);
	unordered_map<string, string> m;
	auto& serializedReq = m[req.carousel_id()];
	req.SerializeToString(&serializedReq);
	if (!gRedis->hset(kSystemCarousel, m)) {
		WARN_LOG("hset failed: %s!", gRedis->last_error_cstr());
		return ErrCodeDB;
	}

	return ErrCodeSucc;
}

ErrCodeType OfflineReq::delCarousel(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, db::RepeatedStrReq, req);
	vector<string> vals;
	for (auto v : req.strs()) {
		vals.emplace_back(v);
	}
	if (!gRedis->hdel(kSystemCarousel, vals)) {
		WARN_LOG("hdel failed: %s!", gRedis->last_error_cstr());
		return ErrCodeDB;
	}

	return ErrCodeSucc;
}

