package rpc


type RpcMonitor interface {

}

type RpcServer interface {
	SetMonitor(mon RpcMonitor)
	Register(obj interface{}) error
	UnRegister(obj interface{}) error
}

type RpcClient interface {

}
