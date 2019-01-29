local regionId = tonumber(redis.call('hget', KEYS[9], ARGV[1]) or 1)
local score = tonumber(redis.call( 'zscore', KEYS[regionId], ARGV[1]) or 0)
return score