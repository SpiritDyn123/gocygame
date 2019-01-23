package etc

import (
	"encoding/json"
	"flag"
	"github.com/SpiritDyn123/gocygame/apps/common"
	"github.com/SpiritDyn123/gocygame/libs/utils"
	"io/ioutil"
)


type Cfg_Json_Cluster struct {

	System_ common.Cfg_Json_SvrBase `json:"system"`

	Log_ common.Cfg_Json_Log `json:"log"`
}

var (
	Cluster_Config_File = "../etc/clustersvr.json"
	Cluster_Config Cfg_Json_Cluster
)

var (
	Cmd_log_console bool
)

func initCmd() {
	flag.BoolVar(&Cmd_log_console, "log_console", false, "log console")
	flag.Parse()
}

func init() {
	fdata, err := ioutil.ReadFile(Cluster_Config_File)
	if err != nil {
		panic(err)
	}

	err = json.Unmarshal(fdata, &Cluster_Config)
	if err != nil {
		panic(err)
	}

	//初始化命令行参数
	initCmd()

	//设置log
	utils.Etc_log_console = Cmd_log_console
	utils.Etc_log_file = Cluster_Config.Log_.Log_file_
	utils.Etc_log_rank = Cluster_Config.Log_.Log_lv_
	utils.Etc_log_size = Cluster_Config.Log_.Log_size_
}