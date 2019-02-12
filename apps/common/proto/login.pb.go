// Code generated by protoc-gen-go. DO NOT EDIT.
// source: login.proto

package ProtoMsg

import proto "github.com/golang/protobuf/proto"
import fmt "fmt"
import math "math"

// Reference imports to suppress errors if they are not otherwise used.
var _ = proto.Marshal
var _ = fmt.Errorf
var _ = math.Inf

type EmLoginType int32

const (
	EmLoginType_Login_type_invalid EmLoginType = 0
	EmLoginType_Login_type_visitor EmLoginType = 1
	EmLoginType_Login_type_user    EmLoginType = 2
	EmLoginType_Login_type_qq      EmLoginType = 3
	EmLoginType_Login_type_wechat  EmLoginType = 4
	EmLoginType_Login_type_alipay  EmLoginType = 5
)

var EmLoginType_name = map[int32]string{
	0: "Login_type_invalid",
	1: "Login_type_visitor",
	2: "Login_type_user",
	3: "Login_type_qq",
	4: "Login_type_wechat",
	5: "Login_type_alipay",
}
var EmLoginType_value = map[string]int32{
	"Login_type_invalid": 0,
	"Login_type_visitor": 1,
	"Login_type_user":    2,
	"Login_type_qq":      3,
	"Login_type_wechat":  4,
	"Login_type_alipay":  5,
}

func (x EmLoginType) String() string {
	return proto.EnumName(EmLoginType_name, int32(x))
}
func (EmLoginType) EnumDescriptor() ([]byte, []int) { return fileDescriptor5, []int{0} }

type EmDeviceType int32

const (
	EmDeviceType_Device_type_invalid EmDeviceType = 0
	EmDeviceType_Device_type_android EmDeviceType = 1
	EmDeviceType_Device_type_ios     EmDeviceType = 2
	EmDeviceType_Device_type_ipad    EmDeviceType = 3
	EmDeviceType_Device_type_mac     EmDeviceType = 4
	EmDeviceType_Device_type_windows EmDeviceType = 5
)

var EmDeviceType_name = map[int32]string{
	0: "Device_type_invalid",
	1: "Device_type_android",
	2: "Device_type_ios",
	3: "Device_type_ipad",
	4: "Device_type_mac",
	5: "Device_type_windows",
}
var EmDeviceType_value = map[string]int32{
	"Device_type_invalid": 0,
	"Device_type_android": 1,
	"Device_type_ios":     2,
	"Device_type_ipad":    3,
	"Device_type_mac":     4,
	"Device_type_windows": 5,
}

func (x EmDeviceType) String() string {
	return proto.EnumName(EmDeviceType_name, int32(x))
}
func (EmDeviceType) EnumDescriptor() ([]byte, []int) { return fileDescriptor5, []int{1} }

// SVR_MSG_REGISTER_LOGIN
type PbSvrRegisterLoginReqMsg struct {
	SvrInfo *PbSvrBaseInfo `protobuf:"bytes,1,opt,name=svr_info,json=svrInfo" json:"svr_info,omitempty"`
}

func (m *PbSvrRegisterLoginReqMsg) Reset()                    { *m = PbSvrRegisterLoginReqMsg{} }
func (m *PbSvrRegisterLoginReqMsg) String() string            { return proto.CompactTextString(m) }
func (*PbSvrRegisterLoginReqMsg) ProtoMessage()               {}
func (*PbSvrRegisterLoginReqMsg) Descriptor() ([]byte, []int) { return fileDescriptor5, []int{0} }

func (m *PbSvrRegisterLoginReqMsg) GetSvrInfo() *PbSvrBaseInfo {
	if m != nil {
		return m.SvrInfo
	}
	return nil
}

type PbSvrRegisterLoginResMsg struct {
	Ret *Ret `protobuf:"bytes,1,opt,name=ret" json:"ret,omitempty"`
}

func (m *PbSvrRegisterLoginResMsg) Reset()                    { *m = PbSvrRegisterLoginResMsg{} }
func (m *PbSvrRegisterLoginResMsg) String() string            { return proto.CompactTextString(m) }
func (*PbSvrRegisterLoginResMsg) ProtoMessage()               {}
func (*PbSvrRegisterLoginResMsg) Descriptor() ([]byte, []int) { return fileDescriptor5, []int{1} }

