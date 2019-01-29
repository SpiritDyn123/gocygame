/**
 * redis_client.cpp
 *
 *  Created on: 2016-09-08
 *      Author: antigloss
 */

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <stdexcept>
#include "rapidjson/document.h"
#include "rapidjson/filestream.h"
#include <endian.h>
#include <StringUtils.h>
#include "redis_client.h"
#include "sqlconnection.h"
#include "MysqlProxy.h"
#include <libant/crypt/base64.h>
#include "../tables/tbl_redis_bin.h"
#include "../log.h"

using namespace std;

RedisClient* gRedis;
RedisClient* gRedisRank;
RedisClient* gRedisBattle;

static string kSetOpErrMsg[RedisClient::kSetOpCnt] = {
		"Unknow error", "Key already exist", "Key not exist"
};

//----------------------------------------------------------
// Public Methods
//----------------------------------------------------------
RedisClient::RedisClient(const string& hostsStr, const std::string& passwd, int dbidx)
{
	if (hostsStr.empty()) {
		throw runtime_error(string("Empty hosts!"));
	}

	auto hosts = Split(hostsStr, ';');
	for (const auto& ip_port : hosts) {
		auto hostAddr = Split(ip_port, ':');
		if (hostAddr.size() != 2) {
			throw runtime_error(string("Invalid ip_port [") + ip_port + "]");
		}

		int port = atoi(hostAddr[1].c_str());
		if (port <= 0 || port > 65535) {
			throw runtime_error(string("Invalid ip_port [") + ip_port + "]");
		}

		m_hosts.emplace_back(make_pair(hostAddr[0], port));
	}
	random_shuffle(m_hosts.begin(), m_hosts.end());

	m_passwd = passwd;
	m_dbidx = dbidx;
	alloc_context();
}

#ifdef CHECK_REPLY
#undef CHECK_REPLY
#endif
#define CHECK_REPLY(reply_, expectedReplyType_) \
		do { \
			if (!reply_) { \
				return false; \
			} \
			switch (reply_->type) { \
			case expectedReplyType_: \
				break; \
			case REDIS_REPLY_ERROR: \
				set_errmsg(reply_->str); \
				return false; \
			default: \
				set_errmsg("Unexpected reply type ", reply_->type); \
				return false; \
			} \
		} while (0)

bool RedisClient::select_db(int dbidx)
{
	ScopedReplyPointer reply = exec("SELECT %d", dbidx);
	CHECK_REPLY(reply, REDIS_REPLY_STATUS);
	return true;
}

bool RedisClient::set_passwd()
{
	if (!m_passwd.empty()) {
		ScopedReplyPointer reply = exec("AUTH %s", m_passwd.c_str());
		CHECK_REPLY(reply, REDIS_REPLY_STATUS);
	}
	return true;
}

bool RedisClient::expire(const std::string& key, unsigned int seconds)
{
	ScopedReplyPointer reply = exec("EXPIRE %b %u", key.c_str(),
									static_cast<size_t>(key.size()), seconds);
	CHECK_REPLY(reply, REDIS_REPLY_INTEGER);
	if (reply->integer == 1) {
		return true;
	}

	set_errmsg("Key does not exist or the timeout could not be set. reply->integer is ", reply->integer);
	return false;
}

bool RedisClient::expire_at(const std::string& key, time_t expired_tm)
{
	ScopedReplyPointer reply = exec("EXPIREAT %b %lld", key.c_str(),
									static_cast<size_t>(key.size()),
									static_cast<long long>(expired_tm));
	CHECK_REPLY(reply, REDIS_REPLY_INTEGER);
	if (reply->integer == 1) {
		return true;
	}

	set_errmsg("Key does not exist or the timeout could not be set. reply->integer is ", reply->integer);
	return false;
}

bool RedisClient::ttl(const std::string& key, long long& ttl)
{
	ScopedReplyPointer reply = exec("TTL %b", key.c_str(), static_cast<size_t>(key.size()));
	CHECK_REPLY(reply, REDIS_REPLY_INTEGER);
	ttl = reply->integer;
	return true;
}

bool RedisClient::exists(const std::string& key, bool& exist)
{
	ScopedReplyPointer reply = exec("EXISTS %b", key.c_str(), static_cast<size_t>(key.size()));
	CHECK_REPLY(reply, REDIS_REPLY_INTEGER);
	exist = reply->integer;
	return true;
}

