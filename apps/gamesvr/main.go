package main

import (
	"github.com/SpiritDyn123/gocygame/libs/utils"
	"github.com/SpiritDyn123/gocygame/apps/gamesvr/src"
)

func main() {
	utils.Run(&src.GameSvrGlobal{})
}
