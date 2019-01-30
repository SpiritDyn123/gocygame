// Code generated by protoc-gen-go. DO NOT EDIT.
// source: db.proto

package ProtoMsg

import proto "github.com/golang/protobuf/proto"
import fmt "fmt"
import math "math"

// Reference imports to suppress errors if they are not otherwise used.
var _ = proto.Marshal
var _ = fmt.Errorf
var _ = math.Inf

type EmDBEngin int32

const (
	EmDBEngin_DB_engine_invalid EmDBEngin = 0
	EmDBEngin_DB_engine_redis   EmDBEngin = 1
	EmDBEngin_DB_engine_mysql   EmDBEngin = 2
)

var EmDBEngin_name = map[int32]string{
	0: "DB_engine_invalid",
	1: "DB_engine_redis",
	2: "DB_engine_mysql",
}
var EmDBEngin_value = map[string]int32{
	"DB_engine_invalid": 0,
	"DB_engine_redis":   1,
	"DB_engine_mysql":   2,
}

func (x EmDBEngin) String() string {
	return proto.EnumName(EmDBEngin_name, int32(x))
}
func (EmDBEngin) EnumDescriptor() ([]byte, []int) { return fileDescriptor2, []int{0} }

// SVR_MSG_REGISTER_DB
type PbSvrRegisterDBReqMsg struct {
	SvrInfo *PbSvrBaseInfo `protobuf:"bytes,1,opt,name=svr_info,json=svrInfo" json:"svr_info,omitempty"`
}

func (m *PbSvrRegisterDBReqMsg) Reset()                    { *m = PbSvrRegisterDBReqMsg{} }
func (m *PbSvrRegisterDBReqMsg) String() string            { return proto.CompactTextString(m) }
func (*PbSvrRegisterDBReqMsg) ProtoMessage()               {}
func (*PbSvrRegisterDBReqMsg) Descriptor() ([]byte, []int) { return fileDescriptor2, []int{0} }

func (m *PbSvrRegisterDBReqMsg) GetSvrInfo() *PbSvrBaseInfo {
	if m != nil {
		return m.SvrInfo
	}
	return nil
}

type PbSvrRegisterDBResMsg struct {
	Ret *Ret `protobuf:"bytes,1,opt,name=ret" json:"ret,omitempty"`
}

func (m *PbSvrRegisterDBResMsg) Reset()                    { *m = PbSvrRegisterDBResMsg{} }
func (m *PbSvrRegisterDBResMsg) String() string            { return proto.CompactTextString(m) }
func (*PbSvrRegisterDBResMsg) ProtoMessage()               {}
func (*PbSvrRegisterDBResMsg) Descriptor() ([]byte, []int) { return fileDescriptor2, []int{1} }

func (m *PbSvrRegisterDBResMsg) GetRet() *Ret {
	if m != nil {
		return m.Ret
	}
	return nil
}

type PbSvrDBServiceReqMsg struct {
	DbEngine   EmDBEngin `protobuf:"varint,1,opt,name=db_engine,json=dbEngine,enum=ProtoMsg.EmDBEngin" json:"db_engine,omitempty"`
	ReqMsgName string    `protobuf:"bytes,2,opt,name=req_msg_name,json=reqMsgName" json:"req_msg_name,omitempty"`
	ReqData    string    `protobuf:"bytes,3,opt,name=req_data,json=reqData" json:"req_data,omitempty"`
}

func (m *PbSvrDBServiceReqMsg) Reset()                    { *m = PbSvrDBServiceReqMsg{} }
func (m *PbSvrDBServiceReqMsg) String() string            { return proto.CompactTextString(m) }
func (*PbSvrDBServiceReqMsg) ProtoMessage()               {}
func (*PbSvrDBServiceReqMsg) Descriptor() ([]byte, []int) { return fileDescriptor2, []int{2} }

func (m *PbSvrDBServiceReqMsg) GetDbEngine() EmDBEngin {
	if m != nil {
		return m.DbEngine
	}
	return EmDBEngin_DB_engine_invalid
}

func (m *PbSvrDBServiceReqMsg) GetReqMsgName() string {
	if m != nil {
		return m.ReqMsgName
	}
	return ""
}

func (m *PbSvrDBServiceReqMsg) GetReqData() string {
	if m != nil {
		return m.ReqData
	}
	return ""
}

type PbSvrDBServiceResMsg struct {
	Ret        *Ret      `protobuf:"bytes,1,opt,name=ret" json:"ret,omitempty"`
	DbEngine   EmDBEngin `protobuf:"varint,2,opt,name=db_engine,json=dbEngine,enum=ProtoMsg.EmDBEngin" json:"db_engine,omitempty"`
	ResMsgName string    `protobuf:"bytes,3,opt,name=res_msg_name,json=resMsgName" json:"res_msg_name,omitempty"`
	ResData    string    `protobuf:"bytes,4,opt,name=res_data,json=resData" json:"res_data,omitempty"`
}