bool RedisClient::set(const string& key, const string& val, const ExpirationTime* expire_tm, SetOpType op_type)
{
	ScopedReplyPointer reply;
	if (expire_tm) {
		long long ttl = expire_tm->remaining_seconds();
		if (ttl <= 0) {
			set_errmsg("Invalid expire time");
			return false;
		}

		switch (op_type) {
		case kSetAnyhow:
			reply = exec("SET %b %b EX %lld", key.c_str(), static_cast<size_t>(key.size()),
							val.c_str(), static_cast<size_t>(val.size()), ttl);
			break;
		case kSetIfNotExist:
			reply = exec("SET %b %b EX %lld NX", key.c_str(), static_cast<size_t>(key.size()),
							val.c_str(), static_cast<size_t>(val.size()), ttl);
			break;
		case kSetIfExist:
			reply = exec("SET %b %b EX %lld XX", key.c_str(), static_cast<size_t>(key.size()),
							val.c_str(), static_cast<size_t>(val.size()), ttl);
			break;
		default:
			set_errmsg("Unsupported op_type ", op_type);
			break;
		}
	} else {
		switch (op_type) {
		case kSetAnyhow:
			reply = exec("SET %b %b", key.c_str(), static_cast<size_t>(key.size()),
							val.c_str(), static_cast<size_t>(val.size()));
			break;
		case kSetIfNotExist:
			reply = exec("SET %b %b NX", key.c_str(), static_cast<size_t>(key.size()),
							val.c_str(), static_cast<size_t>(val.size()));
			break;
		case kSetIfExist:
			reply = exec("SET %b %b XX", key.c_str(), static_cast<size_t>(key.size()),
							val.c_str(), static_cast<size_t>(val.size()));
			break;
		default:
			set_errmsg("Unsupported op_type ", op_type);
			break;
		}
	}

	if (!reply) {
		return false;
	}

	switch (reply->type) {
	case REDIS_REPLY_STATUS:
		return true;
	case REDIS_REPLY_NIL:
		set_errmsg(kSetOpErrMsg[op_type]);
		return false;
	case REDIS_REPLY_ERROR:
		set_errmsg(reply->str);
		return false;
	default:
		set_errmsg("Unexpected reply type ", reply->type);
		return false;
	}
}

bool RedisClient::get(const std::string& key, std::string& val, bool* key_exists)
{
	ScopedReplyPointer reply = exec("GET %b", key.c_str(), static_cast<size_t>(key.size()));
	if (!reply) {
		return false;
	}

	switch (reply->type) {
	case REDIS_REPLY_STRING:
		val.assign(reply->str, reply->len);
		if (key_exists) {
			*key_exists = true;
		}
		return true;
	case REDIS_REPLY_NIL:
		val = "";
		if (key_exists) {
			*key_exists = false;
		}
		return true;
	case REDIS_REPLY_ERROR:
		set_errmsg(reply->str);
		return false;
	default:
		set_errmsg("Unexpected reply type ", reply->type);
		return false;
	}
}

bool RedisClient::del(const std::vector<std::string> keys, long long* cnt)
{
	ScopedReplyPointer reply = execv("DEL", nullptr, &keys);
	CHECK_REPLY(reply, REDIS_REPLY_INTEGER);
	if (cnt) {
		*cnt = reply->integer;
	}
	return true;
}


bool RedisClient::rename(const std::string& old, const std::string& key)
{
	ScopedReplyPointer reply = exec("rename %b %b"
		, old.c_str(), static_cast<size_t>(old.size())
		, key.c_str(), static_cast<size_t>(key.size()));
	if (reply->type == REDIS_REPLY_ERROR) {
		set_errmsg(reply->str);
		return false;
	}
	return true;
}

bool RedisClient::incrby(const std::string& key, long long inc, long long* result)
{
	ScopedReplyPointer reply = exec("INCRBY %b %lld", key.c_str(), static_cast<size_t>(key.size()), inc);
	CHECK_REPLY(reply, REDIS_REPLY_INTEGER);
	if (result) {
		*result = reply->integer;
	}
	return true;
}

bool RedisClient::lpush(const std::string& key, const std::vector<std::string>& vals, long long* cnt)
{
	ScopedReplyPointer reply = execv("LPUSH", &key, &vals);
	CHECK_REPLY(reply, REDIS_REPLY_INTEGER);
	if (cnt) {
		*cnt = reply->integer;
	}
	return true;
}

bool RedisClient::rpush(const std::string& key, const std::vector<std::string>& vals, long long* cnt)
{
	ScopedReplyPointer reply = execv("RPUSH", &key, &vals);
	CHECK_REPLY(reply, REDIS_REPLY_INTEGER);
	if (cnt) {
		*cnt = reply->integer;
	}
	return true;
}

bool RedisClient::lpop(const std::string& key, std::string& result)
{
	ScopedReplyPointer reply = exec("LPOP %b", key.c_str(), static_cast<size_t>(key.size()));
	if (reply->type == REDIS_REPLY_NIL) {
		return true;
	}
	CHECK_REPLY(reply, REDIS_REPLY_STRING);
	result = reply->str;
	return true;
}

