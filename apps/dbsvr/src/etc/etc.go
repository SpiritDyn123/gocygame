package etc

import (
	"encoding/json"
	"flag"
	"github.com/SpiritDyn123/gocygame/apps/common"
	"github.com/SpiritDyn123/gocygame/libs/utils"
	"io/ioutil"
)


type Cfg_Json_DB struct {

	System_ common.Cfg_Json_SvrBase `json:"system"`

	Log_ common.Cfg_Json_Log `json:"log"`

	Cluster_ common.Cfg_Json_Svr_Item `json:"cluster"`

	Redis_cfgs_ []*common.Cfg_Json_Redis_Item `json:"redis"`
}

var (
	DB_Config_File = "../etc/dbsvr.json"
	DB_Config Cfg_Json_DB

)

var (
	Cmd_log_console bool
)

func initCmd() {
	flag.BoolVar(&Cmd_log_console, "log_console", false, "log console")
	flag.Parse()
}

func init() {
	fdata, err := ioutil.ReadFile(DB_Config_File)
	if err != nil {
		panic(err)
	}

	err = json.Unmarshal(fdata, &DB_Config)
	if err != nil {
		panic(err)
	}

	//初始化命令行参数
	initCmd()

	//设置log
	utils.Etc_log_console = Cmd_log_console
	utils.Etc_log_file = DB_Config.Log_.Log_file_
	utils.Etc_log_rank = DB_Config.Log_.Log_lv_
	utils.Etc_log_size = DB_Config.Log_.Log_size_
}