package main

import (
	"fmt"
	"encoding/json"
	"github.com/garyburd/redigo/redis"
)

func Do(cmd string, args ...interface{}) {
	data, err := json.Marshal(args)
	if err != nil {
		panic(err)
	}

	fmt.Println(string(data))
}
func main() {


	js_data := `[1,{"key":"v"},{"key2":1212}, "sdsd"]`
	data := []interface{}{}
	err := json.Unmarshal([]byte(js_data), &data)
	fmt.Println(err , data)
	conn, err := redis.Dial("tcp", "10.216.251.98:6379", redis.DialPassword("1234560."))
	if err != nil {
		panic(err)
	}
	defer conn.Close()

	rep ,err := conn.Do("hgetall", "sb_hash")
	rep_js, err := json.Marshal(rep)
	if err != nil {
		panic(err)
	}

	var ret interface{}
	err = json.Unmarshal(rep_js, ret)
	if err != nil {
		panic(err)
	}

	fmt.Println(string(rep_js))

	type A struct {
		K1 string `redis:"k1"`
		K2 int `redis:"k2"`
	}
	obj := &A{}
	err = redis.ScanStruct(rep.([]interface{}), obj)

	fmt.Println(rep, obj)
	//Do("shabi", 1, 2, map[string]int{"a":1}, []byte{1, 22, 3}, "sdsd")


	fmt.Println("end")
}
