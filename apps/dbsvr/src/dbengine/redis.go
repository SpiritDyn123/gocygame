package dbengine

import (
	"github.com/golang/protobuf/proto"
	"reflect"
	"github.com/SpiritDyn123/gocygame/apps/common/proto"
	"fmt"
	"libs/log"
)

type RedisMsgClientHandler func(message proto.Message)
type redisMsgInfo struct {
	resp_msg_ proto.Message
	handler_  RedisMsgClientHandler
}

type RedisEngineClient struct {
	m_req_msg_info_  map[string]proto.Message
	m_res_msg_info_ map[string]*redisMsgInfo
}

func (rec *RedisEngineClient) Start() (err error) {
	rec.m_req_msg_info_ = make(map[string]proto.Message)
	rec.m_res_msg_info_ = make(map[string]*redisMsgInfo)
	//rec.Svr_global_.GetMsgDispatcher().Register(ProtoMsg.EmMsgId_SVR_MSG_REGISTER_LOGIN)


	return
}

func (rec *RedisEngineClient) Register(req_msg proto.Message, resp_msg proto.Message, handler RedisMsgClientHandler) (err error){
	if req_msg == nil  {
		err = fmt.Errorf("RedisEngineClient::Register req msg nil")
		return
	}

	req_msg_name := reflect.ValueOf(req_msg).Elem().Type().String()
	if _, ok := rec.m_req_msg_info_[req_msg_name]; ok {
		err = fmt.Errorf("RedisEngineClient::Register repeated req msg:%s", req_msg_name)
		return
	}

	rec.m_req_msg_info_[req_msg_name] = resp_msg

	if resp_msg != nil && handler != nil {
		res_msg_name := reflect.ValueOf(resp_msg).Elem().Type().String()
		if _, ok := rec.m_res_msg_info_[res_msg_name]; ok {
			err = fmt.Errorf("RedisEngineClient::Register repeated res msg:%s", req_msg_name)
			return
		}

		rec.m_res_msg_info_[res_msg_name] = &redisMsgInfo{
			handler_: handler,
			resp_msg_: resp_msg,
		}
	}

	return
}

func (rec *RedisEngineClient) Do(req_msg proto.Message) (err error) {
	if req_msg == nil {
		err = fmt.Errorf("RedisEngineClient::Register req msg nil")
		return
	}

	req_msg_name := reflect.ValueOf(req_msg).Elem().Type().String()

	send_msg := &ProtoMsg.PbSvrDBServiceReqMsg{
		DbEngine: ProtoMsg.EmDBEngin_DB_engine_redis,
	}

	send_msg.ReqMsgName = req_msg_name

	var req_data []byte
	req_data, err = proto.Marshal(req_msg)
	if err != nil {
		return
	}
	send_msg.ReqData = string(req_data)

	if resp_msg, ok := rec.m_req_msg_info_[req_msg_name]; ok && resp_msg != nil {
		send_msg.ReqMsgName = reflect.ValueOf(resp_msg).Elem().Type().String()
	}


	return
}

func (rec *RedisEngineClient) onRecvData(service_msg proto.Message) {
	resp_msg := service_msg.(*ProtoMsg.PbSvrDBServiceResMsg)
	if resp_msg.DbEngine != ProtoMsg.EmDBEngin_DB_engine_redis {
		return
	}

	if resp_msg.Ret.ErrCode != 0 {
		log.Error("RedisEngineClient::onRecvData Ret err:%+v", resp_msg.Ret)
		return
	}

	if resp_msg.ResMsgName == "" {
		return
	}

	resp_info, ok := rec.m_res_msg_info_[resp_msg.ResMsgName]
	if !ok {
		return
	}

	err := proto.Unmarshal([]byte(resp_msg.ResData), resp_info.resp_msg_)
	if err != nil {
		log.Error("RedisEngineClient::onRecvData proto Unmarsal msg:%s err:%v", resp_msg.ResMsgName, err)
		return
	}

	resp_info.handler_(resp_info.resp_msg_)
}