bool RedisClient::rpop(const std::string& key, std::string& result)
{
	ScopedReplyPointer reply = exec("RPOP %b", key.c_str(), static_cast<size_t>(key.size()));
	if (reply->type == REDIS_REPLY_NIL) {
		return true;
	}
	CHECK_REPLY(reply, REDIS_REPLY_STRING);
	result = reply->str;
	return true;
}

bool RedisClient::lrange(const std::string& key, long long start, long long stop, std::vector<std::string>& vals)
{
	ScopedReplyPointer reply = exec("LRANGE %b %lld %lld",
									key.c_str(), static_cast<size_t>(key.size()),
									start, stop);
	CHECK_REPLY(reply, REDIS_REPLY_ARRAY);
	arr_reply_to_vector(reply->element, reply->elements, vals);
	return true;
}

bool RedisClient::ltrim(const std::string& key, long long start, long long stop)
{
	ScopedReplyPointer reply = exec("LTRIM %b %lld %lld",
									key.c_str(), static_cast<size_t>(key.size()),
									start, stop);
	CHECK_REPLY(reply, REDIS_REPLY_STATUS);
	return true;
}

bool RedisClient::llen(const std::string& key, long long& cnt)
{
	ScopedReplyPointer reply = exec("LLEN %b", key.c_str(), static_cast<size_t>(key.size()));
	CHECK_REPLY(reply, REDIS_REPLY_INTEGER);
	cnt = reply->integer;
	return true;
}

bool RedisClient::sadd(const string& key, const vector<string>& vals, long long* cnt)
{
	ScopedReplyPointer reply = execv("SADD", &key, &vals);
	CHECK_REPLY(reply, REDIS_REPLY_INTEGER);
	if (cnt) {
		*cnt = reply->integer;
	}
	return true;
}

bool RedisClient::srem(const std::string& key, const std::vector<std::string>& vals, long long* cnt)
{
	ScopedReplyPointer reply = execv("SREM", &key, &vals);
	CHECK_REPLY(reply, REDIS_REPLY_INTEGER);
	if (cnt) {
		*cnt = reply->integer;
	}
	return true;
}

bool RedisClient::scard(const std::string& key, long long& cnt)
{
	ScopedReplyPointer reply = exec("SCARD %b", key.c_str(), static_cast<size_t>(key.size()));
	CHECK_REPLY(reply, REDIS_REPLY_INTEGER);
	cnt = reply->integer;
	return true;
}

bool RedisClient::smembers(const std::string& key, std::vector<std::string>& vals)
{
	ScopedReplyPointer reply = exec("SMEMBERS %b", key.c_str(), static_cast<size_t>(key.size()));
	CHECK_REPLY(reply, REDIS_REPLY_ARRAY);
	arr_reply_to_vector(reply->element, reply->elements, vals);
	return true;
}

bool RedisClient::srandmembers(const std::string& key, uint32_t cnt, std::vector<std::string>& vals)
{
	ScopedReplyPointer reply = exec("SRANDMEMBER %b %u",
									key.c_str(), static_cast<size_t>(key.size()),
									cnt);
	CHECK_REPLY(reply, REDIS_REPLY_ARRAY);
	arr_reply_to_vector(reply->element, reply->elements, vals);
	return true;
}

bool RedisClient::sdiff(const vector<string>& keys, vector<string>& result)
{
	ScopedReplyPointer reply = execv("SDIFF", 0, &keys);
	CHECK_REPLY(reply, REDIS_REPLY_ARRAY);
	arr_reply_to_vector(reply->element, reply->elements, result);
	return true;
}

bool RedisClient::sdiff_store(const string& dest, const vector<string>& keys, long long* cnt)
{
	ScopedReplyPointer reply = execv("SDIFFSTORE", &dest, &keys);
	CHECK_REPLY(reply, REDIS_REPLY_INTEGER);
	if (cnt) {
		*cnt = reply->integer;
	}
	return true;
}

bool RedisClient::sinter(const vector<string>& keys, vector<string>& result)
{
	ScopedReplyPointer reply = execv("SINTER", 0, &keys);
	CHECK_REPLY(reply, REDIS_REPLY_ARRAY);
	arr_reply_to_vector(reply->element, reply->elements, result);
	return true;
}

bool RedisClient::sinter_store(const string& dest, const vector<string>& keys, long long* cnt)
{
	ScopedReplyPointer reply = execv("SINTERSTORE", &dest, &keys);
	CHECK_REPLY(reply, REDIS_REPLY_INTEGER);
	if (cnt) {
		*cnt = reply->integer;
	}
	return true;
}

bool RedisClient::sismember(const std::string& key, const std::string& val, bool& is_member)
{
	ScopedReplyPointer reply = exec("SISMEMBER %b %b", key.c_str(), static_cast<size_t>(key.size()),
									val.c_str(), static_cast<size_t>(val.size()));
	CHECK_REPLY(reply, REDIS_REPLY_INTEGER);
	is_member = reply->integer;
	return true;
}

