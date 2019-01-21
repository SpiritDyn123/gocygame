package main

import (
	"encoding/binary"
	"fmt"
	"github.com/SpiritDyn123/gocygame/libs/chanrpc"
	"github.com/SpiritDyn123/gocygame/libs/log"
	"github.com/SpiritDyn123/gocygame/libs/net/tcp"
	"net"
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
	var endian binary.ByteOrder = binary.BigEndian
	go func() {
		chanS := chanrpc.NewServer(111)
		chanS.Register("recv", func(args []interface{}){
			fmt.Println("new recv:", args)
			//args[0].(*tcp.Session).Send([]byte{3,3,3,1,3})
		})
		chanS.Register("accept", func(args []interface{}){
			fmt.Println("accept:", args)
		})
		chanS.Register("closed", func(args []interface{}){
			fmt.Println("closed:", args)
		})

		msgP := tcp.NewMsgParser()
		msgP.SetByteOrder(endian == binary.LittleEndian)
		msgP.SetMsgLen(4, 22222, 2222)
		ser, err := tcp.CreateTcpServer("tcp", "127.0.0.1:9966", &CusProtocol{}, msgP,
		100, chanS, "accept", "recv", "closed")
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

	conn, err := net.Dial("tcp", "127.0.0.1:9966")
	if err != nil {
		panic(err)
	}

	body_data := []byte{1,2,3,4}
	len_data := make([]byte, 4)
	endian.PutUint32(len_data, uint32(len(body_data)))

	_, err = conn.Write(append(len_data, body_data...))
	if err != nil {
		fmt.Println(err)
	}

	conn.Close()
	time.Sleep(time.Second * 3)
	fmt.Println("end")
	log.Close()
}