func (m *PbSvrRegisterLoginResMsg) GetRet() *Ret {
	if m != nil {
		return m.Ret
	}
	return nil
}

type PbCsPlayerLoginReqMsg struct {
	LoginType    EmLoginType  `protobuf:"varint,1,opt,name=login_type,json=loginType,enum=ProtoMsg.EmLoginType" json:"login_type,omitempty"`
	DeviceType   EmDeviceType `protobuf:"varint,2,opt,name=device_type,json=deviceType,enum=ProtoMsg.EmDeviceType" json:"device_type,omitempty"`
	DeviceDesc   string       `protobuf:"bytes,3,opt,name=device_desc,json=deviceDesc" json:"device_desc,omitempty"`
	Version      string       `protobuf:"bytes,4,opt,name=version" json:"version,omitempty"`
	PlatfomToken string       `protobuf:"bytes,5,opt,name=platfom_token,json=platfomToken" json:"platfom_token,omitempty"`
	UserName     string       `protobuf:"bytes,6,opt,name=user_name,json=userName" json:"user_name,omitempty"`
	UserPwd      string       `protobuf:"bytes,7,opt,name=user_pwd,json=userPwd" json:"user_pwd,omitempty"`
}

func (m *PbCsPlayerLoginReqMsg) Reset()                    { *m = PbCsPlayerLoginReqMsg{} }
func (m *PbCsPlayerLoginReqMsg) String() string            { return proto.CompactTextString(m) }
func (*PbCsPlayerLoginReqMsg) ProtoMessage()               {}
func (*PbCsPlayerLoginReqMsg) Descriptor() ([]byte, []int) { return fileDescriptor5, []int{2} }

func (m *PbCsPlayerLoginReqMsg) GetLoginType() EmLoginType {
	if m != nil {
		return m.LoginType
	}
	return EmLoginType_Login_type_invalid
}

func (m *PbCsPlayerLoginReqMsg) GetDeviceType() EmDeviceType {
	if m != nil {
		return m.DeviceType
	}
	return EmDeviceType_Device_type_invalid
}

func (m *PbCsPlayerLoginReqMsg) GetDeviceDesc() string {
	if m != nil {
		return m.DeviceDesc
	}
	return ""
}

func (m *PbCsPlayerLoginReqMsg) GetVersion() string {
	if m != nil {
		return m.Version
	}
	return ""
}

func (m *PbCsPlayerLoginReqMsg) GetPlatfomToken() string {
	if m != nil {
		return m.PlatfomToken
	}
	return ""
}

func (m *PbCsPlayerLoginReqMsg) GetUserName() string {
	if m != nil {
		return m.UserName
	}
	return ""
}

func (m *PbCsPlayerLoginReqMsg) GetUserPwd() string {
	if m != nil {
		return m.UserPwd
	}
	return ""
}

type PbCsPlayerLoginResMsg struct {
	Ret *Ret   `protobuf:"bytes,1,opt,name=ret" json:"ret,omitempty"`
	Uid uint64 `protobuf:"varint,2,opt,name=uid" json:"uid,omitempty"`
}

func (m *PbCsPlayerLoginResMsg) Reset()                    { *m = PbCsPlayerLoginResMsg{} }
func (m *PbCsPlayerLoginResMsg) String() string            { return proto.CompactTextString(m) }
func (*PbCsPlayerLoginResMsg) ProtoMessage()               {}
func (*PbCsPlayerLoginResMsg) Descriptor() ([]byte, []int) { return fileDescriptor5, []int{3} }

func (m *PbCsPlayerLoginResMsg) GetRet() *Ret {
	if m != nil {
		return m.Ret
	}
	return nil
}

func (m *PbCsPlayerLoginResMsg) GetUid() uint64 {
	if m != nil {
		return m.Uid
	}
	return 0
}

type PbCsPlayerBindReqMsg struct {
}

func (m *PbCsPlayerBindReqMsg) Reset()                    { *m = PbCsPlayerBindReqMsg{} }
func (m *PbCsPlayerBindReqMsg) String() string            { return proto.CompactTextString(m) }
func (*PbCsPlayerBindReqMsg) ProtoMessage()               {}
func (*PbCsPlayerBindReqMsg) Descriptor() ([]byte, []int) { return fileDescriptor5, []int{4} }

