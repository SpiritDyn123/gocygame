#include "ScriptMgr.h"
#include <dirent.h>
#include <string.h>
#include <algorithm>
#include <fstream>
#include <streambuf>
#include <libant/hash/hash_algo.h>
#include "redis_client.h"

bool IsDir(const std::string& path) {
	auto pDir = opendir(path.c_str());
	auto isDir = pDir != NULL;
	if (pDir) {
		closedir(pDir);
	}
	return isDir;
}

void ForEachFile(const std::string& path
	, const std::function<void(const std::string& parent, const std::string& file, bool isDir)>& fn
	, bool openSub = false)
{
	auto pDir = opendir(path.c_str());
	if (!pDir) {
		return;
	}
	struct dirent* ent;
	while ((ent = readdir(pDir)) != NULL) {
		if (strcmp(".", ent->d_name) == 0
			|| strcmp("..", ent->d_name) == 0
			) {
			continue;
		}

		auto fullPath = std::string(path).append("/").append(ent->d_name);
		if (IsDir(fullPath)) {
			fn(path, ent->d_name, true);
			if (openSub) {
				ForEachFile(fullPath, fn, openSub);
			}
		}
		else {
			fn(path, ent->d_name, false);
		}
	}
	closedir(pDir);
}

void ScriptMgr::Load(const std::string dir)
{
	ForEachFile(dir, [this](const std::string& parent, const std::string& file, bool isDir) {
		auto fPath = std::string(parent).append("/").append(file);
		std::ifstream fs(fPath);
		ScriptInfo* script = new ScriptInfo();
		script->script = std::string((std::istreambuf_iterator<char>(fs)), std::istreambuf_iterator<char>());
		mDict.Insert(fPath, script);
	}, true);
	//gRedis->script_flush();
	//mDict.Visit([](const std::string&k, ScriptInfo* v) {
	//	gRedis->script_load(v->script, v->sha);
	//});
}

const std::string & ScriptMgr::GetScriptStr(const std::string& script, bool sha /* = false */) const
{
	auto info = mDict.GetC(script);
	if (info) {
		if (sha) {
			return info->sha;
		}
		else {
			return info->script;
		}
	}
	return mEmpty;
}

const std::string & ScriptMgr::Empty() const
{
	return mEmpty;
}

void ScriptMgr::Visit(const std::function<void(const std::string&k, const std::string&v)>& fn)
{
	mDict.Visit([&fn](const std::string&k, const ScriptInfo* v) {
		fn(k, v->script);
	});
}
