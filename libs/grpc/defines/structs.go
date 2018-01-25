package defines

const (
	CTX_SERVER_ID_KEY = "serverId"
	CTX_SERVER_VERSION_KEY = "serverVer"
)

type ServiceValue struct {
	Addr string
	ServerName string
	ServerId int
	ServiceName string
	Version string
}
