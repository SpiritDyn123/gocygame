package main

import (
	"crypto/tls"
	"fmt"
	"github.com/SpiritDyn123/gocygame/libs/chanrpc"
	"github.com/SpiritDyn123/gocygame/libs/log"
	"github.com/SpiritDyn123/gocygame/libs/net/tcp"
	"github.com/gorilla/websocket"
	"net/http"
	"net/url"
	"time"
)

type CusCodec struct {

}

func(cp *CusCodec) Marshal(v interface{}) ([]byte, error) {
	return v.([]interface{})[0].([]byte), nil
}

func(cp *CusCodec) Unmarshal(data []byte) (interface{}, error) {
	return data, nil
}

type CusProtocol struct {

}

func(cp *CusProtocol) NewCodec() tcp.Codec {
	return &CusCodec{}
}

func main() {
	go func() {
		chanS := chanrpc.NewServer(111)
		chanS.Register("recv", func(args []interface{}){
			fmt.Println("new recv:", args)
			args[0].(*tcp.Session).Send([]byte{3,3,3,1,3})
		})
		chanS.Register("accept", func(args []interface{}){
			fmt.Println("accept:", args)
		})
		chanS.Register("closed", func(args []interface{}){
			fmt.Println("closed:", args)
		})

		ser, err := tcp.CreateWSServer("tcp", "127.0.0.1:9966", "crt.pem", "pri.key", &CusProtocol{},
			1024, 1024, time.Second * 3, time.Second *3, 100, chanS, "accept", "recv", "closed")
		if err != nil {
			panic(err)
		}

		ser.Start()
		for {
			select {
			case ci := <- chanS.ChanCall:
				chanS.Exec(ci)
			}
		}
	}()

	url := url.URL{Scheme: "wss", Host: "127.0.0.1:9966", Path: ""}
	d := &websocket.Dialer{
		TLSClientConfig: &tls.Config{
			InsecureSkipVerify: true,
		},
	}
	ws_conn, resp, err := d.Dial(url.String(), http.Header{})
	if err != nil {
		panic(err)
	}
	defer resp.Body.Close()

	err = ws_conn.WriteMessage(websocket.BinaryMessage, []byte{1,3,2,1})
	if err != nil {
		panic(err)
	}

	_, data, err := ws_conn.ReadMessage()
	if err != nil {
		fmt.Println(err)
	} else {
		fmt.Println("client recv:", data)
	}
	ws_conn.Close()
	time.Sleep(time.Second * 3)
	fmt.Println("end")
	log.Close()
}

