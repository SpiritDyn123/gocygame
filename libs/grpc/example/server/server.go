package main

import (
	"google.golang.org/grpc"
	"net"
	lb"github.com/SpiritDyn123/gocygame/libs/grpc/etcd"
	hw"github.com/SpiritDyn123/gocygame/libs/grpc/example/pb"
	"context"
	"github.com/SpiritDyn123/gocygame/libs/log"
	"flag"
	"fmt"
)

type RpcTestServiceServer struct {

}

var (
	ServerId int
	Addr string
	ServiceName string
)

func (rs *RpcTestServiceServer) Hello(ctx context.Context, reqMsg *hw.RpcTestRequest) (*hw.RpcTestResponse, error) {
	log.Release("tmpServer %d recvMsg:%+v", ServerId, reqMsg)

	return &hw.RpcTestResponse{Code:100, Message:"response from tmpServer" + fmt.Sprintf("_%d", ServerId)}, nil
}

func (rs *RpcTestServiceServer) HB(hbs hw.RpcTestService_HBServer) error {
	for {
		_, err := hbs.Recv()
		if err != nil {
			log.Error("hb recv err:%v", err)
			return err
		}

		log.Release("recv hb msg success")

		err = hbs.Send(&hw.RpcHBResponse{})
		if err != nil {
			log.Error("hb send err:%v", err)
			return err
		}

		log.Release("response hb msg success")
	}

	return nil
}


func RunSer() {
	ser := grpc.NewServer()
	ls := lb.NewLBServer()
	ls.RegisterService("tmpServer", ServerId, ServiceName, "127.0.0.1" + Addr, []string{"192.168.1.232:2379"})
	ln, err := net.Listen("tcp", Addr)
	hw.RegisterRpcTestServiceServer(ser, &RpcTestServiceServer{})
	if err != nil {
		panic(err)
	}
	log.Release("tmpServer id:%d run listen:%s success", ServerId, Addr)
	ser.Serve(ln)

}

func main() {
	flag.IntVar(&ServerId, "sid", 1, "server id")
	flag.StringVar(&Addr, "addr", ":8810", "listen port")
	flag.StringVar(&ServiceName, "service", "hello_service", "listen port")
	flag.Parse()

	RunSer()
}
