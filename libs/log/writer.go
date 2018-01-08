package log

import "io"

type LogWriter interface {
	io.WriteCloser
}