// TODO -----  Set Ops ----------------------------------

bool RedisClient::hkeys(const std::string & key, std::vector<std::string>& fields)
{
	ScopedReplyPointer reply = exec("HKEYS %b", key.c_str(), static_cast<size_t>(key.size()));
	CHECK_REPLY(reply, REDIS_REPLY_ARRAY);
	for (size_t i = 0; i < reply->elements; ++i) {
		redisReply* r = reply->element[i];
		fields.emplace_back(r->str, r->len);
	}
	return true;
}

bool RedisClient::hvals(const std::string & key, std::vector<std::string>& fields)
{
	ScopedReplyPointer reply = exec("HVALS %b", key.c_str(), static_cast<size_t>(key.size()));
	CHECK_REPLY(reply, REDIS_REPLY_ARRAY);
	for (size_t i = 0; i < reply->elements; ++i) {
		redisReply* r = reply->element[i];
		fields.emplace_back(r->str, r->len);
	}
	return true;
}

bool RedisClient::hget(const string& key, unordered_map<string, string>& fields)
{
	bool kExists = false;
	exists(key, kExists);
	if (!kExists) {
		loadFromMysql(key, fields, true);
		return true;
	}
	ScopedReplyPointer reply = execm("HMGET", key, fields);
	CHECK_REPLY(reply, REDIS_REPLY_ARRAY);
	if (reply->elements != fields.size()) {
		set_errmsg("Invalid number of elements returned! Expected ",
					fields.size(), ", Returned ", reply->elements);
		return false;
	}

	size_t n = 0;
	for (auto it = fields.begin(); it != fields.end(); ++it) {
		redisReply* r = reply->element[n];
		switch (r->type) {
		case REDIS_REPLY_STRING:
			it->second = string(r->str, r->len);
			break;
		case REDIS_REPLY_NIL:
			it->second = "";
			break;
		case REDIS_REPLY_ERROR:
			set_errmsg(reply->str);
			return false;
		default:
			set_errmsg("Unexpected reply type ", reply->type);
			return false;
		}
		++n;
	}
	return true;
}

uint64_t Str2Num(const std::string& str)
{
	uint64_t val = 0;
	for (size_t i = 0; i < str.length(); ++i) {
		char c = str[i];
		if (c >= '0' && c <= '9') {
			val *= 10;
			val += c - '0';
		}
	}
	return val;
}

void SplitPrefixAndId(const std::string& str, std::string& prefix, uint64_t& id)
{
	id = 0;
	bool getNum = false;
	for (size_t i = 0; i < str.length(); ++i) {
		char c = str[i];
		if (c >= '0' && c <= '9') {
			id *= 10;
			id += c - '0';
			if (!getNum) {
				getNum = true;
				prefix = str.substr(0, i);
			}
		}
		else {
			if (c == '}' && i == str.length() - 1) {
				continue;
			}
			getNum = false;
			id = 0;
		}
	}
	if (!getNum) {
		prefix = str;
	}
}

void UnpackBinToDict(const char* buff, uint64_t dlen, std::unordered_map<std::string, std::string>& fields)
{
	uint32_t p = 0;
	uint32_t len = 0;
	while (p < dlen) {
		len = be32toh(*(uint32_t*)(buff + p));
		p += sizeof(len);
		if( p + len >= dlen ) {
			break;
		}
		std::string key;
		key.assign(buff + p, len);
		p += len;
		len = be32toh(*(uint32_t*)(buff + p));
		p += sizeof(len);
		if (p + len > dlen) {
			break;
		}
		std::string val;
		val.assign(buff + p, len);
		p += len;
		fields[key] = val;
	}
}

void UnpackJsonToDict(const char* jStr, std::unordered_map<std::string, std::string>& fields)
{
	rapidjson::Document jDoc;
	jDoc.Parse<0>(jStr);
	for (auto it = jDoc.MemberBegin(); it != jDoc.MemberEnd(); ++it) {
		fields[it->name.GetString()] = ant::Base64Decode(it->value.GetString());
	}
}

//为了避免某些key初始为空时一直访问mysql，加入一个无意义字段
static std::string emptySign = "__empty_sign";

