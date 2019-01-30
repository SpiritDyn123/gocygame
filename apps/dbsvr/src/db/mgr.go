package db

import (
	"github.com/SpiritDyn123/gocygame/apps/common/net/strategy"
	"github.com/garyburd/redigo/redis"
	"time"
	"github.com/SpiritDyn123/gocygame/apps/dbsvr/src/etc"
	"fmt"
	"github.com/SpiritDyn123/gocygame/apps/dbsvr/src/global"
	"github.com/SpiritDyn123/gocygame/libs/log"
)

var DBMgr global.IDBMgr
func init() {
	DBMgr = &dbMgr{}
}

type dbMgr struct {
	//redis
	strategy_redis_ strategy.Selector

	//mysql
}

func (mgr *dbMgr) Start() (err error) {
	//初始化redis
	mgr.strategy_redis_ = strategy.CreateSelector(strategy.Cons_hash, 0)
	for _, cfg := range  etc.DB_Config.Redis_cfgs_ {
		redis_pool := &redis.Pool{
			Dial:  func() (redis.Conn, error){
				dial_opts := []redis.DialOption{
					redis.DialConnectTimeout(time.Second * 3),
				}
				if cfg.Pwd_ != "" {
					dial_opts = append(dial_opts, redis.DialPassword(cfg.Pwd_))
				}
				conn, err_conn := redis.Dial("tcp", cfg.Addr_, dial_opts...)
				if err_conn != nil {
					return nil, err_conn
				}
				return conn, err_conn
			},
			MaxIdle: cfg.Max_idle_,
			MaxActive: cfg.Max_active_,
			IdleTimeout:time.Minute,
		}

		mgr.strategy_redis_.AddElement(fmt.Sprintf("%s#%d", cfg.Addr_, cfg.Id_), redis_pool)
		log.Release("dbMgr::Start redis cfg:%+v", cfg)
	}

	return
}

func (mgr *dbMgr) Stop() {

}

func (mgr *dbMgr) GetRedisPool(key interface{}) *redis.Pool {
	mgr.strategy_redis_.SetElementId(key)
	obj := mgr.strategy_redis_.Select()
	if obj == nil {
		return nil
	}
	return obj.(*redis.Pool)
}
