package etcd

import (
	"google.golang.org/grpc/naming"
	etcd"github.com/coreos/etcd/clientv3"
	"strings"
	"github.com/SpiritDyn123/gocygame/libs/log"
	"context"
	"github.com/coreos/etcd/mvcc/mvccpb"
	"encoding/json"
	"github.com/SpiritDyn123/gocygame/libs/grpc/defines"
)

/*
	首先找到server，在找到service
*/

type watcher struct {
	re *resolver
	etcdCli *etcd.Client
	bInit bool
}

func(w *watcher) Next() ([]*naming.Update, error) {
	keyPrefix := genEtcdRpcKeyPrefix(w.re.serviceName)
	if !w.bInit {
		w.bInit = true
		resp, err := w.etcdCli.Get(context.Background(), keyPrefix, etcd.WithPrefix())
		if err == nil && resp.Kvs != nil && len(resp.Kvs) > 0 {
			updates := make([]*naming.Update, len(resp.Kvs))
			if resp.Kvs != nil {
				for idx, item := range resp.Kvs {
					data := &defines.ServiceValue{}
					ui := &naming.Update{Op:naming.Add}
					err = json.Unmarshal(item.Value, data)
					if err != nil {
						ui.Addr = string(item.Value)
					} else {
						ui.Addr = data.Addr
						ui.Metadata = data
					}

					updates[idx] = ui
				}
			}

			return updates, nil
		}

		log.Release("watch etcd init get err:%v", err)
	}

	rch := w.etcdCli.Watch(context.Background(), keyPrefix, etcd.WithPrefix())
	addr := ""
	for wresp := range rch {
		updates := []*naming.Update{}
		for _, ev := range wresp.Events {
			switch ev.Type {
			case mvccpb.PUT:
				data := &defines.ServiceValue{}
				var mdata interface{} = nil
				err := json.Unmarshal(ev.Kv.Value, data)
				if err != nil {
					addr = string(ev.Kv.Value)
				} else {
					addr = data.Addr
					mdata = data
				}
				updates = append(updates, &naming.Update{Op: naming.Add, Addr: addr, Metadata: mdata})
			case mvccpb.DELETE:
				delKey := string(ev.Kv.Key)
				updates = append(updates, &naming.Update{Op: naming.Delete, Addr: delKey[strings.LastIndex(delKey, "/")+1:]})
			}
		}
		return updates, nil
	}

	return nil, nil
}

func(w *watcher) Close() {

}

type resolver struct {
	serviceName string
}

func (r *resolver) Resolve(ectdAddrs string) (naming.Watcher, error) {
	cli, err := etcd.New(etcd.Config{
		Endpoints:strings.Split(ectdAddrs, ","),
	})

	if err != nil {
		return nil, err
	}

	return &watcher{etcdCli:cli, re:r}, nil
}

func NewResolver(serviceName string) naming.Resolver {
	return &resolver{
		serviceName:serviceName,
	}
}
