package file

/*
	超过制定大小或者跨天自动回滚
*/
import (
	"os"
	"fmt"
	"sync"
	"path/filepath"
	"time"
)

const (
	DEFAULT_MAX_SIZE = 1024 * 1024 * 1024 //默认文件回滚大小1G
)

type fileInfo struct {
	file *os.File
	createTime time.Time
	curIndex int
	curSize int64
}

type Writer struct {
	MaxSize int64
	FileName string

	lastDay string
	curFileIndex int

	file *fileInfo

	mu sync.Mutex
}

func (l *Writer) Write(p []byte) (n int, err error) {
	l.mu.Lock()
	defer l.mu.Unlock()

	if l.MaxSize == 0 {
		l.MaxSize = DEFAULT_MAX_SIZE
	}

	writeLen := int64(len(p))
	if writeLen > l.MaxSize {
		return 0, fmt.Errorf(
			"write length %d exceeds maximum file size %d", writeLen, l.MaxSize,
		)
	}

	if l.file == nil { //第一次进来
		if err = l.openExistingOrNew(len(p)); err != nil {
			return 0, err
		}
	} else {
		now := time.Now()
		curDay := now.Format("20060102")
		if l.lastDay != curDay { //检查是否跨天
			l.lastDay = curDay
			l.curFileIndex = 0
			if err := l.rotate(); err != nil {
				return 0, err
			}

		} else if l.file.curSize+writeLen > l.MaxSize { 	//检查是否超过文件大小
			if err := l.rotate(); err != nil {
				return 0, err
			}
		}
	}

	n, err = l.file.file.Write(p)
	l.file.curSize += int64(n)

	return n, err
}

func (l *Writer) Close() error {
	l.mu.Lock()
	defer l.mu.Unlock()
	return l.close()
}

func (l *Writer) getCurFileIndex() int {
	if l.curFileIndex == 0 {
		l.curFileIndex = 1
		for {
			name := fmt.Sprintf("%s.%s.%d", l.FileName, l.lastDay, l.curFileIndex)
			_, err := os.Stat(name)
			if os.IsNotExist(err) {
				break
			}
			l.curFileIndex++
		}
	}

	return l.curFileIndex
}

func (l *Writer) openNew() error {
	err := os.MkdirAll(l.dir(), 0744)
	if err != nil {
		return fmt.Errorf("can't make directories for new logfile: %s", err)
	}

	name := l.FileName
	mode := os.FileMode(0644)
	info, err := os.Stat(name)
	if err == nil {
		// Copy the mode off the old logfile.
		mode = info.Mode()
		// move the existing file
		newname := fmt.Sprintf("%s.%s.%d", l.FileName, l.file.createTime.Format("20060102"), l.file.curIndex)
		if err := os.Rename(name, newname); err != nil {
			return fmt.Errorf("can't rename log file: %s", err)
		}
	}

	// we use truncate here because this should only get called when we've moved
	// the file ourselves. if someone else creates the file in the meantime,
	// just wipe out the contents.
	f, err := os.OpenFile(name, os.O_CREATE|os.O_WRONLY|os.O_TRUNC, mode)
	if err != nil {
		return fmt.Errorf("can't open new logfile: %s", err)
	}
	l.file.file = f
	l.file.curSize = 0
	l.file.curIndex = l.getCurFileIndex()
	l.file.createTime = time.Now()
	l.curFileIndex++

	return nil
}


func (l *Writer) openExistingOrNew(writeLen int) error {
	now := time.Now()
	l.file = &fileInfo{}
	l.lastDay = now.Format("20060102")
	l.getCurFileIndex()

	filename := l.FileName
	info, err := os.Stat(filename)
	if os.IsNotExist(err) {
		return l.openNew()
	}

	if err != nil {
		return fmt.Errorf("error getting log file info: %s", err)
	}

	l.file.createTime = now
	l.file.curSize = info.Size()
	l.file.curIndex = l.getCurFileIndex()

	//检查文件大小
	if info.Size()+int64(writeLen) >= l.MaxSize {
		return l.rotate()
	}

	file, err := os.OpenFile(filename, os.O_APPEND|os.O_WRONLY, 0644)
	if err != nil {
		// if we fail to open the old log file for some reason, just ignore
		// it and open a new log file.
		return l.openNew()
	}

	l.file.file = file
	l.curFileIndex++

	return nil
}

// close closes the file if it is open.
func (l *Writer) close() error {
	if l.file == nil || l.file.file == nil {
		return nil
	}

	err := l.file.file.Close()
	return err
}

func (l *Writer) rotate() error {
	if err := l.close(); err != nil {
		return err
	}
	if err := l.openNew(); err != nil {
		return err
	}

	return nil
}

func (l *Writer) dir() string {
	return filepath.Dir(l.FileName)
}

