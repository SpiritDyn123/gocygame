package etcd

import(
	etcd"github.com/coreos/etcd/clientv3"
	"encoding/json"
	"context"
	"libs/leaf/log"
	"sync"
	"time"
	"github.com/SpiritDyn123/gocygame/libs/grpc/defines"
)

const (
	LEASE_TTL_SECOND = 10
)

type EtcdLBServer struct {
	cancal context.Context
	cf context.CancelFunc
	wg sync.WaitGroup

	registered bool
}

func (lb *EtcdLBServer) RegisterService(serverName string, serverId int, serviceName string,  version string, addr string, etcdAdrrs []string) error {
	value := &defines.ServiceValue{
		Addr:addr,
		ServerName:serverName,
		ServerId:serverId,
		ServiceName:serviceName,
		Version:version,
	}
	data, err := json.Marshal(value)
	if err != nil {
		return err
	}
	valueStr := string(data)

	etcdCli, err := etcd.New(etcd.Config{
		Endpoints:etcdAdrrs,
	})

	if err != nil {
		return err
	}

	lb.cancal, lb.cf = context.WithCancel(context.Background())
	lb.wg.Add(1)
	go func() {
		defer lb.wg.Done()
		key := genEtcdRpcKey(addr, serviceName)
		for {
			lease, err := etcdCli.Grant(context.TODO(), LEASE_TTL_SECOND)
			if err != nil {
				log.Error("EtcdlbServer Grant err:%v", err)
				return
			}

			_, err = etcdCli.Put(lb.cancal, key, valueStr, etcd.WithLease(lease.ID))
			if err != nil {
				log.Error("EtcdlbServer Put err:%v", err)
				return
			}

			t := time.Second *(LEASE_TTL_SECOND - 1)
			if t == 0 {
				t = time.Millisecond
			}
			tc := time.After(t)
			select {
				case <- tc:
				case <- lb.cancal.Done():
					//删除
					log.Release("EtcdlbServer cancaled reason:", lb.cancal.Err())
					_, err := etcdCli.Delete(lb.cancal, key)
					if err != nil {
						log.Error("EtcdlbServer Delete err:%v", err)
					}

					return
			}
		}
	}()

	lb.registered = true
	return nil
}

func (lb *EtcdLBServer) UnRegister() {
	if !lb.registered {
		return
	}

	lb.registered = false

	lb.cf()
	lb.wg.Wait()
}


func NewLBServer() *EtcdLBServer {
	return &EtcdLBServer{}
}
