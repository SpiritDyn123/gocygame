package player

import common_global"github.com/SpiritDyn123/gocygame/apps/common/global"

type player struct {
	session_ 		common_global.ILogicSession
	game_svr_id_  	int32
	uid_ 			uint64
	b_register_  	bool //首次注册

}