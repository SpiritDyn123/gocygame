/*===============================================================
* @Author: car
* @Created Time : 2018年05月23日 星期三 20时31分14秒
*
* @File Name: CapRes.h
* @Description:
*
================================================================*/
#ifndef _CAPRES_H_
#define _CAPRES_H_

#include "../proto/ErrCode.pb.h"
#include "../proto/Core.pb.h"
#include "../proto/db.pb.h"
#include "../CSV/CSVNpcRes.h"
#include "../CSV/CSVResInfo.h"
#include <list>
#include "MsgHandler.h"
class CapResData
	: public MsgHandler
{
public:
	CapResData();

private:
	ErrCodeType updateCapResInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType checkCapResInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType scanCapResInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType updateCapResStatus(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);
	ErrCodeType freeCapRes(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);

	// 拉取挂件信息
	ErrCodeType getPendantInfo(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);

public:
	void Init() override;

	// 初始化数据
	void CheckCapAIResInfo(uint32_t game_zone, uint32_t region);
	void BuildNpcRes(uint32_t game_region, uint32_t region, std::list<uint32_t>& res, uint32_t& plid);
	void BuildNpcResTest(uint32_t num, uint32_t region);
	void CalRes(uint32_t total, uint32_t cur, std::map<uint32_t, uint32_t>& samp, std::list<uint32_t>& res);
	uint32_t CalAIID(uint32_t region, uint32_t num);

	void AddToCandidateRes(uint32_t game_region, uint32_t region, const std::string& key);
	void RemoveFromCandidateRes(uint32_t game_region, uint32_t region, const std::string& key);
	std::string CalCandidateId(bool isAI, uint32_t id);

	// 保护中的资源
	void CheckProtectCapRes(uint32_t game_region, uint32_t region);
	void PushProtectCapRes(uint32_t game_region, uint32_t region, uint32_t ptime, uint32_t pid);

	// 段位资源分布
	void RemoveFromResDis(uint32_t game_region, uint32_t plid);
	void AddToResDis(uint32_t game_region, uint32_t plid);

private:
	bool checkCapResCanFight(std::string& key);
	uint32_t getCapResNum(std::string& resKey);
	// 根据段位随机给出玩家
	void randGetPlayerResInfo(uint32_t region, uint32_t result, std::string& key, uint32_t explid, uint32_t& plid);

private:
	void checkPlayerResProtect(uint32_t game_region, uint32_t region, uint32_t plid);
	void registerRes(uint32_t game_region, uint32_t region, uint32_t plid, bool isProtect, db::ResPointInfo& info);


public:
	bool TryLock(const std::string& key);
	void Unlock(const std::string& key);
	uint32_t GetNextFiveClock();
	uint32_t GetLastFiveClock();
	void BuildNpc(uint32_t num, uint32_t region, std::list<uint32_t>& res);

private:
	void packPrimaryMon(uint32_t plid, db::PendantInfo& info);
	void packHatch(uint32_t plid, db::PendantInfo& info);
	void packArena(uint32_t plid, db::PendantInfo& info);
	void packHomeRes(uint32_t plid, db::PendantInfo& info);

};




class CapResRegionItem {
public:
	CapResRegionItem(CSVNpcRes::Item* item);
public:
	std::unordered_map<uint32_t, uint32_t> Probs;		// 总分类
	uint32_t ResPro;
};

class CapResRegionConfig {
public:
	void Init();

	CapResRegionItem* GetRegionInfo(uint32_t region) 
	{
		auto iter = Items.find(region);
		if (iter == Items.end()) {
			return nullptr;
		} else {
			return &(iter->second);
		}
	}

public:
	std::unordered_map<uint32_t, CapResRegionItem> Items;
};

extern CapResData gCapResData;
extern CapResRegionConfig gCapResRegionConfig; 

#endif //_CAPRES_H_
