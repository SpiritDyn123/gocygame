package common

import "encoding/binary"

const (
	RECV_KEY = "TCP_RECV"
	CLOSED_KEY = "TCP_CLOSED"
	ACCEPT_KEY = "TCP_ACCEPT"
)

const (
	TCP_ADDR = ":9966"
	MAX_SEND = 65535
	MAX_RECV = 65535
	MSGLEN = 4
	SEND_CHAN_SIZE = 100
)

var BytesOrder binary.ByteOrder = binary.BigEndian