func (m *PbSvrDBServiceResMsg) Reset()                    { *m = PbSvrDBServiceResMsg{} }
func (m *PbSvrDBServiceResMsg) String() string            { return proto.CompactTextString(m) }
func (*PbSvrDBServiceResMsg) ProtoMessage()               {}
func (*PbSvrDBServiceResMsg) Descriptor() ([]byte, []int) { return fileDescriptor2, []int{3} }

func (m *PbSvrDBServiceResMsg) GetRet() *Ret {
	if m != nil {
		return m.Ret
	}
	return nil
}

func (m *PbSvrDBServiceResMsg) GetDbEngine() EmDBEngin {
	if m != nil {
		return m.DbEngine
	}
	return EmDBEngin_DB_engine_invalid
}

func (m *PbSvrDBServiceResMsg) GetResMsgName() string {
	if m != nil {
		return m.ResMsgName
	}
	return ""
}

func (m *PbSvrDBServiceResMsg) GetResData() string {
	if m != nil {
		return m.ResData
	}
	return ""
}

type PbSvrDBTestRecvReqMsg struct {
	Id   int32  `protobuf:"varint,1,opt,name=id" json:"id,omitempty"`
	Name string `protobuf:"bytes,2,opt,name=name" json:"name,omitempty"`
}

func (m *PbSvrDBTestRecvReqMsg) Reset()                    { *m = PbSvrDBTestRecvReqMsg{} }
func (m *PbSvrDBTestRecvReqMsg) String() string            { return proto.CompactTextString(m) }
func (*PbSvrDBTestRecvReqMsg) ProtoMessage()               {}
func (*PbSvrDBTestRecvReqMsg) Descriptor() ([]byte, []int) { return fileDescriptor2, []int{4} }

func (m *PbSvrDBTestRecvReqMsg) GetId() int32 {
	if m != nil {
		return m.Id
	}
	return 0
}

func (m *PbSvrDBTestRecvReqMsg) GetName() string {
	if m != nil {
		return m.Name
	}
	return ""
}

type PbSvrDBTestRecvResMsg struct {
	Id   int32  `protobuf:"varint,1,opt,name=id" json:"id,omitempty"`
	Name string `protobuf:"bytes,2,opt,name=name" json:"name,omitempty"`
}

func (m *PbSvrDBTestRecvResMsg) Reset()                    { *m = PbSvrDBTestRecvResMsg{} }
func (m *PbSvrDBTestRecvResMsg) String() string            { return proto.CompactTextString(m) }
func (*PbSvrDBTestRecvResMsg) ProtoMessage()               {}
func (*PbSvrDBTestRecvResMsg) Descriptor() ([]byte, []int) { return fileDescriptor2, []int{5} }

func (m *PbSvrDBTestRecvResMsg) GetId() int32 {
	if m != nil {
		return m.Id
	}
	return 0
}

func (m *PbSvrDBTestRecvResMsg) GetName() string {
	if m != nil {
		return m.Name
	}
	return ""
}

func init() {
	proto.RegisterType((*PbSvrRegisterDBReqMsg)(nil), "ProtoMsg.PbSvrRegisterDBReqMsg")
	proto.RegisterType((*PbSvrRegisterDBResMsg)(nil), "ProtoMsg.PbSvrRegisterDBResMsg")
	proto.RegisterType((*PbSvrDBServiceReqMsg)(nil), "ProtoMsg.PbSvrDBServiceReqMsg")
	proto.RegisterType((*PbSvrDBServiceResMsg)(nil), "ProtoMsg.PbSvrDBServiceResMsg")
	proto.RegisterType((*PbSvrDBTestRecvReqMsg)(nil), "ProtoMsg.PbSvrDBTestRecvReqMsg")
	proto.RegisterType((*PbSvrDBTestRecvResMsg)(nil), "ProtoMsg.PbSvrDBTestRecvResMsg")
	proto.RegisterEnum("ProtoMsg.EmDBEngin", EmDBEngin_name, EmDBEngin_value)
}

func init() { proto.RegisterFile("db.proto", fileDescriptor2) }

