package log

import (
	"errors"
	"fmt"
	"log"
	"os"
	"strings"
	"github.com/ivpusic/grpool"
	"runtime"
	lfile"github.com/SpiritDyn123/gocygame/libs/log/file"
)

// levels
const (
	debugLevel   = 0
	releaseLevel = 1
	errorLevel   = 2
	fatalLevel   = 3
)

const (
	printDebugLevel   = "[debug] "
	printReleaseLevel = "[release] "
	printErrorLevel   = "[error] "
	printFatalLevel   = "[fatal] "
)

var (
	Invalid_depath = 3
	Use_Pool = false
)

type Logger struct {
	level      int
	baseLogger *log.Logger
	consoleLogger *log.Logger
	baseWriter   LogWriter
	pool *grpool.Pool
	flag int
}

func New(pathname string, maxSize int, strLevel string, flag int) (*Logger, error) {
	// level
	var level int
	switch strings.ToLower(strLevel) {
	case "debug":
		level = debugLevel
	case "release":
		level = releaseLevel
	case "error":
		level = errorLevel
	case "fatal":
		level = fatalLevel
	default:
		return nil, errors.New("unknown level: " + strLevel)
	}

	curFlag := 0
	if Use_Pool {
		Invalid_depath = 3
		//用为采用协程打断了log的文件runtime.Caller
		if flag&(log.Lshortfile|log.Llongfile) != 0 {
			curFlag = flag
			flag &= ^log.Lshortfile
			flag &= ^log.Llongfile
		}
	}

	// logger
	var baseLogger *log.Logger
	var consoleLogger *log.Logger
	var baseWriter LogWriter
	if pathname != "" {
		file := &lfile.Writer{
			FileName:pathname,
			MaxSize:int64(maxSize),
		}

		baseLogger = log.New(file, "", flag)
		baseWriter = file

		//debug日志输出到屏幕
		if level <= debugLevel {
			consoleLogger = log.New(os.Stdout, "", flag)
		}
	} else {
		consoleLogger = log.New(os.Stdout, "", flag)
	}

	// new
	logger := new(Logger)
	logger.level = level
	logger.baseLogger = baseLogger
	logger.consoleLogger = consoleLogger
	logger.baseWriter = baseWriter
	logger.flag = curFlag

	if Use_Pool {
		logger.pool = grpool.NewPool(1, 1000) //单线程写文件
	}

	return logger, nil
}

// It's dangerous to call the method on logging
func (logger *Logger) Close() {
	if logger.baseWriter != nil {
		logger.baseWriter.Close()
	}

	logger.baseLogger = nil
	logger.consoleLogger = nil
	logger.baseWriter = nil
	if Use_Pool {
		logger.pool.Release()
	}
}

func (logger *Logger) doPrintf(level int, printLevel string, format string, a ...interface{}) {
	if level < logger.level {
		return
	}

	logClosed := true
	header := ""
	if Use_Pool {
		if logger.flag & (log.Lshortfile|log.Llongfile) != 0 {
			_, file, line, ok := runtime.Caller(2)
			if ok {
				if logger.flag & log.Lshortfile != 0 {
					short := file
					for i := len(file) - 1; i > 0; i-- {
						if file[i] == '/' {
							short = file[i+1:]
							break
						}
					}
					file = short
				}
			} else {
				file = "???"
				line = 0
			}

			header = fmt.Sprintf("%s:%d ", file, line)
		}
	}

	format = header + printLevel + format
	if logger.baseLogger != nil {
		logClosed = false
		if Use_Pool {
			logger.pool.JobQueue <- func() {
				logger.baseLogger.Output(Invalid_depath, fmt.Sprintf(format, a...))
			}
		} else {
			logger.baseLogger.Output(Invalid_depath, fmt.Sprintf(format, a...))
		}

	}

	if logger.consoleLogger != nil {
		logClosed = false
		if Use_Pool {
			logger.pool.JobQueue <- func() {
				logger.consoleLogger.Output(Invalid_depath, fmt.Sprintf(format, a...))
			}
		} else {
			logger.consoleLogger.Output(Invalid_depath, fmt.Sprintf(format, a...))
		}
	}

	if logClosed {
		panic("logger closed")
	}

	if level == fatalLevel {
		os.Exit(1) //如果使用pool，很可能会导致日志没写进去就exit
	}
}

func (logger *Logger) Debug(format string, a ...interface{}) {
	logger.doPrintf(debugLevel, printDebugLevel, format, a...)
}

func (logger *Logger) Release(format string, a ...interface{}) {
	logger.doPrintf(releaseLevel, printReleaseLevel, format, a...)
}

func (logger *Logger) Error(format string, a ...interface{}) {
	logger.doPrintf(errorLevel, printErrorLevel, format, a...)
}

func (logger *Logger) Fatal(format string, a ...interface{}) {
	logger.doPrintf(fatalLevel, printFatalLevel, format, a...)
}

func (logger *Logger) SetLevel(strLevel string) error {
	var level int
	switch strings.ToLower(strLevel) {
	case "debug":
		level = debugLevel
	case "release":
		level = releaseLevel
	case "error":
		level = errorLevel
	case "fatal":
		level = fatalLevel
	default:
		return errors.New("unknown level: " + strLevel)
	}

	logger.level = level
	return nil
}

var gLogger, _ = New("", 0,"debug",  log.LstdFlags|log.Lshortfile)

// It's dangerous to call the method on logging
func Export(logger *Logger) {
	if logger != nil {
		gLogger = logger
	}
}

func Debug(format string, a ...interface{}) {
	gLogger.doPrintf(debugLevel, printDebugLevel, format, a...)
}

func Release(format string, a ...interface{}) {
	gLogger.doPrintf(releaseLevel, printReleaseLevel, format, a...)
}

func Error(format string, a ...interface{}) {
	gLogger.doPrintf(errorLevel, printErrorLevel, format, a...)
}

func Fatal(format string, a ...interface{}) {
	gLogger.doPrintf(fatalLevel, printFatalLevel, format, a...)
}

func Close() {
	gLogger.Close()
}

func SetLevel(strLevel string) error {
	if gLogger == nil {
		return errors.New("gLogger nil")
	}

	return gLogger.SetLevel(strLevel)
}
