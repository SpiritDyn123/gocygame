package utils

import (
	"reflect"
	"unsafe"
	"os"
)

func SetVersion(ver string) {
	argv0str := (*reflect.StringHeader)(unsafe.Pointer(&os.Args[0]))
	argv0 := (*[1 << 30]byte)(unsafe.Pointer(argv0str.Data))[:]
	line := os.Args[0]
	for i := 1; i < len(os.Args); i++ {
		line += (" " + os.Args[i])
	}
	line += (" " + ver)
	copy(argv0, line)
	argv0[len(line)] = 0
}
