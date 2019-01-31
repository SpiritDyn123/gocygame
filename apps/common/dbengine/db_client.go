package dbengine

import (
	"fmt"
	"github.com/SpiritDyn123/gocygame/apps/common"
	"github.com/SpiritDyn123/gocygame/apps/common/global"
	"github.com/SpiritDyn123/gocygame/apps/common/net/svr_info"
	"github.com/SpiritDyn123/gocygame/apps/common/proto"
	"github.com/SpiritDyn123/gocygame/libs/log"
	"github.com/golang/protobuf/proto"
	"reflect"
)

type DBClientHandler func(message proto.Message)
type dbMsgInfo struct {
	resp_msg_ proto.Message
	handler_  DBClientHandler
}

type DBEngineClient struct {
	Svr_global_ global.IServerGlobal
	Svr_group_  *svr_info.SvrGroup

	m_res_msg_info_ map[string]*dbMsgInfo
}

func (rec *DBEngineClient) Start() (err error) {
	rec.m_res_msg_info_ = make(map[string]*dbMsgInfo)

	rec.Svr_global_.GetMsgDispatcher().Register(uint32(ProtoMsg.EmSSMsgId_SVR_MSG_DB_SERVICE), &ProtoMsg.PbSvrDBServiceResMsg{}, rec.onRecvData)

	return
}

func (rec *DBEngineClient) Register(resp_msg proto.Message, handler DBClientHandler) (err error){
	if resp_msg == nil || handler == nil {
		err = fmt.Errorf("DBEngineClient::Register resp msg nil")
		return
	}

	res_msg_name := reflect.ValueOf(resp_msg).Elem().Type().String()
	if _, ok := rec.m_res_msg_info_[res_msg_name]; ok {
		err = fmt.Errorf("DBEngineClient::Register repeated res msg:%s", res_msg_name)
		return
	}

	rec.m_res_msg_info_[res_msg_name] = &dbMsgInfo{
		handler_: handler,
		resp_msg_: resp_msg,
	}

	return
}

func (rec *DBEngineClient) Do(engine ProtoMsg.EmDBEngin, uid uint64, req_msg proto.Message) (err error) {
	if req_msg == nil {
		err = fmt.Errorf("DBEngineClient::Do req msg nil")
		return
	}

	req_msg_name := reflect.ValueOf(req_msg).Elem().Type().String()

	send_msg := &ProtoMsg.PbSvrDBServiceReqMsg{
		DbEngine: engine,
	}

	send_msg.ReqMsgName = req_msg_name

	var req_data []byte
	req_data, err = proto.Marshal(req_msg)
	if err != nil {
		return
	}
	send_msg.ReqData = string(req_data)

	rec.Svr_group_.Send(uid,
		&common.ProtocolInnerHead{Msg_id_: uint32(ProtoMsg.EmSSMsgId_SVR_MSG_DB_SERVICE)},
		send_msg)
	return
}

func (rec *DBEngineClient) DoRedis(uid uint64, req_msg proto.Message) (err error) {
	return rec.Do(ProtoMsg.EmDBEngin_DB_engine_redis, uid, req_msg)
}

//
func (rec *DBEngineClient) DoMysql(uid uint64, req_msg proto.Message) (err error) {
	return rec.Do(ProtoMsg.EmDBEngin_DB_engine_mysql, uid, req_msg)
	return
}

func (rec *DBEngineClient) onRecvData(sink interface{}, head common.IMsgHead, msg proto.Message) {
	resp_msg := msg.(*ProtoMsg.PbSvrDBServiceResMsg)
	if resp_msg.Ret.ErrCode != 0 {
		log.Error("DBEngineClient::onRecvData Ret err:%+v", resp_msg.Ret)
		return
	}

	if resp_msg.ResMsgName == "" {
		return
	}

	resp_info, ok := rec.m_res_msg_info_[resp_msg.ResMsgName]
	if !ok {
		log.Error("DBEngineClient::onRecvData msg:%s no handler", resp_msg.ResMsgName)
		return
	}

	err := proto.Unmarshal([]byte(resp_msg.ResData), resp_info.resp_msg_)
	if err != nil {
		log.Error("DBEngineClient::onRecvData proto Unmarsal msg:%s err:%v", resp_msg.ResMsgName, err)
		return
	}

	resp_info.handler_(resp_info.resp_msg_)
}
