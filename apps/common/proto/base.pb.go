// Code generated by protoc-gen-go. DO NOT EDIT.
// source: base.proto

/*
Package ProtoMsg is a generated protocol buffer package.

It is generated from these files:
	base.proto
	cluster.proto
	db.proto
	game.proto
	login.proto
	user.proto
	world.proto

It has these top-level messages:
	Ret
	PbSvrBaseInfo
	PbSvrRegisterClusterReqMsg
	PbSvrRegisterClusterResMsg
	PbSvrBroadClusterMsg
	PbSvrRegisterDBReqMsg
	PbSvrRegisterDBResMsg
	PbSvrDBServiceReqMsg
	PbSvrDBServiceResMsg
	PbSvrDBTestRecvReqMsg
	PbSvrDBTestRecvResMsg
	PbSvrRegisterGameReqMsg
	PbSvrRegisterGameResMsg
	PbSvrRegisterLoginReqMsg
	PbSvrRegisterLoginResMsg
	PbSvrRegisterWorldReqMsg
	PbSvrRegisterWorldResMsg
*/
package ProtoMsg

import proto "github.com/golang/protobuf/proto"
import fmt "fmt"
import math "math"

// Reference imports to suppress errors if they are not otherwise used.
var _ = proto.Marshal
var _ = fmt.Errorf
var _ = math.Inf

// This is a compile-time assertion to ensure that this generated file
// is compatible with the proto package it is being compiled against.
// A compilation error at this line likely means your copy of the
// proto package needs to be updated.
const _ = proto.ProtoPackageIsVersion2 // please upgrade the proto package

type EmMsgId int32

const (
	EmMsgId_MSG_INVALID    EmMsgId = 0
	EmMsgId_MSG_HEART_BEAT EmMsgId = 1
	EmMsgId_CLI_MSG_BEGIN  EmMsgId = 100000
	EmMsgId_CLI_MSG_LOGIN  EmMsgId = 100001
	EmMsgId_CLI_MSG_END    EmMsgId = 199999
	EmMsgId_SVR_MSG_BEGIN  EmMsgId = 200000
	// 集群服务
	EmMsgId_SVR_MSG_REGISTER_CLUSTER EmMsgId = 200001
	EmMsgId_SVR_MSG_BROAD_CLUSTER    EmMsgId = 200002
	// game
	EmMsgId_SVR_MSG_REGISTER_GAME EmMsgId = 201001
	// world
	EmMsgId_SVR_MSG_REGISTER_WORLD EmMsgId = 202001
	// 登陆
	EmMsgId_SVR_MSG_REGISTER_LOGIN EmMsgId = 203001
	// db相关
	EmMsgId_SVR_MSG_REGISTER_DB EmMsgId = 204001
	EmMsgId_SVR_MSG_DB_SERVICE  EmMsgId = 204002
	EmMsgId_SVR_MSG_END         EmMsgId = 299999
)

var EmMsgId_name = map[int32]string{
	0:      "MSG_INVALID",
	1:      "MSG_HEART_BEAT",
	100000: "CLI_MSG_BEGIN",
	100001: "CLI_MSG_LOGIN",
	199999: "CLI_MSG_END",
	200000: "SVR_MSG_BEGIN",
	200001: "SVR_MSG_REGISTER_CLUSTER",
	200002: "SVR_MSG_BROAD_CLUSTER",
	201001: "SVR_MSG_REGISTER_GAME",
	202001: "SVR_MSG_REGISTER_WORLD",
	203001: "SVR_MSG_REGISTER_LOGIN",
	204001: "SVR_MSG_REGISTER_DB",
	204002: "SVR_MSG_DB_SERVICE",
	299999: "SVR_MSG_END",
}
var EmMsgId_value = map[string]int32{
	"MSG_INVALID":              0,
	"MSG_HEART_BEAT":           1,
	"CLI_MSG_BEGIN":            100000,
	"CLI_MSG_LOGIN":            100001,
	"CLI_MSG_END":              199999,
	"SVR_MSG_BEGIN":            200000,
	"SVR_MSG_REGISTER_CLUSTER": 200001,
	"SVR_MSG_BROAD_CLUSTER":    200002,
	"SVR_MSG_REGISTER_GAME":    201001,
	"SVR_MSG_REGISTER_WORLD":   202001,
	"SVR_MSG_REGISTER_LOGIN":   203001,
	"SVR_MSG_REGISTER_DB":      204001,
	"SVR_MSG_DB_SERVICE":       204002,
	"SVR_MSG_END":              299999,
}

func (x EmMsgId) String() string {
	return proto.EnumName(EmMsgId_name, int32(x))
}
func (EmMsgId) EnumDescriptor() ([]byte, []int) { return fileDescriptor0, []int{0} }

type Ret struct {
	ErrCode int32  `protobuf:"varint,1,opt,name=err_code,json=errCode" json:"err_code,omitempty"`
	ErrMsg  string `protobuf:"bytes,2,opt,name=err_msg,json=errMsg" json:"err_msg,omitempty"`
}