bool RedisClient::loadFromMysql(const std::string& key, std::unordered_map<std::string, std::string>& fields, bool specified /* = false */)
{
	auto& proxy = MysqlProxy::Instance();
	if (!proxy.IsLoad() || !proxy.Redis2mysqlEnable()) {
		return false;
	}
	std::string prefix;
	uint64_t uId;
	SplitPrefixAndId(key, prefix, uId);
	if (!proxy.IsValidPrefix(prefix)) {
		return false;
	}
	DEBUG_LOG("Load From Mysql key:%s", key.c_str());
	TblRedisBin redisBin(uId, proxy.GetKv("redis2MysqlDB"), prefix.c_str());
	std::unordered_map<std::string, std::string>* data = &fields;
	if (specified) {
		data = new std::unordered_map<std::string, std::string>();
	}
	redisBin.Get(*data);
	if (data->size() == 0) {
		(*data)[emptySign] = "";
	}
	hset(key, *data);
	(*data).erase(emptySign);
	if(specified) {
		for (auto& it : fields) {
			auto eIt = data->find(it.first);
			if (eIt != data->end()) {
				it.second = eIt->second;
			}
		}
	}
	if (specified) {
		delete data;
	}
	return true;
}

bool RedisClient::hgetall(const string& key, unordered_map<string, string>& fields)
{
	ScopedReplyPointer reply = exec("HGETALL %b", key.c_str(), static_cast<size_t>(key.size()));
	CHECK_REPLY(reply, REDIS_REPLY_ARRAY);

	if (reply->elements == 0) {
		loadFromMysql(key, fields);
		return true;
	}
	size_t nloops = reply->elements / 2;
	for (size_t i = 0; i != nloops; ++i) {
		size_t n = i * 2;
		fields[string(reply->element[n]->str, reply->element[n]->len)] = string(reply->element[n + 1]->str, reply->element[n + 1]->len);
	}
	fields.erase(emptySign);
	return true;
}

bool RedisClient::hset(const string& key, const unordered_map<string, string>& fields)
{
	ScopedReplyPointer reply = execm("HMSET", key, fields, true);
	CHECK_REPLY(reply, REDIS_REPLY_STATUS);
	return true;
}

bool RedisClient::hdel(const std::string& key, const std::vector<std::string>& fields)
{
	ScopedReplyPointer reply = execv("HDEL", &key, &fields);
	CHECK_REPLY(reply, REDIS_REPLY_INTEGER);
	return true;
}

bool RedisClient::hincrby(const std::string& key, const std::string& field, long long inc, long long* result)
{
	ScopedReplyPointer reply = exec("HINCRBY %b %b %lld",
									key.c_str(), static_cast<size_t>(key.size()),
									field.c_str(), static_cast<size_t>(field.size()), inc);
	CHECK_REPLY(reply, REDIS_REPLY_INTEGER);
	if (result) {
		*result = reply->integer;
	}
	return true;
}

bool RedisClient::hlen(const std::string& key, long long& result)
{
	ScopedReplyPointer reply = exec("HLEN %b", key.c_str(), static_cast<size_t>(key.size()));
	CHECK_REPLY(reply, REDIS_REPLY_INTEGER);
	result = reply->integer;
	return true;
}

bool RedisClient::hexists(const std::string& key, const std::string& field, bool& exist)
{
	ScopedReplyPointer reply = exec("HEXISTS %b %b", key.c_str(), static_cast<size_t>(key.size()),
									field.c_str(), static_cast<size_t>(field.size()));
	CHECK_REPLY(reply, REDIS_REPLY_INTEGER);
	exist = reply->integer;
	return true;
}

//-------------------------------zset opps--------------------------------------

bool RedisClient::zincrby(const std::string& key, const std::string& field, long long inc, long long* result)
{
	ScopedReplyPointer reply = exec("ZINCRBY %b %lld %b",
									key.c_str(), static_cast<size_t>(key.size()), inc,
									field.c_str(), static_cast<size_t>(field.size()));

	CHECK_REPLY(reply, REDIS_REPLY_STRING);
	if (result) {
		*result = reply->integer;
	}

	return true;	
}


bool RedisClient::zrem(const std::string& key, const std::vector<std::string>& fields)
{
	ScopedReplyPointer reply = execv("ZREM", &key, &fields);
	CHECK_REPLY(reply, REDIS_REPLY_INTEGER);
	return true;
}


bool RedisClient::zrange(const std::string& key, const int32_t from, const int32_t to, std::vector<std::string>& result, bool withscore)
{
	ScopedReplyPointer reply = nullptr;
	if(withscore) {
		reply = exec("ZRANGE %b %d %d WITHSCORES", key.c_str(), static_cast<size_t>(key.size()),
				from, to);
	} else {
		reply = exec("ZRANGE %b %d %d", key.c_str(), static_cast<size_t>(key.size()),
				from, to);
	}
	CHECK_REPLY(reply, REDIS_REPLY_ARRAY);
	arr_reply_to_vector(reply->element, reply->elements, result);
	return true;
}


bool RedisClient::zrangebyscore(const std::string& key, const double min, const double max, std::vector<std::string>& result)
{
	ScopedReplyPointer reply = nullptr;
	reply = exec("ZRANGEBYSCORE %b %f %f", key.c_str(), static_cast<size_t>(key.size()),
				min, max);
	CHECK_REPLY(reply, REDIS_REPLY_ARRAY);
	arr_reply_to_vector(reply->element, reply->elements, result);
	return true;

}

