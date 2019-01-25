// Code generated by protoc-gen-go.
// source: world.proto
// DO NOT EDIT!

package ProtoMsg

import proto "github.com/golang/protobuf/proto"
import fmt "fmt"
import math "math"

// Reference imports to suppress errors if they are not otherwise used.
var _ = proto.Marshal
var _ = fmt.Errorf
var _ = math.Inf

// SVR_MSG_REGISTER_WORLD
type PbSvrRegisterWorldReqMsg struct {
	SvrInfo *PbSvrBaseInfo `protobuf:"bytes,1,opt,name=svr_info,json=svrInfo" json:"svr_info,omitempty"`
}

func (m *PbSvrRegisterWorldReqMsg) Reset()                    { *m = PbSvrRegisterWorldReqMsg{} }
func (m *PbSvrRegisterWorldReqMsg) String() string            { return proto.CompactTextString(m) }
func (*PbSvrRegisterWorldReqMsg) ProtoMessage()               {}
func (*PbSvrRegisterWorldReqMsg) Descriptor() ([]byte, []int) { return fileDescriptor5, []int{0} }

func (m *PbSvrRegisterWorldReqMsg) GetSvrInfo() *PbSvrBaseInfo {
	if m != nil {
		return m.SvrInfo
	}
	return nil
}

type PbSvrRegisterWorldResMsg struct {
	Ret *Ret `protobuf:"bytes,1,opt,name=ret" json:"ret,omitempty"`
}

func (m *PbSvrRegisterWorldResMsg) Reset()                    { *m = PbSvrRegisterWorldResMsg{} }
func (m *PbSvrRegisterWorldResMsg) String() string            { return proto.CompactTextString(m) }
func (*PbSvrRegisterWorldResMsg) ProtoMessage()               {}
func (*PbSvrRegisterWorldResMsg) Descriptor() ([]byte, []int) { return fileDescriptor5, []int{1} }

func (m *PbSvrRegisterWorldResMsg) GetRet() *Ret {
	if m != nil {
		return m.Ret
	}
	return nil
}

func init() {
	proto.RegisterType((*PbSvrRegisterWorldReqMsg)(nil), "ProtoMsg.PbSvrRegisterWorldReqMsg")
	proto.RegisterType((*PbSvrRegisterWorldResMsg)(nil), "ProtoMsg.PbSvrRegisterWorldResMsg")
}

func init() { proto.RegisterFile("world.proto", fileDescriptor5) }

var fileDescriptor5 = []byte{
	// 158 bytes of a gzipped FileDescriptorProto
	0x1f, 0x8b, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0xff, 0xe2, 0xe2, 0x2e, 0xcf, 0x2f, 0xca,
	0x49, 0xd1, 0x2b, 0x28, 0xca, 0x2f, 0xc9, 0x17, 0xe2, 0x08, 0x00, 0x51, 0xbe, 0xc5, 0xe9, 0x52,
	0xbc, 0xc9, 0x39, 0xa5, 0xc5, 0x25, 0xa9, 0x45, 0x10, 0x09, 0x29, 0xae, 0xa4, 0xc4, 0xe2, 0x54,
	0x08, 0x5b, 0xc9, 0x8f, 0x4b, 0x22, 0x20, 0x29, 0xb8, 0xac, 0x28, 0x28, 0x35, 0x3d, 0x13, 0xa4,
	0x24, 0x1c, 0x64, 0x40, 0x50, 0x6a, 0xa1, 0x6f, 0x71, 0xba, 0x90, 0x11, 0x17, 0x47, 0x71, 0x59,
	0x51, 0x7c, 0x66, 0x5e, 0x5a, 0xbe, 0x04, 0xa3, 0x02, 0xa3, 0x06, 0xb7, 0x91, 0xb8, 0x1e, 0xcc,
	0x4c, 0x3d, 0xb0, 0x2e, 0xa7, 0xc4, 0xe2, 0x54, 0xcf, 0xbc, 0xb4, 0xfc, 0x20, 0xf6, 0xe2, 0xb2,
	0x22, 0x10, 0x43, 0xc9, 0x1a, 0xbb, 0x79, 0xc5, 0x20, 0xf3, 0xe4, 0xb9, 0x98, 0x8b, 0x52, 0x4b,
	0xa0, 0x46, 0xf1, 0x22, 0x8c, 0x0a, 0x4a, 0x2d, 0x09, 0x02, 0xc9, 0x24, 0xb1, 0x81, 0xdd, 0x64,
	0x0c, 0x08, 0x00, 0x00, 0xff, 0xff, 0x09, 0x99, 0xbc, 0x37, 0xc7, 0x00, 0x00, 0x00,
}
