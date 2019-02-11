package main

import (
	"fmt"

	"reflect"
)

func test(obj interface{}) {
	fmt.Println(reflect.TypeOf(obj))
}
func main() {
	//re := &RedisEngine{}

	test(nil)

	fmt.Println("end")
}