bool RedisClient::zrank(const std::string& key, const std::string& field,long long* result)
{
	ScopedReplyPointer reply = exec("ZRANK %b %b",
			key.c_str(), static_cast<size_t>(key.size()),
			field.c_str(), static_cast<size_t>(field.size()));
	if (reply->type == REDIS_REPLY_NIL) {
		if (result) {
			*result = -1;
		}
		return true;
	}
	CHECK_REPLY(reply, REDIS_REPLY_INTEGER);

	if(result) {
		*result = reply->integer;
	}
	return true;
}


bool RedisClient::zadd(const std::string& key, std::string& val, const std::string& field, bool& success)
{
	ScopedReplyPointer reply = exec("ZADD %b %b %b"
		, key.c_str(), static_cast<size_t>(key.size())
		, val.c_str(), static_cast<size_t>(val.size())
		, field.c_str(), static_cast<size_t>(field.size()));
	CHECK_REPLY(reply, REDIS_REPLY_INTEGER);
	success = reply->integer;
	return true;
}

bool RedisClient::zadd(const std::string& key, double score, const std::string& field) {
	ScopedReplyPointer reply = exec("ZADD  %b %f %b"
		, key.c_str(), static_cast<size_t>(key.size())
		, score
		, field.c_str(), static_cast<size_t>(field.size()));
	CHECK_REPLY(reply, REDIS_REPLY_INTEGER);
	return true;
}


bool RedisClient::zadd(const std::string& key, const std::vector<std::string>& fields)
{
	ScopedReplyPointer reply = execv("ZADD", &key, &fields);
	CHECK_REPLY(reply, REDIS_REPLY_STATUS);
	return true;
}

bool RedisClient::hsetnx(const std::string& key, const std::string& field, std::string& val, bool& success)
{
	ScopedReplyPointer reply = exec("HSETNX  %b %b %b", key.c_str(), static_cast<size_t>(key.size()),
									field.c_str(), static_cast<size_t>(field.size()), val.c_str(), static_cast<size_t>(val.size()));
	CHECK_REPLY(reply, REDIS_REPLY_INTEGER);
	success = reply->integer;
	return true;
}

bool RedisClient::hscan(const std::string& key, uint32_t& cursor, std::unordered_map<std::string, std::string>& fields, uint32_t maxCnt/* =0 */)
{
	ScopedReplyPointer reply = nullptr;
	if (maxCnt != 0) {
		reply = exec("HSCAN %b %u COUNT %u", key.c_str(), static_cast<size_t>(key.size()), cursor, maxCnt);
	}
	else {
		reply = exec("HSCAN %b %u", key.c_str(), static_cast<size_t>(key.size()), cursor);
	}
	CHECK_REPLY(reply, REDIS_REPLY_ARRAY);
	if (reply->elements != 2) {
		return false;
	}
	CHECK_REPLY(reply->element[0], REDIS_REPLY_STRING);
	cursor = atoi(reply->element[0]->str);
	auto repFields = reply->element[1];
	CHECK_REPLY(repFields, REDIS_REPLY_ARRAY);
	size_t nloops = repFields->elements / 2;
	for (size_t i = 0; i != nloops; ++i) {
		size_t n = i * 2;
		fields[string(repFields->element[n]->str, repFields->element[n]->len)] = string(repFields->element[n + 1]->str, repFields->element[n + 1]->len);
	}
	return true;
}

bool RedisClient::zcount(const std::string& key, double mini, double max, uint32_t& cnt)
{
	cnt = 0;
	ScopedReplyPointer reply = exec("ZCOUNT %b %f %f", key.c_str(), static_cast<size_t>(key.size()), mini, max);
	CHECK_REPLY(reply, REDIS_REPLY_INTEGER);
	cnt = reply->integer;
	return true;
}

bool RedisClient::zscore(const std::string & key, const std::string & member, int32_t& score)
{
	score = 0;
	ScopedReplyPointer reply = exec("ZSCORE %b %b", key.c_str(), static_cast<size_t>(key.size())
		, member.c_str(), static_cast<size_t>(member.size()) );
	if (reply->type == REDIS_REPLY_NIL) {
		return true;
	}
	CHECK_REPLY(reply, REDIS_REPLY_STRING);
	score = atoi(reply->str);
	return true;
}

bool RedisClient::zscore(const std::string & key, const std::string & member, double& score)
{
	score = 0;
	ScopedReplyPointer reply = exec("ZSCORE %b %b", key.c_str(), static_cast<size_t>(key.size())
		, member.c_str(), static_cast<size_t>(member.size()) );
	if (reply->type == REDIS_REPLY_NIL) {
		return true;
	}
	CHECK_REPLY(reply, REDIS_REPLY_STRING);
	score = atof(reply->str);
	return true;
}

