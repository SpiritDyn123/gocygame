package etcd

import(
	"fmt"
)
const (
	ETCD_PREFIX = "etcd_lb_key"
)

func genEtcdRpcKeyPrefix(serviceName string) string {
	return fmt.Sprintf("/%s/%s", ETCD_PREFIX, serviceName)
}

func genEtcdRpcKey(addr string, serviceName string) string {
	return fmt.Sprintf("%s/%s", genEtcdRpcKeyPrefix(serviceName), addr)
}
