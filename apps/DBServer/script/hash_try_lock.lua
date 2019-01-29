local val = redis.call('incrby', KEYS[1], 1)
local is_lock = 0
if val == 1 then
	is_lock = 1
	redis.call('EXPIRE', KEYS[1], 5)
end
return is_lock
