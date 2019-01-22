package main

import (
	"bytes"
	"encoding/binary"
	"fmt"
	"github.com/SpiritDyn123/gocygame/apps/common"
)



func main() {
	h := &common.ProtocolInnerHead{
		Protocol_id_: 1,
		Seq_: 2,
		Uid_lst_: []uint64{2, 3333212},
	}

	data, err := h.Write(binary.LittleEndian)
	if err != nil {
		panic(err)
	}
	fmt.Println(data)

	data = append(data, 1)
	buf := bytes.NewBuffer(data)
	h2 := &common.ProtocolInnerHead{}
	err = h2.Read(buf, binary.LittleEndian)
	if err != nil {
		panic(err)
	}
	fmt.Println(h2, buf.Len())
	fmt.Println("end")
}
