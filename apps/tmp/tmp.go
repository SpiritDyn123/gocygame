package main

import (
	"fmt"
	"reflect"
)

type IA interface {
	Func()
}

type A struct {

}
func(obj *A) Func() {

}


func fun(obj IA) {
	_, ok1 := obj.(IA)
	_, ok2 := obj.(*A)
	fmt.Println(reflect.TypeOf(obj).String(), ok1, ok2)

	rtype := reflect.TypeOf(obj)
	fmt.Println(reflect.TypeOf(reflect.New(rtype).Elem().Interface()).String())
}

func main() {
	var obj IA
	obj = &A{}
	fun(obj)
	fmt.Println("end")
}
