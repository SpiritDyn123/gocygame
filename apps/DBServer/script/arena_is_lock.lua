local val = redis.call('incrby', KEYS[1], 1)
local is_lock = 0
if val > 1 then
	is_lock = 1
end
if is_lock == 1 then
	redis.call('set', KEYS[1], 1)
else
	redis.call('set', KEYS[1], 0)
end
return is_lock
--return {is_lock, redis.call('get', KEYS[1])}
