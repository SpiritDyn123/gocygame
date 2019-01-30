package common

//json配置的log相关
type Cfg_Json_Log struct {
	Log_file_ string 		`json:"log_file"`
	Log_lv_  string 		`json:"log_level"`
	Log_size_  int64 		`json:"log_size"`
}

type Cfg_Json_TLS struct {
	Ws_ bool 				`json:"ws"`
	Cert_file_ string		`json:"cert_file"`
	Key_file_ string		`json:"key_file"`
}

type Cfg_Json_Svr_Item struct {
	Id_  int     `json:"id"`
	Addr_ string `json:"addr"`
	Timeout_ int `json:"timeout"`
	Ttl_      int    `json:"ttl"`
}

type Cfg_Json_Redis_Item struct {
	Id_ 			int `json:"id"`
	Addr_ 			string `json:"addr"`
	User_ 			string  `json:"user"`
	Pwd_  			string  `json:"pwd"`
	Max_idle_ 		int `json:"max_idle"`
	Max_active_ 	int  `json:"max_active"`
}

type Cfg_Json_SvrBase struct {
	//基础配置
	Svr_group_id_ int			`json:"group_id"`
	Svr_id_  int				`json:"id"`
	Svr_name_ string			`json:"name"`

	//网络相关
	Svr_addr_  string			`json:"addr"`
	Svr_ttl_   int				`json:"ttl"`
	Svr_timeout_  int			`json:"timeout"`
	Svr_max_conn_  int			`json:"max_conn"`
	Svr_in_bytes_  int64		`json:"in_bytes"`
	Svr_out_bytes_	int64		`json:"out_bytes"`

	//其他
	Svr_alarm_url_  string		`json:"alarm_url"`

}