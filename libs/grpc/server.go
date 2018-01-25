package grpc

import (
	"google.golang.org/grpc"
	lb"github.com/SpiritDyn123/gocygame/libs/grpc/etcd"
	"net"
	"strings"
)

type RpcServer interface {
	Start() error
	Stop() error
}

type RpcHandler func(server *grpc.Server)

type RpcServerOptions struct {
	ServiceName string
	EtcdAddr[]string
	Version string
	Handler RpcHandler
}

type rpcServer struct {
	serverName string
	serverId   int
	addr string

	opts []*RpcServerOptions
	lbsArr []*lb.EtcdLBServer

	listener net.Listener
	grpcSer *grpc.Server
}

func (ser *rpcServer) Start() error {
	if ser.opts != nil {
		ser.lbsArr = []*lb.EtcdLBServer{}
		for _, opt := range ser.opts {
			lbs := lb.NewLBServer()
			if err := lbs.RegisterService(ser.serverName, ser.serverId, opt.ServiceName, opt.Version, ser.addr, opt.EtcdAddr); err != nil {
				for _, lbs := range ser.lbsArr {
					lbs.UnRegister()
				}
				return err
			}
			ser.lbsArr = append(ser.lbsArr, lbs)
		}

		for _, opt := range ser.opts {
			opt.Handler(ser.grpcSer)
		}
	}

	go ser.grpcSer.Serve(ser.listener)
	return nil
}

func (ser *rpcServer) Stop() error {
	for _, lbs := range ser.lbsArr {
		lbs.UnRegister()
	}

	ser.grpcSer.Stop()
	return nil
}

func NewServer(serverName string, serverId int, addr string, opts ...*RpcServerOptions) (ser RpcServer, err error) {
	ln, err := net.Listen("tcp", addr[strings.LastIndex(addr, ":"):])
	if err != nil {
		return nil, err
	}

	ser = &rpcServer{
		opts:opts,
		addr:addr,
		serverId:serverId,
		serverName:serverName,
		grpcSer:grpc.NewServer(),
		listener:ln,
	}

	return
}
