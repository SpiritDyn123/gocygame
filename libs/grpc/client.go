package grpc

import (
	"google.golang.org/grpc"
	"golang.org/x/net/context"
)

type RpcClient interface {
	SendMsg(interface{}) error
}

type RpcClientOptions struct {
	ServerType int
	ServiceName string
	TargetAddrs string
}

type rpcClient struct {
	ctx context.Context
	ctx_cf context.CancelFunc

	opts []*RpcClientOptions
	grpcConns map[string]*grpc.ClientConn
}

func NewClient(opts ...*RpcClientOptions) {
	r := lb.NewResolver(ServiceName)
	b := b.NewServerIdBalancer(r)
	conn, err := grpc.DialContext(context.Background(), "http://192.168.1.232:2379", grpc.WithBlock(), grpc.WithInsecure(), grpc.WithBalancer(b))
	if err != nil {
		panic(err)
	}
}
