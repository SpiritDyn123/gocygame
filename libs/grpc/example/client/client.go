package main

import(
	"flag"
	"google.golang.org/grpc"
	"context"
	hw"github.com/SpiritDyn123/gocygame/libs/grpc/example/pb"
	"strconv"
	"github.com/SpiritDyn123/gocygame/libs/log"
	"time"
	"github.com/SpiritDyn123/gocygame/libs/grpc/defines"
	"sync"
	"os"
	"os/signal"
	"fmt"
	"reflect"
	cgrpc"github.com/SpiritDyn123/gocygame/libs/grpc"
)

var (
	ServerId int
	ServiceName string
	GoNum int
	Hb bool
	Td int

)

func main() {
	flag.IntVar(&ServerId, "sid", 0, "use server id, 0 is roundrotine")
	flag.StringVar(&ServiceName, "service", "hello_service", "service name")
	flag.IntVar(&GoNum, "gonum", 1, "send msg go routine num")
	flag.BoolVar(&Hb, "hb", false, "test hb stream")
	flag.IntVar(&Td, "td", 1000, "time ticker(Millisecond)")
	flag.Parse()

	var wg sync.WaitGroup
	doneCh := make(chan struct{})

	__handler := func(conn *grpc.ClientConn) error {
		wg.Add(GoNum)
		for t := 0; t < GoNum;t++ {
			go func(index int) {
				defer wg.Done()
				b_ctx, cb := context.WithCancel(context.Background())
				tc := time.NewTicker(time.Millisecond * time.Duration(Td))
				if Hb {
					rpcCli := hw.NewRpcTestServiceClient(conn)
					rpcHBCli, err := rpcCli.HB(context.WithValue(b_ctx, defines.CTX_SERVER_ID_KEY, ServerId))
					if err != nil {
						log.Error("rpcCli HB err:%v", err)
						return
					}
					defer rpcHBCli.CloseSend()

					index := 0
					for {
						select {
						case _ = <- doneCh:
							cb()
							return
						case _ = <- tc.C:
							err = rpcHBCli.Send(&hw.RpcHBRequest{})
							if err != nil {
								log.Error("rpcCli send hb err:%v", err)
								return
							}

							log.Release("send hb msg success")

							msg := &hw.RpcTestResponse{}
							err = rpcHBCli.RecvMsg(msg)
							//msg, err := rpcHBCli.Recv()
							if err != nil {
								log.Error("rpcCli recv hb err:%v", err)
								return
							}
							log.Release("recv hb msg:%v context:%+v success", reflect.TypeOf(msg).String(), msg)
							index++
							if index == 2 {
								hresp, err := rpcCli.Hello(b_ctx, &hw.RpcTestRequest{})
								log.Release("hello Resp:%+v, err:%v", hresp, err)
							}
						}
					}
				} else {
					i := 0
					for {
						select {
						case _ = <- doneCh:
							cb()
							return
						case _ = <- tc.C:
							rpcCli := hw.NewRpcTestServiceClient(conn)
							resp, err := rpcCli.Hello(context.WithValue(b_ctx, defines.CTX_SERVER_ID_KEY, ServerId), &hw.RpcTestRequest{Id:int32(i+1), Data: strconv.Itoa(index) + "_name_" + strconv.Itoa(i+1)})
							log.Release("recv resp:%+v, err:%v\n",resp, err)
							i++
						}
					}
				}
			}(t)
		}
		return nil
	}

	cliOpt := &cgrpc.RpcClientOptions{
		ServiceName:ServiceName,
		TargetAddrs:"http://192.168.1.232:2379",
		Handler: __handler,
	}

	rc, err := cgrpc.NewClient(cliOpt)
	if err != nil {
		panic(err)
	}

	err = rc.Start()
	if err != nil {
		panic(err)
	}

	sigCh := make(chan os.Signal)
	signal.Notify(sigCh, os.Interrupt)
	<-sigCh
	close(doneCh)
	wg.Wait()
	err = rc.Stop()
	fmt.Println("rc stop result:%v", err)
	fmt.Println("end")
}