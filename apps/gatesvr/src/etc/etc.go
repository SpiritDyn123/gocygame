package etc

import (
	"github.com/SpiritDyn123/gocygame/apps/common"
	"io/ioutil"
	"encoding/json"
	"encoding/binary"
)

var (
	Chan_Server_Len = 10000
	Go_Server_Len = 1000

	Net_Endian = binary.BigEndian
	Net_Head_Len = 4
)

type Cfg_Json_Gate struct {

	System_ common.Cfg_Json_SvrBase `json:"system"`

	Log_ common.Cfg_Json_Log `json:"log"`

	TLS *struct {
		Ws_ bool 				`json:"ws"`
		Cert_file_ string		`json:"cert"`
		Key_file_ string		`json:"key"`
	} `json:"tls"`
}

var (
	Gate_Config_File = "../etc/gatesvr.json"
	Gate_Config Cfg_Json_Gate
)

func init() {
	fdata, err := ioutil.ReadFile(Gate_Config_File)
	if err != nil {
		panic(err)
	}

	err = json.Unmarshal(fdata, &Gate_Config)
	if err != nil {
		panic(err)
	}
}