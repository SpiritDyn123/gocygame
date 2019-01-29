local val = redis.call('incrby', KEYS[1], 1)
local is_lock = 0
if val == 1 then
	is_lock = 1
end
if 0 == is_lock then
	redis.call('set', KEYS[1], 1)
end
return is_lock
