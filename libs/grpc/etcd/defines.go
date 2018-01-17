package etcd

import(
	"fmt"
)
const (
	ETCD_PREFIX = "etcd_lb_key"
)

type EtcdValue struct {
	Addr string
	ServerName string
	ServerId int
	ServiceName string
}

func genEtcdRpcKeyPrefix(serviceName string) string {
	return fmt.Sprintf("/%s/%s", ETCD_PREFIX, serviceName)
}

func genEtcdRpcKey(serId int, serName, serviceName string) string {
	return fmt.Sprintf("%s/%s_%d", genEtcdRpcKeyPrefix(serviceName), serName, serId)
}
