package log

import (
	"errors"
	"fmt"
	"log"
	"os"
	"strings"
	"runtime"
	"sync"
	"encoding/json"
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

const (
	Logger_Go_Num = 1
)

type LogSon map[string]interface{}

type loggerItem struct {
	line string
	level int
}

type Logger struct {
	level      int
	baseLogger *log.Logger
	consoleLogger *log.Logger
	baseWriter   LogWriter
	flag int

	wg sync.WaitGroup
	logChan chan *loggerItem
	closed bool
	lock sync.RWMutex
}

func New(writer LogWriter, strLevel string, flag int, console bool) (*Logger, error) {
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

	//用为采用协程打断了log的文件runtime.Caller
	curFlag := flag
	if flag & (log.Lshortfile | log.Llongfile) != 0 {
		flag &= ^log.Lshortfile
		flag &= ^log.Llongfile
	}

	// new
	logger := new(Logger)
	logger.logChan = make(chan *loggerItem, 500)
	logger.level = level

	if writer != nil {
		logger.baseLogger = log.New(writer, "", flag)
		logger.baseWriter = writer
	}

	logger.flag = curFlag
	if console {
		logger.consoleLogger = log.New(os.Stdout, "", flag)
	}

	logger.wg.Add(Logger_Go_Num)
	go logger.goLogWrite()

	return logger, nil
}

// It's dangerous to call the method on logging
func (logger *Logger) Close() {
	logger.lock.Lock()
	defer logger.lock.Unlock()
	logger.closed = true

	close(logger.logChan)
	logger.wg.Wait()
	if logger.baseWriter != nil {
		logger.baseWriter.Close()
	}

	logger.baseLogger = nil
	logger.consoleLogger = nil
	logger.baseWriter = nil
}

func (logger *Logger) goLogWrite() {
	defer logger.wg.Done()

	for {
		select{
		case logLine, ok := <- logger.logChan:
			if !ok {
				return
			}

			if logger.baseLogger != nil {
				logger.baseLogger.Output(3, logLine.line)
			}

			if logger.consoleLogger != nil {
				logger.consoleLogger.Output(3, logLine.line)
			}

			if logLine.level == fatalLevel {
				os.Exit(1) //如果使用pool，很可能会导致日志没写进去就exit
			}
		}
	}
}

func (logger *Logger) doPrintf(level int, printLevel string, format interface{}, a ...interface{}) {
	if level < logger.level {
		return
	}

	header := ""
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

	whole_fmt := header + printLevel
	switch format.(type) {
	case string:
		whole_fmt +=  format.(string)
		whole_fmt = fmt.Sprintf(whole_fmt, a...)
	case LogSon:
		son_fmt, _ := json.Marshal(format)
		whole_fmt += string(son_fmt)
	}

	logger.lock.RLock()
	if !logger.closed {
		logger.logChan <- &loggerItem{whole_fmt, level}
	}
	logger.lock.RUnlock()
}

func (logger *Logger) Debug(format interface{}, a ...interface{}) {
	logger.doPrintf(debugLevel, printDebugLevel, format, a...)
}

func (logger *Logger) Release(format interface{}, a ...interface{}) {
	logger.doPrintf(releaseLevel, printReleaseLevel, format, a...)
}

func (logger *Logger) Error(format interface{}, a ...interface{}) {
	logger.doPrintf(errorLevel, printErrorLevel, format, a...)
}

func (logger *Logger) Fatal(format interface{}, a ...interface{}) {
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

var gLogger, _ = New(nil,"debug",  log.LstdFlags|log.Lshortfile, true)

// It's dangerous to call the method on logging
func Export(logger *Logger) {
	if logger != nil {
		gLogger = logger
	}
}

func Debug(format interface{}, a ...interface{}) {
	gLogger.doPrintf(debugLevel, printDebugLevel, format, a...)
}

func Release(format interface{}, a ...interface{}) {
	gLogger.doPrintf(releaseLevel, printReleaseLevel, format, a...)
}

func Error(format interface{}, a ...interface{}) {
	gLogger.doPrintf(errorLevel, printErrorLevel, format, a...)
}

func Fatal(format interface{}, a ...interface{}) {
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