var fileDescriptor2 = []byte{
	// 356 bytes of a gzipped FileDescriptorProto
	0x1f, 0x8b, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0xff, 0x8c, 0x92, 0x41, 0x4f, 0xea, 0x40,
	0x14, 0x85, 0x5f, 0x0b, 0xef, 0x51, 0xee, 0x13, 0xc4, 0x41, 0x22, 0xb2, 0x91, 0x74, 0x45, 0x5c,
	0x10, 0x83, 0x1b, 0x13, 0x77, 0x4d, 0x5d, 0x18, 0xc5, 0x90, 0xc1, 0x7d, 0x33, 0x65, 0x2e, 0xcd,
	0x24, 0xb4, 0xb5, 0x73, 0x6b, 0x13, 0x7f, 0x80, 0x7f, 0xc5, 0xdf, 0x69, 0x3a, 0x14, 0x2b, 0x09,
	0x0b, 0x56, 0x3d, 0x99, 0x73, 0xcf, 0xc9, 0x77, 0x6f, 0x0a, 0x8e, 0x0c, 0xa7, 0x6f, 0x3a, 0xcd,
	0x53, 0xe6, 0x2c, 0xca, 0xcf, 0x9c, 0xa2, 0x51, 0x67, 0xb5, 0x79, 0xa7, 0x1c, 0xf5, 0xd6, 0x18,
	0x41, 0x28, 0x08, 0xb7, 0xda, 0x7d, 0x82, 0xc1, 0x22, 0x5c, 0x16, 0x9a, 0x63, 0xa4, 0xca, 0x11,
	0xdf, 0xe3, 0x98, 0xcd, 0x29, 0x62, 0x33, 0x70, 0xa8, 0xd0, 0x81, 0x4a, 0xd6, 0xe9, 0xd0, 0x1a,
	0x5b, 0x93, 0xff, 0xb3, 0x8b, 0xe9, 0xae, 0x70, 0x6a, 0x22, 0x9e, 0x20, 0x7c, 0x4c, 0xd6, 0x29,
	0x6f, 0x51, 0xa1, 0x4b, 0xe1, 0xde, 0x1d, 0x28, 0xa3, 0xb2, 0xec, 0x0a, 0x1a, 0x1a, 0xf3, 0xaa,
	0xa7, 0x53, 0xf7, 0x70, 0xcc, 0x79, 0xe9, 0xb8, 0x9f, 0x16, 0x9c, 0x9b, 0xa8, 0xef, 0x2d, 0x51,
	0x17, 0x6a, 0x85, 0x15, 0xc6, 0x0d, 0xb4, 0x65, 0x18, 0x60, 0x12, 0xa9, 0x04, 0x4d, 0xbe, 0x3b,
	0xeb, 0xd7, 0x79, 0x8c, 0x7d, 0xef, 0xa1, 0xf4, 0xb8, 0x23, 0x43, 0x23, 0x90, 0x8d, 0xe1, 0x44,
	0x63, 0x16, 0xc4, 0x14, 0x05, 0x89, 0x88, 0x71, 0x68, 0x8f, 0xad, 0x49, 0x9b, 0x83, 0x36, 0x7d,
	0x2f, 0x22, 0x46, 0x76, 0x09, 0x4e, 0x39, 0x21, 0x45, 0x2e, 0x86, 0x0d, 0xe3, 0xb6, 0x34, 0x66,
	0xbe, 0xc8, 0x85, 0xfb, 0x75, 0x80, 0xe3, 0xa8, 0x0d, 0xf6, 0x41, 0xed, 0xa3, 0x41, 0xa9, 0x06,
	0x6d, 0xec, 0x40, 0x69, 0x0f, 0x94, 0xb6, 0xa0, 0xcd, 0x1d, 0x28, 0x19, 0xd0, 0xfb, 0xea, 0xd4,
	0xbe, 0xf7, 0x8a, 0x94, 0x73, 0x5c, 0x15, 0xd5, 0xc1, 0xba, 0x60, 0x2b, 0x69, 0x38, 0xff, 0x72,
	0x5b, 0x49, 0xc6, 0xa0, 0xf9, 0xeb, 0x0c, 0x46, 0x1f, 0x0c, 0xd3, 0x91, 0xe1, 0xeb, 0x67, 0x68,
	0xff, 0x6c, 0xc3, 0x06, 0x70, 0xe6, 0x7b, 0xd5, 0xd6, 0x81, 0x4a, 0x0a, 0xb1, 0x51, 0xb2, 0xf7,
	0x87, 0xf5, 0xe1, 0xb4, 0x7e, 0xd6, 0x28, 0x15, 0xf5, 0xac, 0xfd, 0xc7, 0xf8, 0x83, 0xb2, 0x4d,
	0xcf, 0x0e, 0xff, 0x99, 0xdf, 0xf0, 0xf6, 0x3b, 0x00, 0x00, 0xff, 0xff, 0x40, 0x48, 0x6f, 0xce,
	0xb7, 0x02, 0x00, 0x00,
}