package main

import (
	"github.com/SpiritDyn123/gocygame/libs/utils"
	"github.com/SpiritDyn123/gocygame/apps/loginsvr/src"
)

func main() {
	utils.Run(&src.LoginSvrGlobal{})
}
