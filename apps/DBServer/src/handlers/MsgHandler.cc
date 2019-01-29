#include "MsgHandler.h"
#include "../log.h"
#include <typeinfo>
MsgHandler::MsgHandler(int order /*= 0*/)
{
	MsgHdlMgr::Instance().AddHdl(this, order);
}

void MsgHdlMgr::Init()
{
	for (auto& it : mHdls) {
		for (auto hdl : it.second) {
			JSON_LOG_DICT_TAG("MsgHdlMgr::Init", "type", typeid(*hdl).name());
			hdl->Init();
		}
	}
}

void MsgHdlMgr::AddHdl(MsgHandler* hdl, int order)
{
	auto& vec = mHdls[order];
	vec.push_back(hdl);
}
