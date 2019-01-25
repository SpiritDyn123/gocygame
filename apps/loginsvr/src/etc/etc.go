package etc

import (
	"encoding/json"
	"flag"
	"github.com/SpiritDyn123/gocygame/apps/common"
	"github.com/SpiritDyn123/gocygame/libs/utils"
	"io/ioutil"
)


type Cfg_Json_Login struct {

	System_ common.Cfg_Json_SvrBase `json:"system"`

	Log_ common.Cfg_Json_Log `json:"log"`

	Cluster_ common.Cfg_Json_Svr_Item `json:"cluster"`
}

var (
	Login_Config_File = "../etc/loginsvr.json"
	Login_Config Cfg_Json_Login
)

var (
	Cmd_log_console bool
)

func initCmd() {
	flag.BoolVar(&Cmd_log_console, "log_console", false, "log console")
	flag.Parse()
}

func init() {
	fdata, err := ioutil.ReadFile(Login_Config_File)
	if err != nil {
		panic(err)
	}

	err = json.Unmarshal(fdata, &Login_Config)
	if err != nil {
		panic(err)
	}

	//初始化命令行参数
	initCmd()

	//设置log
	utils.Etc_log_console = Cmd_log_console
	utils.Etc_log_file = Login_Config.Log_.Log_file_
	utils.Etc_log_rank = Login_Config.Log_.Log_lv_
	utils.Etc_log_size = Login_Config.Log_.Log_size_
}