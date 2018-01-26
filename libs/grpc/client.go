package grpc

import (
	"google.golang.org/grpc"
	"golang.org/x/net/context"
	sb"github.com/SpiritDyn123/gocygame/libs/grpc/balancer"
	"fmt"
	lb"github.com/SpiritDyn123/gocygame/libs/grpc/etcd"
)

type RpcClient interface {
	Start() error
	Stop() error
}

type RpcClientHandler func(cc *grpc.ClientConn) error

type RpcClientOptions struct {
	ServiceName string
	TargetAddrs string
	Handler RpcClientHandler
}

type rpcCliInfo struct {
	opt *RpcClientOptions
	gconn *grpc.ClientConn
}

type rpcClient struct {
	opts []*RpcClientOptions
	grpcConns map[string]*rpcCliInfo
}

func (cli *rpcClient) Start() error {
	for _, opt := range cli.opts {
		if _, ok := cli.grpcConns[opt.ServiceName]; ok {
			return fmt.Errorf("NewRpcClient service:%s repeated", opt.ServiceName)
		}

		r := lb.NewResolver(opt.ServiceName)
		b := sb.NewServerIdBalancer(r)
		conn, err := grpc.DialContext(context.Background(), opt.TargetAddrs, grpc.WithBlock(), grpc.WithInsecure(), grpc.WithBalancer(b))
		if err != nil {
			return err
		}

		err = opt.Handler(conn)
		if err != nil {
			return err
		}

		cli.grpcConns[opt.ServiceName] = &rpcCliInfo{
			opt:opt,
			gconn:conn,
		}
	}

	return nil
}

func (cli *rpcClient) Stop() error {
	var err error
	for _, ri := range cli.grpcConns {
		err2 := ri.gconn.Close()
		if err2 != nil {
			if err == nil {
				err = fmt.Errorf("serviceName:%s err:%v;", ri.opt.ServiceName, err2)
			} else {
				err = fmt.Errorf("%sserviceName:%s err:%v;", err.Error(), ri.opt.ServiceName, err2)
			}

		}
	}

	return err
}

func NewClient(opts ...*RpcClientOptions) (RpcClient, error){
	rc := &rpcClient{
		opts:opts,
		grpcConns:make(map[string]*rpcCliInfo),
	}

	return rc, nil
}