bool RedisClient::zrem(const std::string& key, const std::string& field){
	ScopedReplyPointer reply = exec("ZREM  %b %b", key.c_str(), static_cast<size_t>(key.size()),
									field.c_str(), static_cast<size_t>(field.size()));
	CHECK_REPLY(reply, REDIS_REPLY_INTEGER);
	return true;
}

bool RedisClient::zscan(const std::string & key, uint32_t & cursor, std::vector<std::string>& result, uint32_t maxCnt)
{
	ScopedReplyPointer reply = nullptr;
	if (maxCnt != 0) {
		reply = exec("ZSCAN %b %u COUNT %u", key.c_str(), static_cast<size_t>(key.size()), cursor, maxCnt);
	}
	else {
		reply = exec("ZSCAN %b %u", key.c_str(), static_cast<size_t>(key.size()), cursor);
	}
	CHECK_REPLY(reply, REDIS_REPLY_ARRAY);
	if (reply->elements != 2) {
		return false;
	}
	CHECK_REPLY(reply->element[0], REDIS_REPLY_STRING);
	cursor = atoi(reply->element[0]->str);
	auto fReplay = reply->element[1];
	CHECK_REPLY(fReplay, REDIS_REPLY_ARRAY);
	arr_reply_to_vector(fReplay->element, fReplay->elements, result);
	return false;
}

bool RedisClient::zrevrange(const std::string& key, const int32_t from, const int32_t to, std::vector<std::string>& result, bool withscore)
{
	ScopedReplyPointer reply = nullptr;
	if(withscore) {
		reply = exec("ZREVRANGE %b %d %d WITHSCORES", key.c_str(), static_cast<size_t>(key.size()),
				from, to);
	} else {
		reply = exec("ZREVRANGE %b %d %d", key.c_str(), static_cast<size_t>(key.size()),
				from, to);
	}
	CHECK_REPLY(reply, REDIS_REPLY_ARRAY);
	arr_reply_to_vector(reply->element, reply->elements, result);
	return true;
}

bool RedisClient::zcard(const std::string& key, long long* result)
{
	ScopedReplyPointer reply = exec("ZCARD  %b ", key.c_str(), static_cast<size_t>(key.size()));
	CHECK_REPLY(reply, REDIS_REPLY_INTEGER);
	*result = reply->integer;
	return true;
}

bool RedisClient::zrevrank(const std::string& key, const std::string& field,long long* result)
{
	ScopedReplyPointer reply = exec("ZREVRANK %b %b",
			key.c_str(), static_cast<size_t>(key.size()),
			field.c_str(), static_cast<size_t>(field.size()));
	CHECK_REPLY(reply, REDIS_REPLY_INTEGER);
	if(result) {
		*result = reply->integer;
	}
	return true;
}

bool RedisClient::geoadd(const std::string& key, double longitude, double latitude, const std::string& field)
{
	ScopedReplyPointer reply = exec("GEOADD  %b %f %f %b", key.c_str(), static_cast<size_t>(key.size()), longitude, latitude,
									field.c_str(), static_cast<size_t>(field.size()));
	CHECK_REPLY(reply, REDIS_REPLY_INTEGER);
	return true;
}

bool RedisClient::georadius(const std::string& key, double longitude, double latitude, long long dist, std::vector<std::vector<std::string>>& result)
{
	ScopedReplyPointer reply = exec("GEORADIUS  %b %f %f %lld km WITHDIST", key.c_str(), static_cast<size_t>(key.size()), longitude, latitude, dist);
	CHECK_REPLY(reply, REDIS_REPLY_ARRAY);
	result.clear();
	result.reserve(reply->elements);
	for (size_t i = 0; i != reply->elements; ++i) {
		std::vector<std::string> ret;
		arr_reply_to_vector(reply->element[i]->element, reply->element[i]->elements, ret);
		result.emplace_back(ret);
	}
	return true;
}

bool RedisClient::eval_only(const std::string& script, const std::vector<std::string>* keys, const std::vector<std::string>* args)
{
	ScopedReplyPointer reply = eval(script, keys, args);
	if (!reply) {
		return false;
	}

	if (reply->type == REDIS_REPLY_ERROR) {
		set_errmsg(reply->str);
		return false;
	}

	return true;
}

