--行为排行榜分数递增
local inc = ARGV[1]
local tid = ARGV[2]
local now = ARGV[3]
local val = redis.call('zscore', KEYS[1], tid) or 0
val = val + inc
local score = '' .. math.floor(val) .. '.' .. (2000000000 - now)
redis.call('zadd', KEYS[1], score, tid)
return 0

