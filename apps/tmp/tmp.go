package main

import (
	"encoding/json"
	"fmt"
	"github.com/SpiritDyn123/gocygame/apps/tmp/pkg"
)

type A struct {
	A int `json:"a"`
	B string `json:"b"`
}

type F struct {
	pkg.A
	F bool `json:"f"`
}

func main() {
	json_data := []byte(`
	{
		"a":111,
		"b": "sdo",
		"f":true
	}
	`)

	obj := &F{}

	json.Unmarshal(json_data, obj)
	fmt.Println(obj)
}