redisReply* RedisClient::eval_imp(const std::string& cmd, const std::string& script
	, const std::vector<std::string>* keys, const std::vector<std::string>* args)
{
	if (!has_context() && !alloc_context()) {
		return nullptr;
	}
	string keysNumStr("0");
	vector<string>::size_type keysNum = 0;
	vector<string>::size_type argsNum = 0;
	if (keys) {
		keysNum = keys->size();
		keysNumStr = to_string(keysNum);
	}
	if (args) {
		argsNum = args->size();
	}
	vector<string>::size_type totalNum = keysNum + argsNum;

	if (keysNum > kVecArgMaxSize || argsNum > kVecArgMaxSize || totalNum > kVecArgMaxSize) {
		set_errmsg("Size of args is invalid (", keysNum, ", ", argsNum, ')');
		return nullptr;
	}

	const_char_ptr* argv = m_tmpArgv;
	size_t* arglens = m_tmpArgLens;
	int argc = 3;
	if (totalNum) {
		argc += totalNum;
	}

	if (argc > kStaticArgvMaxSize) {
		argv = new const_char_ptr[argc];
		arglens = new size_t[argc];
	}

	argv[0] = cmd.c_str();
	arglens[0] = cmd.length();
	argv[1] = script.c_str();
	arglens[1] = script.size();
	argv[2] = keysNumStr.c_str();
	arglens[2] = keysNumStr.size();

	int idx = 3;
	if (keys) {
		for (auto it = keys->begin(); it != keys->end(); ++it) {
			argv[idx] = it->c_str();
			arglens[idx] = it->size();
			++idx;
		}
	}

	if (args) {
		for (auto it = args->begin(); it != args->end(); ++it) {
			argv[idx] = it->c_str();
			arglens[idx] = it->size();
			++idx;
		}
	}
	redisReply* reply = reinterpret_cast<redisReply*>(redisCommandArgv(m_context, argc, argv, arglens));
	if (!reply && realloc_context()) {
		reply = reinterpret_cast<redisReply*>(redisCommandArgv(m_context, argc, argv, arglens));
	}
	if (!reply && m_context) {
		set_errmsg(m_context->errstr, " (", m_context->err, ')');
		free_context();
	}
	free_argv(argv, arglens, argc);
	return reply;
}

redisReply* RedisClient::eval(const std::string& script, const std::vector<std::string>* keys, const std::vector<std::string>* args)
{
	return eval_imp("EVAL", script, keys, args);
}

bool RedisClient::script_load(const std::string & script, std::string & sha)
{
	ScopedReplyPointer reply = exec("SCRIPT LOAD %b", script.c_str(), static_cast<size_t>(script.size()));
	CHECK_REPLY(reply, REDIS_REPLY_STRING);
	sha = reply->str;
	return true;
}

bool RedisClient::script_flush()
{
	ScopedReplyPointer reply = exec("SCRIPT FLUSH");
	CHECK_REPLY(reply, REDIS_REPLY_STRING);
	return strcmp(reply->str, "OK") == 0;
}

redisReply* RedisClient::eval_sha(const std::string & sha, const std::vector<std::string>* keys, const std::vector<std::string>* args)
{
	return eval_imp("EVALSHA", sha, keys, args);
}

//----------------------------------------------------------
// Private Methods
//----------------------------------------------------------
bool RedisClient::alloc_context()
{
	for (const auto& host : m_hosts) {
		m_ip = host.first;
		m_port = host.second;
		m_ip_port = m_ip + ':' + to_string(m_port);

		timeval timeout = { 2, 0 };
		m_context = redisConnectWithTimeout(m_ip.c_str(), m_port, timeout);
		if (m_context && (m_context->err == 0)) {
			if (set_passwd() && select_db(m_dbidx)) {
				redisEnableKeepAlive(m_context);
				return true;
			} else {
				return false;
			}
		}

		if (m_context) {
			set_errmsg("Failed to connect to ", m_ip_port, ": ", m_context->errstr, " (", m_context->err, ')');
			free_context();
		} else {
			set_errmsg("Failed to connect to ", m_ip_port, ": Cannot allocate redisContext");
		}
	}

	return false;
}

redisReply* RedisClient::exec(const char* fmt, ...)
{
	if (!has_context() && !alloc_context()) {
		return 0;
	}

	va_list ap;
	va_start(ap, fmt);
	redisReply* reply = reinterpret_cast<redisReply*>(redisvCommand(m_context, fmt, ap));
	va_end(ap);
	if (!reply && realloc_context()) {
		va_list aq;
		va_start(aq, fmt);
		reply = reinterpret_cast<redisReply*>(redisvCommand(m_context, fmt, aq));
		va_end(aq);
	}

	if (!reply && m_context) {
		set_errmsg(m_context->errstr, " (", m_context->err, ')');
		free_context();
	}
	return reply;
}

void RedisClient::arr_reply_to_vector(redisReply* const replies[], size_t reply_num, vector<string>& result)
{
	result.clear();
	result.reserve(reply_num);
	for (size_t i = 0; i != reply_num; ++i) {
		result.emplace_back(string(replies[i]->str, replies[i]->len));
	}
}
