package main

import (
	"fmt"

	"reflect"
)

type IA interface {
	f()
}

type A struct {

}
func(obj *A) f() {

}

func f1() IA {
	return &A{}
}

func f2() IA {
	return nil
}


func main() {
	//re := &RedisEngine{}

	f := f1()

	fmt.Println(f == nil, reflect.TypeOf(f).String())
	f = f2()
	fmt.Println(f == nil)

	fmt.Println("end")
}
