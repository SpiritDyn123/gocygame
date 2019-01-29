#pragma once
#include <Singleton.h>
#include <map>
#include <vector>
class MsgHandler {
public:
	MsgHandler(int order = 0);
	virtual void Init() {}
};

class MsgHdlMgr
	: public Singleton<MsgHdlMgr>
{
public:
	void Init();
	void AddHdl(MsgHandler* hdl, int order);
private:
	std::map<int, std::vector<MsgHandler*> > mHdls;
};
