package operation

import (
	"github.com/golang/protobuf/proto"
	"github.com/SpiritDyn123/gocygame/apps/common/proto"
	"github.com/SpiritDyn123/gocygame/libs/log"
	"github.com/SpiritDyn123/gocygame/apps/dbsvr/src/global"
	"github.com/garyburd/redigo/redis"
)

func opr_test_recv(in_msg proto.Message) (proto.Message, error) {
	req_msg := in_msg.(*ProtoMsg.PbSvrDBTestRecvReqMsg)
	log.Release("opr_test_recv recv data:%+v", req_msg)

	//resp = true
	redis_pool := global.DBSvrGlobal.GetDBMgr().GetRedisPool(req_msg.Id)
	if redis_pool == nil {
		log.Release("opr_test_recv get redis empty")
		return nil, nil
	}
	redis_conn := redis_pool.Get()
	defer redis_conn.Close()

	d, err := redis.String(redis_conn.Do("Get", "spirit"))
	log.Release("opr_test_recv redis data:%+v %v", d, err)
	return nil, nil
}