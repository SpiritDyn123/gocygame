package main

import (
	"fmt"
)


type IA interface {
	Fun1()
}

type IA2 interface {
	Fun2()
}

type Obj struct {

}


func (o *Obj )Fun1() {

}

func (o *Obj )Fun2() {

}

func main() {

	var o IA = &Obj{}

	_, ok := o.(IA2)

	fmt.Println(ok)
	fmt.Println("end")
}
