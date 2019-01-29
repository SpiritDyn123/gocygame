#include <sstream>
#include "KeyPrefixDef.h"
#include "HomeRes.h"
#include "../proto/CommonMsg.pb.h"
#include "../core/dispatcher.h"
#include "../proto/db.pb.h"
#include "../proto/SvrProtoID.pb.h"
#include "../core/redis_client.h"
static HomeRes ins;

HomeRes::HomeRes()
{
	gMsgDispatcher.RegisterHandler(DBProtoHomeResUpdate, *this, &HomeRes::UpdateResFactory, new cs::HomeResFactory);
	gMsgDispatcher.RegisterHandler(DBProtoHomeResGet, *this, &HomeRes::GetResFactory, nullptr, new cs::HomeResFactoryArr);
	gMsgDispatcher.RegisterHandler(DBProtoHomeResHireUpate, *this, &HomeRes::UpdateHierInfo, new cs::HomeResHireInfoArr);
}

ErrCodeType HomeRes::UpdateResFactory(const SSProtoHead & h, google::protobuf::Message * inMsg, google::protobuf::Message * outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, cs::HomeResFactory, req);
	auto uKey = makeKey(h.TargetID);
	std::stringstream ss;
	ss << req.build_id();
	std::unordered_map<std::string, std::string> data;
	req.SerializeToString(&data[ss.str()]);
	gRedis->hset(uKey, data);
	return ErrCodeType::ErrCodeSucc;
}

ErrCodeType HomeRes::GetResFactory(const SSProtoHead & h, google::protobuf::Message * inMsg, google::protobuf::Message * outMsg)
{
	REAL_PROTOBUF_MSG(outMsg, cs::HomeResFactoryArr, resp);
	auto uKey = makeKey(h.TargetID);
	std::vector<std::string> list;
	std::unordered_map<std::string, std::string> data;
	gRedis->hgetall(uKey, data);
	for (auto it : data) {
		std::string data;
		resp.add_facs()->ParseFromString(it.second);
	}
	{
		auto uKey = makeHireKey(h.TargetID);
		std::string data;
		gRedis->get(uKey, data);

		cs::HomeResHireInfoArr hire;
		hire.ParseFromString(data);
		resp.mutable_hire()->CopyFrom(hire.hire());
	}

	return ErrCodeType::ErrCodeSucc;
}

ErrCodeType HomeRes::UpdateHierInfo(const SSProtoHead & h, google::protobuf::Message * inMsg, google::protobuf::Message * outMsg)
{
	REAL_PROTOBUF_MSG(inMsg, cs::HomeResHireInfoArr, req);
	auto uKey = makeHireKey(h.TargetID);
	std::string data;
	req.SerializeToString(&data);
	gRedis->set(uKey, data);
	return ErrCodeType::ErrCodeSucc;
}

std::string HomeRes::makeKey(uint32_t playerId)
{
	std::ostringstream oss;
	oss << kKeyPrefixHomeResFactory << "_" << playerId;
	return oss.str();
}

std::string HomeRes::makeHireKey(uint32_t playerId)
{
	std::ostringstream oss;
	oss << kKeyPrefixHomeResHire << "_" << playerId;
	return oss.str();
}
