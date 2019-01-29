#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <stdint.h>
#include <functional>
#include <Singleton.h>
#include <libant/utils/StringMap.h>

class ScriptMgr : public Singleton<ScriptMgr>
{
public:
	void Load(const std::string dir);
	const std::string& GetScriptStr(const std::string& script, bool sha = true) const;
	const std::string& Empty() const;
	void Visit(const std::function<void(const std::string& k, const std::string& v)>& fn);
private:
	struct ScriptInfo {
		std::string script;
		std::string sha;
	};
	StringMap<ScriptInfo> mDict;
	std::string mEmpty;
};