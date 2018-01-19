package grpc

const (
	CTX_SERVER_ID_KEY = "serverId"
)

type ServiceValue struct {
	Addr string
	ServerName string
	ServerId int
	ServiceName string
}
