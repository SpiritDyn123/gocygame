--key:1~8 regin_key; 9:info_key
--arg:1 userId; 2:score; 3:now; 4:overwrite;

local rid = redis.call('hget', KEYS[9], ARGV[1])
local regionId = tonumber(rid) or 1
local overwrite = tonumber(ARGV[4] or 0) > 0
local score = 0
if overwrite then
	score = ARGV[2]
else
	score = redis.call( 'zincrby', KEYS[regionId], ARGV[2], ARGV[1])
end

score = tonumber(score) or 0

if score < 0 then
	score = 0
end

local region = {
	200,
	500,
	800,
	1200,
	1600,
	2000,
	2500,
	3000,
}

local realRegionId = #region

local add = 0

if not overwrite and tonumber(ARGV[2]) < 0 then
	add = -29 + 1
end

for i,v in ipairs(region) do
	local offset = 0
	if i < regionId then
		offset = add
	end
	if score < v + offset then
		realRegionId = i
		break
	end
end

score = '' .. math.floor(score) .. '.' .. ((2000000000 - math.floor(ARGV[3])) % 1000000)

if realRegionId ~= tonumber(rid) then
	redis.call('zrem', KEYS[regionId], ARGV[1])
	redis.call('zadd', KEYS[realRegionId], score, ARGV[1])
	redis.call('hset', KEYS[9], ARGV[1], realRegionId)
else
	redis.call('zadd', KEYS[realRegionId], score, ARGV[1])
end
return 0
--return redis.call('zscore', KEYS[realRegionId], ARGV[1])
