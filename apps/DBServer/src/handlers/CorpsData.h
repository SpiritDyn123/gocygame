/*===============================================================
* @Author: car
* @Created Time : 2017年09月14日 星期四 16时57分16秒
*
* @File Name: CorpsData.h
* @Description:
*
================================================================*/
#ifndef DIGIMON_CORPS_DATA_H_
#define DIGIMON_CORPS_DATA_H_

#include "../proto/ErrCode.pb.h"
#include "../proto/Core.pb.h"
#include "../proto/db.pb.h"
#include "../proto/Corps.pb.h"


class CorpsData {
private:
	enum {
		kCorpsLvLimit	= 1,
		kCorpsQuitColdTm = 200,
	};
public:
	CorpsData();

private:
	ErrCodeType listCorpsInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType listCorpsInfoByName(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType newCorpsInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType updateCorpsInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType checkName(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType updatePlayerCorpsInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType delPlayerCorpsInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType updateCorpsTaskInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType commonLogScan(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType commonLogPush(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType checkCorpsInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType updateCorpsSkills(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType ListUint32Hash(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType SetUint32Hash(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType InviteRefuse(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType GetAllCorpsID(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);

	/*
	ErrCodeType RegisterDataCom(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType FreeDataCom(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType GetRegisterDataList(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	*/

public:
	void getCorpsInfo(uint32_t key, db::CorpsInfo& info, bool check = false);
	uint32_t GetCorpsOwnerId(uint32_t key);

private:
	std::string buildLogKey(uint32_t type, uint64_t cid, uint32_t target);
	std::string buildListKey(const db::Keyformat& key);
	void refuseInvite(uint32_t plid, uint32_t cid);
	uint32_t getCidByName(const std::string& name);
	uint32_t checkCorps(uint32_t plid);
	void getNeighborCorps(const cs::GeoPos& pos, db::ListCorpInfoRsp& rsp, uint32_t channel = 0);
	void checkCorpsPos(const db::CorpsInfo& info);
	void delCorpsPos(uint32_t key, bool remove = false);
	void updateCorpsPos(const db::CorpsInfo& info);
	bool IsExistsCorpsPos(uint32_t key);

};

extern CorpsData gCorpsData;

#endif   // DIGIMON_CORPS_DATA_H_
