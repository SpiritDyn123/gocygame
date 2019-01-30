package operation

import (
	"github.com/SpiritDyn123/gocygame/apps/dbsvr/src/global"
	"github.com/SpiritDyn123/gocygame/apps/common/proto"
	"github.com/SpiritDyn123/gocygame/apps/common"
	"github.com/golang/protobuf/proto"
	"github.com/SpiritDyn123/gocygame/libs/log"
	"reflect"
	common_global"github.com/SpiritDyn123/gocygame/apps/common/global"
)

var DbOperationMgr global.IDbOperationMgr
func init() {
	DbOperationMgr = &dbOperationMgr{}
}

type dbOprHandler func(req_msg proto.Message) (proto.Message, error, bool)
type dbOperationMgr struct {
	m_operations_ map[string]dbOprHandler
}

func (mgr *dbOperationMgr) Start() (err  error) {
	global.DBSvrGlobal.GetMsgDispatcher().Register(ProtoMsg.EmMsgId_SVR_MSG_DB_SERVICE,
		&ProtoMsg.PbSvrDBServiceReqMsg{}, mgr.onRecvMsg)

	//注册操作
	mgr.m_operations_ = make(map[string]dbOprHandler)
	mgr.regOpr(&ProtoMsg.PbSvrDBTestRecvReqMsg{}, opr_test_recv)//测试协议

	return
}

func (mgr *dbOperationMgr) Stop() {

}

func (mgr *dbOperationMgr) regOpr(msg proto.Message, handler dbOprHandler) {
	msg_name := reflect.ValueOf(msg).Elem().Type().String()
	if _, ok := mgr.m_operations_[msg_name]; ok {
		log.Error("dbOperationMgr::regOpr repeated msg:%s", msg_name)
		return
	}
	mgr.m_operations_[msg_name] = handler
}

func (mgr *dbOperationMgr) onRecvMsg(sink interface{}, head common.IMsgHead, msg proto.Message) {
	global.DBSvrGlobal.GetGoServer().Go(func(){
		req_msg := msg.(*ProtoMsg.PbSvrDBServiceReqMsg)
		if req_msg.ReqMsgName == "" {
			return
		}

		handler := mgr.m_operations_[req_msg.ReqMsgName]
		if handler == nil {
			log.Error("dbOperationMgr::onRecvMsg proto msg:%s no handler", req_msg.ReqMsgName)
			return
		}

		req_rf_type := proto.MessageType(req_msg.ReqMsgName)
		if req_rf_type == nil {
			log.Error("dbOperationMgr::onRecvMsg invalid proto msg:%s", req_msg.ReqMsgName)
			return
		}

		logic_msg := reflect.New(req_rf_type.Elem()).Interface().(proto.Message)
		err := proto.Unmarshal([]byte(req_msg.ReqData), logic_msg)
		if err != nil {
			log.Error("dbOperationMgr::onRecvMsg Unmarshal proto msg:%s err:%v", req_msg.ReqMsgName, err)
			return
		}

		res_logic_msg, err, b_resp := handler(logic_msg)
		//需要回复
		if b_resp {
			resp_msg := &ProtoMsg.PbSvrDBServiceResMsg{
				Ret: &ProtoMsg.Ret{
					ErrCode: 0,
				},
				DbEngine: req_msg.DbEngine,
			}

			if err != nil {
				resp_msg.Ret.ErrCode = -1
				resp_msg.Ret.ErrMsg = err.Error()
			} else {
				resp_msg.ResMsgName = reflect.ValueOf(res_logic_msg).Elem().Type().String()
				resp_data, err := proto.Marshal(res_logic_msg)
				if err != nil {
					resp_msg.Ret.ErrCode = -2
					resp_msg.Ret.ErrMsg = err.Error()
				} else {
					resp_msg.ResData = string(resp_data)
				}
			}

			sink.(common_global.ILogicSession).Send(head, resp_msg)
		}
	}, func(){})

}