type PbCsPlayerBindResMsg struct {
	Ret *Ret `protobuf:"bytes,1,opt,name=ret" json:"ret,omitempty"`
}

func (m *PbCsPlayerBindResMsg) Reset()                    { *m = PbCsPlayerBindResMsg{} }
func (m *PbCsPlayerBindResMsg) String() string            { return proto.CompactTextString(m) }
func (*PbCsPlayerBindResMsg) ProtoMessage()               {}
func (*PbCsPlayerBindResMsg) Descriptor() ([]byte, []int) { return fileDescriptor5, []int{5} }

func (m *PbCsPlayerBindResMsg) GetRet() *Ret {
	if m != nil {
		return m.Ret
	}
	return nil
}

func init() {
	proto.RegisterType((*PbSvrRegisterLoginReqMsg)(nil), "ProtoMsg.PbSvrRegisterLoginReqMsg")
	proto.RegisterType((*PbSvrRegisterLoginResMsg)(nil), "ProtoMsg.PbSvrRegisterLoginResMsg")
	proto.RegisterType((*PbCsPlayerLoginReqMsg)(nil), "ProtoMsg.PbCsPlayerLoginReqMsg")
	proto.RegisterType((*PbCsPlayerLoginResMsg)(nil), "ProtoMsg.PbCsPlayerLoginResMsg")
	proto.RegisterType((*PbCsPlayerBindReqMsg)(nil), "ProtoMsg.PbCsPlayerBindReqMsg")
	proto.RegisterType((*PbCsPlayerBindResMsg)(nil), "ProtoMsg.PbCsPlayerBindResMsg")
	proto.RegisterEnum("ProtoMsg.EmLoginType", EmLoginType_name, EmLoginType_value)
	proto.RegisterEnum("ProtoMsg.EmDeviceType", EmDeviceType_name, EmDeviceType_value)
}

func init() { proto.RegisterFile("login.proto", fileDescriptor5) }

