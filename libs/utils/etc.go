package utils

import (
	syslog "log"
	"github.com/SpiritDyn123/gocygame/libs/log"
)

//日志相关
var (
	Etc_log_file string //日志文件路径和名字
	Etc_log_rank string = "debug"  //日志等级
	Etc_log_size int64 = 1024 * 1024 * 512 //日志大小 默认512M
	Etc_log_flag = syslog.LstdFlags | syslog.Lshortfile
	Etc_log_console = false

	Etc_log_writer log.LogWriter //这个可以设置
)

//
var (

)