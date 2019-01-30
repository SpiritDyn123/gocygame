package global

import "github.com/garyburd/redigo/redis"

type IDBMgr interface {
	Start() error
	Stop()
	GetRedisPool(key interface{}) *redis.Pool
}