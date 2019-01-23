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
	m_data := make(map[int]map[int]int)
	m_data[1] = make(map[int]int)
	a := m_data[1]
	a[1] = 1
	fmt.Println(m_data)

	fmt.Println("end")
}
