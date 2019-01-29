-- arface_mon_dailytask.lua KEYS1  monid  到期时间
local t = tonumber(redis.call('hget', KEYS[1], "expireflag") or 0)
local result = 0

if (t == 0)
then
	redis.call('hset', KEYS[1], "expireflag", 1)
	redis.call('expire', KEYS[1], ARGV[2])
end

t = tonumber(redis.call('hget',  KEYS[1], ARGV[1]) or 0)

if (t > tonumber(ARGV[3]))
then
	result = 0
else
	result = 1
end

return result