func (m *Ret) Reset()                    { *m = Ret{} }
func (m *Ret) String() string            { return proto.CompactTextString(m) }
func (*Ret) ProtoMessage()               {}
func (*Ret) Descriptor() ([]byte, []int) { return fileDescriptor0, []int{0} }

func (m *Ret) GetErrCode() int32 {
	if m != nil {
		return m.ErrCode
	}
	return 0
}

func (m *Ret) GetErrMsg() string {
	if m != nil {
		return m.ErrMsg
	}
	return ""
}

func init() {
	proto.RegisterType((*Ret)(nil), "ProtoMsg.Ret")
	proto.RegisterEnum("ProtoMsg.EmMsgId", EmMsgId_name, EmMsgId_value)
}

func init() { proto.RegisterFile("base.proto", fileDescriptor0) }

var fileDescriptor0 = []byte{
	// 315 bytes of a gzipped FileDescriptorProto
	0x1f, 0x8b, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0xff, 0x6c, 0x91, 0x3d, 0x4b, 0x03, 0x31,
	0x1c, 0xc6, 0xbd, 0x8a, 0xd7, 0xfa, 0x6f, 0xd4, 0xf8, 0x2f, 0xea, 0x15, 0x45, 0x8a, 0x53, 0x71,
	0x70, 0x71, 0x72, 0xbc, 0x97, 0x70, 0x06, 0xee, 0x5a, 0xc9, 0xd5, 0x3a, 0x06, 0x6b, 0xc3, 0x4d,
	0xe5, 0x24, 0xd7, 0xaf, 0xd1, 0xc1, 0xb5, 0x93, 0xba, 0x88, 0x6e, 0x8e, 0x9d, 0x7c, 0xf9, 0x20,
	0x8a, 0x7e, 0x0b, 0x37, 0x89, 0x72, 0xa5, 0x52, 0xa7, 0x27, 0xfc, 0x7e, 0x79, 0x20, 0x0f, 0x01,
	0xe8, 0x9d, 0xe7, 0xea, 0xe0, 0x52, 0x67, 0xc3, 0x0c, 0x2b, 0x27, 0x26, 0xe2, 0x3c, 0xdd, 0x3b,
	0x82, 0x45, 0xa1, 0x86, 0x58, 0x87, 0x8a, 0xd2, 0x5a, 0x5e, 0x64, 0x7d, 0xe5, 0x58, 0x0d, 0xab,
	0xb9, 0x24, 0xca, 0x4a, 0x6b, 0x3f, 0xeb, 0x2b, 0xdc, 0x02, 0x73, 0x94, 0x83, 0x3c, 0x75, 0x4a,
	0x0d, 0xab, 0xb9, 0x2c, 0x6c, 0xa5, 0x75, 0x9c, 0xa7, 0xfb, 0x6f, 0x25, 0x28, 0xab, 0x41, 0x9c,
	0xa7, 0xbc, 0x8f, 0x6b, 0x50, 0x8d, 0x93, 0x50, 0xf2, 0x56, 0xd7, 0x8d, 0x78, 0x40, 0x17, 0x10,
	0x61, 0xd5, 0x80, 0x63, 0xe6, 0x8a, 0x8e, 0xf4, 0x98, 0xdb, 0xa1, 0x16, 0xd6, 0x60, 0xc5, 0x8f,
	0xb8, 0x34, 0xdc, 0x63, 0x21, 0x6f, 0xd1, 0xeb, 0x91, 0x3d, 0x0b, 0xa3, 0xb6, 0x81, 0x37, 0x23,
	0x1b, 0xd7, 0xa1, 0x5a, 0x40, 0xd6, 0x0a, 0xe8, 0xd3, 0x98, 0x98, 0x7b, 0x49, 0x57, 0xcc, 0x94,
	0x9f, 0xc7, 0x04, 0x77, 0xc1, 0x29, 0xa0, 0x60, 0x21, 0x4f, 0x3a, 0x4c, 0x48, 0x3f, 0x3a, 0x35,
	0x49, 0x5f, 0xc6, 0x04, 0xb7, 0x61, 0x63, 0x5a, 0x12, 0x6d, 0x37, 0x98, 0xca, 0xd7, 0xbf, 0x72,
	0x5a, 0x0e, 0xdd, 0x98, 0xd1, 0xfb, 0x5b, 0x82, 0x3b, 0xb0, 0x39, 0x27, 0xcf, 0xda, 0x22, 0x0a,
	0xe8, 0xd5, 0xc3, 0xff, 0xf6, 0xf7, 0xf5, 0x5f, 0x8f, 0x04, 0xeb, 0x50, 0x9b, 0xb3, 0x81, 0x47,
	0x3f, 0x26, 0x04, 0x1d, 0xc0, 0x42, 0x05, 0x9e, 0x4c, 0x98, 0xe8, 0x72, 0x9f, 0xd1, 0xcf, 0x09,
	0x31, 0x93, 0x0b, 0x63, 0x26, 0xbf, 0xdf, 0x61, 0xcf, 0xfe, 0xf9, 0xac, 0xc3, 0xef, 0x00, 0x00,
	0x00, 0xff, 0xff, 0xea, 0x00, 0x42, 0xdb, 0xba, 0x01, 0x00, 0x00,
}
