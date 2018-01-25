package main

import (
	"google.golang.org/grpc"
	hw"github.com/SpiritDyn123/gocygame/libs/grpc/example/pb"
	"context"
	"github.com/SpiritDyn123/gocygame/libs/log"
	"flag"
	"fmt"
	"github.com/golang/protobuf/proto"
	cgrpc"github.com/SpiritDyn123/gocygame/libs/grpc"
	"os"
	"os/signal"
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
	i := 0
	for {
		_, err := hbs.Recv()
		if err != nil {
			log.Error("hb recv err:%v", err)
			return err
		}

		log.Release("recv hb msg success")

		var msg proto.Message
		if i%2 == 0 {
			msg = &hw.RpcHBResponse{}
			msg = &hw.RpcTestResponse{100, "ssss"}
			log.Release("send RpcHBResponse")
		} else {
			msg = &hw.RpcTestResponse{100, "ssss"}
			log.Release("send RpcTestResponse")
		}

		err = hbs.SendMsg(msg)
		if err != nil {
			log.Error("hb send err:%v", err)
			return err
		}

		log.Release("response hb msg success")
		i++
	}

	return nil
}


func RunSer() {
	opt := &cgrpc.RpcServerOptions{
		ServiceName:ServiceName,
		Version:"1.0.0",
		EtcdAddr:[]string{"192.168.1.232:2379"},
		Handler:func(gser *grpc.Server) {
			hw.RegisterRpcTestServiceServer(gser, &RpcTestServiceServer{})
		},
	}

	ser, err := cgrpc.NewServer("tmpServer", ServerId,  "127.0.0.1" + Addr, opt)
	if err != nil {
		panic(err)
	}

	if err = ser.Start();err != nil {
		panic(err)
	}
	log.Release("tmpServer id:%d run listen:%s success", ServerId, Addr)

	sig := make(chan os.Signal)
	signal.Notify(sig, os.Interrupt)
	<- sig
	log.Release("closed by interrupt")
	ser.Stop()
}

func main() {
	flag.IntVar(&ServerId, "sid", 1, "server id")
	flag.StringVar(&Addr, "addr", ":8810", "listen port")
	flag.StringVar(&ServiceName, "service", "hello_service", "listen port")
	flag.Parse()

	RunSer()
}