var fileDescriptor5 = []byte{
	// 476 bytes of a gzipped FileDescriptorProto
	0x1f, 0x8b, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0xff, 0x8c, 0x53, 0x4d, 0x73, 0xda, 0x30,
	0x10, 0xad, 0xf9, 0x08, 0xb0, 0x84, 0x56, 0x51, 0x02, 0x71, 0xd3, 0x43, 0x32, 0xee, 0x25, 0x93,
	0x03, 0x07, 0xda, 0x99, 0x1c, 0x7a, 0x4b, 0xb9, 0xb4, 0x93, 0x64, 0x18, 0x37, 0x77, 0x8f, 0xb0,
	0x16, 0xaa, 0xa9, 0x91, 0x8c, 0x64, 0xcc, 0xf0, 0x3b, 0x7a, 0xea, 0xa5, 0xbf, 0xb5, 0x23, 0x39,
	0x80, 0x71, 0x73, 0xc8, 0xc9, 0x4f, 0xef, 0xed, 0x7b, 0x5e, 0xed, 0xda, 0xd0, 0x4d, 0xd4, 0x5c,
	0xc8, 0x61, 0xaa, 0x55, 0xa6, 0x68, 0x7b, 0x62, 0x1f, 0x0f, 0x66, 0x7e, 0xd1, 0x8b, 0x93, 0x95,
	0xc9, 0x50, 0x17, 0xc2, 0x05, 0x4c, 0x99, 0xc1, 0x02, 0x07, 0x8f, 0xe0, 0x4f, 0xa6, 0x3f, 0x72,
	0x1d, 0xe2, 0x5c, 0xd8, 0x92, 0x7b, 0x1b, 0x10, 0xe2, 0xf2, 0xc1, 0xcc, 0xe9, 0x08, 0xda, 0x26,
	0xd7, 0x91, 0x90, 0x33, 0xe5, 0x7b, 0x57, 0xde, 0x75, 0x77, 0x74, 0x3e, 0xdc, 0x66, 0x0e, 0x9d,
	0xeb, 0x8e, 0x19, 0xfc, 0x26, 0x67, 0x2a, 0x6c, 0x99, 0x5c, 0x5b, 0x10, 0x7c, 0x79, 0x39, 0xcf,
	0xd8, 0xbc, 0x4b, 0xa8, 0x6b, 0xcc, 0x9e, 0xa3, 0x7a, 0xfb, 0xa8, 0x10, 0xb3, 0xd0, 0x2a, 0xc1,
	0xdf, 0x1a, 0xf4, 0x27, 0xd3, 0xaf, 0x66, 0x92, 0xb0, 0xcd, 0x61, 0x2b, 0x9f, 0x01, 0xdc, 0xd5,
	0xa2, 0x6c, 0x93, 0xa2, 0x4b, 0x78, 0x3b, 0xea, 0xef, 0x13, 0x70, 0xe1, 0x8a, 0x9f, 0x36, 0x29,
	0x86, 0x9d, 0x64, 0x0b, 0xe9, 0x2d, 0x74, 0x39, 0xe6, 0x22, 0xc6, 0xc2, 0x56, 0x73, 0xb6, 0x41,
	0xd9, 0x36, 0x76, 0xb2, 0xf3, 0x01, 0xdf, 0x61, 0x7a, 0xb9, 0x33, 0x72, 0x34, 0xb1, 0x5f, 0xbf,
	0xf2, 0xae, 0x3b, 0xdb, 0x82, 0x31, 0x9a, 0x98, 0xfa, 0xd0, 0xca, 0x51, 0x1b, 0xa1, 0xa4, 0xdf,
	0x70, 0xe2, 0xf6, 0x48, 0x3f, 0x42, 0x2f, 0x4d, 0x58, 0x36, 0x53, 0x8b, 0x28, 0x53, 0xbf, 0x50,
	0xfa, 0x4d, 0xa7, 0x1f, 0x3f, 0x93, 0x4f, 0x96, 0xa3, 0x1f, 0xa0, 0xb3, 0x32, 0xa8, 0x23, 0xc9,
	0x16, 0xe8, 0x1f, 0xb9, 0x82, 0xb6, 0x25, 0x1e, 0xd9, 0x02, 0xe9, 0x7b, 0x70, 0x38, 0x4a, 0xd7,
	0xdc, 0x6f, 0x15, 0xe1, 0xf6, 0x3c, 0x59, 0xf3, 0xe0, 0xfb, 0x0b, 0xf3, 0x79, 0xd5, 0x68, 0x29,
	0x81, 0xfa, 0x4a, 0x70, 0x37, 0x82, 0x46, 0x68, 0x61, 0x30, 0x80, 0xb3, 0x7d, 0xd6, 0x9d, 0x90,
	0xbc, 0x18, 0x75, 0x70, 0xfb, 0x3f, 0xff, 0xaa, 0x57, 0xdc, 0xfc, 0xf6, 0xa0, 0x5b, 0x5a, 0x04,
	0x1d, 0x00, 0xbd, 0xdf, 0xed, 0x2c, 0x12, 0x32, 0x67, 0x89, 0xe0, 0xe4, 0x4d, 0x85, 0xcf, 0x85,
	0x11, 0x99, 0xd2, 0xc4, 0xa3, 0xa7, 0xf0, 0xae, 0xc4, 0xdb, 0x2b, 0x93, 0x1a, 0x3d, 0x81, 0x5e,
	0x89, 0x5c, 0x2e, 0x49, 0x9d, 0xf6, 0xe1, 0xa4, 0x44, 0xad, 0x31, 0xfe, 0xc9, 0x32, 0xd2, 0xa8,
	0xd0, 0x2c, 0x11, 0x29, 0xdb, 0x90, 0xe6, 0xcd, 0x1f, 0x0f, 0x8e, 0xcb, 0x7b, 0xa6, 0xe7, 0x70,
	0x3a, 0xde, 0x7f, 0x14, 0xa5, 0xbe, 0x2a, 0x02, 0x93, 0x5c, 0x2b, 0xc1, 0x8b, 0xc6, 0x0e, 0x1c,
	0xca, 0x90, 0x1a, 0x3d, 0x03, 0x72, 0x40, 0xa6, 0x8c, 0x93, 0x7a, 0xb5, 0x74, 0xc1, 0x62, 0xd2,
	0xa8, 0x06, 0xaf, 0x85, 0xe4, 0x6a, 0x6d, 0x48, 0x73, 0x7a, 0xe4, 0xfe, 0xc1, 0x4f, 0xff, 0x02,
	0x00, 0x00, 0xff, 0xff, 0xc4, 0x8b, 0x5d, 0xcb, 0xb7, 0x03, 0x00, 0x00,
}
