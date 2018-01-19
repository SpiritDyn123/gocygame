// Code generated by protoc-gen-go. DO NOT EDIT.
// source: hw.proto

/*
Package hellworld is a generated protocol buffer package.

It is generated from these files:
	hw.proto

It has these top-level messages:
	RpcTestRequest
	RpcHBRequest
	RpcHBResponse
	RpcTestResponse
*/
package hellworld

import proto "github.com/golang/protobuf/proto"
import fmt "fmt"
import math "math"
import google_protobuf "github.com/golang/protobuf/protoc-gen-go/descriptor"

import (
	context "golang.org/x/net/context"
	grpc "google.golang.org/grpc"
)

// Reference imports to suppress errors if they are not otherwise used.
var _ = proto.Marshal
var _ = fmt.Errorf
var _ = math.Inf

// This is a compile-time assertion to ensure that this generated file
// is compatible with the proto package it is being compiled against.
// A compilation error at this line likely means your copy of the
// proto package needs to be updated.
const _ = proto.ProtoPackageIsVersion2 // please upgrade the proto package

type RpcTestRequest struct {
	Id   int32  `protobuf:"varint,1,opt,name=id" json:"id,omitempty"`
	Data string `protobuf:"bytes,2,opt,name=data" json:"data,omitempty"`
}

func (m *RpcTestRequest) Reset()                    { *m = RpcTestRequest{} }
func (m *RpcTestRequest) String() string            { return proto.CompactTextString(m) }
func (*RpcTestRequest) ProtoMessage()               {}
func (*RpcTestRequest) Descriptor() ([]byte, []int) { return fileDescriptor0, []int{0} }

func (m *RpcTestRequest) GetId() int32 {
	if m != nil {
		return m.Id
	}
	return 0
}

func (m *RpcTestRequest) GetData() string {
	if m != nil {
		return m.Data
	}
	return ""
}

type RpcHBRequest struct {
}

func (m *RpcHBRequest) Reset()                    { *m = RpcHBRequest{} }
func (m *RpcHBRequest) String() string            { return proto.CompactTextString(m) }
func (*RpcHBRequest) ProtoMessage()               {}
func (*RpcHBRequest) Descriptor() ([]byte, []int) { return fileDescriptor0, []int{1} }

type RpcHBResponse struct {
}

func (m *RpcHBResponse) Reset()                    { *m = RpcHBResponse{} }
func (m *RpcHBResponse) String() string            { return proto.CompactTextString(m) }
func (*RpcHBResponse) ProtoMessage()               {}
func (*RpcHBResponse) Descriptor() ([]byte, []int) { return fileDescriptor0, []int{2} }

type RpcTestResponse struct {
	Code    int32  `protobuf:"varint,1,opt,name=code" json:"code,omitempty"`
	Message string `protobuf:"bytes,2,opt,name=message" json:"message,omitempty"`
}

func (m *RpcTestResponse) Reset()                    { *m = RpcTestResponse{} }
func (m *RpcTestResponse) String() string            { return proto.CompactTextString(m) }
func (*RpcTestResponse) ProtoMessage()               {}
func (*RpcTestResponse) Descriptor() ([]byte, []int) { return fileDescriptor0, []int{3} }

func (m *RpcTestResponse) GetCode() int32 {
	if m != nil {
		return m.Code
	}
	return 0
}

func (m *RpcTestResponse) GetMessage() string {
	if m != nil {
		return m.Message
	}
	return ""
}

var E_MyOption = &proto.ExtensionDesc{
	ExtendedType:  (*google_protobuf.MessageOptions)(nil),
	ExtensionType: (*string)(nil),
	Field:         51234,
	Name:          "hellworld.my_option",
	Tag:           "bytes,51234,opt,name=my_option,json=myOption",
	Filename:      "hw.proto",
}

func init() {
	proto.RegisterType((*RpcTestRequest)(nil), "hellworld.RpcTestRequest")
	proto.RegisterType((*RpcHBRequest)(nil), "hellworld.RpcHBRequest")
	proto.RegisterType((*RpcHBResponse)(nil), "hellworld.RpcHBResponse")
	proto.RegisterType((*RpcTestResponse)(nil), "hellworld.RpcTestResponse")
	proto.RegisterExtension(E_MyOption)
}

// Reference imports to suppress errors if they are not otherwise used.
var _ context.Context
var _ grpc.ClientConn

// This is a compile-time assertion to ensure that this generated file
// is compatible with the grpc package it is being compiled against.
const _ = grpc.SupportPackageIsVersion4

// Client API for RpcTestService service

type RpcTestServiceClient interface {
	Hello(ctx context.Context, in *RpcTestRequest, opts ...grpc.CallOption) (*RpcTestResponse, error)
	HB(ctx context.Context, opts ...grpc.CallOption) (RpcTestService_HBClient, error)
}

type rpcTestServiceClient struct {
	cc *grpc.ClientConn
}

func NewRpcTestServiceClient(cc *grpc.ClientConn) RpcTestServiceClient {
	return &rpcTestServiceClient{cc}
}

func (c *rpcTestServiceClient) Hello(ctx context.Context, in *RpcTestRequest, opts ...grpc.CallOption) (*RpcTestResponse, error) {
	out := new(RpcTestResponse)
	err := grpc.Invoke(ctx, "/hellworld.RpcTestService/hello", in, out, c.cc, opts...)
	if err != nil {
		return nil, err
	}
	return out, nil
}

func (c *rpcTestServiceClient) HB(ctx context.Context, opts ...grpc.CallOption) (RpcTestService_HBClient, error) {
	stream, err := grpc.NewClientStream(ctx, &_RpcTestService_serviceDesc.Streams[0], c.cc, "/hellworld.RpcTestService/HB", opts...)
	if err != nil {
		return nil, err
	}
	x := &rpcTestServiceHBClient{stream}
	return x, nil
}

type RpcTestService_HBClient interface {
	Send(*RpcHBRequest) error
	Recv() (*RpcHBResponse, error)
	grpc.ClientStream
}

type rpcTestServiceHBClient struct {
	grpc.ClientStream
}

func (x *rpcTestServiceHBClient) Send(m *RpcHBRequest) error {
	return x.ClientStream.SendMsg(m)
}

func (x *rpcTestServiceHBClient) Recv() (*RpcHBResponse, error) {
	m := new(RpcHBResponse)
	if err := x.ClientStream.RecvMsg(m); err != nil {
		return nil, err
	}
	return m, nil
}

// Server API for RpcTestService service

type RpcTestServiceServer interface {
	Hello(context.Context, *RpcTestRequest) (*RpcTestResponse, error)
	HB(RpcTestService_HBServer) error
}

func RegisterRpcTestServiceServer(s *grpc.Server, srv RpcTestServiceServer) {
	s.RegisterService(&_RpcTestService_serviceDesc, srv)
}

func _RpcTestService_Hello_Handler(srv interface{}, ctx context.Context, dec func(interface{}) error, interceptor grpc.UnaryServerInterceptor) (interface{}, error) {
	in := new(RpcTestRequest)
	if err := dec(in); err != nil {
		return nil, err
	}
	if interceptor == nil {
		return srv.(RpcTestServiceServer).Hello(ctx, in)
	}
	info := &grpc.UnaryServerInfo{
		Server:     srv,
		FullMethod: "/hellworld.RpcTestService/Hello",
	}
	handler := func(ctx context.Context, req interface{}) (interface{}, error) {
		return srv.(RpcTestServiceServer).Hello(ctx, req.(*RpcTestRequest))
	}
	return interceptor(ctx, in, info, handler)
}

func _RpcTestService_HB_Handler(srv interface{}, stream grpc.ServerStream) error {
	return srv.(RpcTestServiceServer).HB(&rpcTestServiceHBServer{stream})
}

type RpcTestService_HBServer interface {
	Send(*RpcHBResponse) error
	Recv() (*RpcHBRequest, error)
	grpc.ServerStream
}

type rpcTestServiceHBServer struct {
	grpc.ServerStream
}

func (x *rpcTestServiceHBServer) Send(m *RpcHBResponse) error {
	return x.ServerStream.SendMsg(m)
}

func (x *rpcTestServiceHBServer) Recv() (*RpcHBRequest, error) {
	m := new(RpcHBRequest)
	if err := x.ServerStream.RecvMsg(m); err != nil {
		return nil, err
	}
	return m, nil
}

var _RpcTestService_serviceDesc = grpc.ServiceDesc{
	ServiceName: "hellworld.RpcTestService",
	HandlerType: (*RpcTestServiceServer)(nil),
	Methods: []grpc.MethodDesc{
		{
			MethodName: "hello",
			Handler:    _RpcTestService_Hello_Handler,
		},
	},
	Streams: []grpc.StreamDesc{
		{
			StreamName:    "HB",
			Handler:       _RpcTestService_HB_Handler,
			ServerStreams: true,
			ClientStreams: true,
		},
	},
	Metadata: "hw.proto",
}

func init() { proto.RegisterFile("hw.proto", fileDescriptor0) }

var fileDescriptor0 = []byte{
	// 281 bytes of a gzipped FileDescriptorProto
	0x1f, 0x8b, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0xff, 0x6c, 0x90, 0xc1, 0x4a, 0xc3, 0x40,
	0x10, 0x86, 0xdd, 0x68, 0x6d, 0x3b, 0x6a, 0x2a, 0x73, 0x31, 0xcd, 0xc5, 0x92, 0x53, 0x4e, 0x41,
	0xd4, 0x53, 0x41, 0x91, 0x9e, 0x7a, 0x50, 0x84, 0xe8, 0x5d, 0xd2, 0xec, 0x98, 0x06, 0x92, 0xce,
	0x9a, 0xdd, 0x5a, 0x7a, 0xf5, 0x09, 0xd4, 0x47, 0xf0, 0x49, 0xa5, 0xd9, 0x34, 0x50, 0x74, 0x4f,
	0x33, 0xf3, 0xcf, 0xfc, 0xfc, 0xdf, 0x42, 0x6f, 0xbe, 0x8a, 0x54, 0xc5, 0x86, 0xb1, 0x3f, 0xa7,
	0xa2, 0x58, 0x71, 0x55, 0x48, 0xff, 0x54, 0x92, 0x4e, 0xab, 0x5c, 0x19, 0xae, 0xac, 0x18, 0x5c,
	0x83, 0x1b, 0xab, 0xf4, 0x99, 0xb4, 0x89, 0xe9, 0x6d, 0x49, 0xda, 0xa0, 0x0b, 0x4e, 0x2e, 0x3d,
	0x31, 0x12, 0x61, 0x27, 0x76, 0x72, 0x89, 0x08, 0x07, 0x32, 0x31, 0x89, 0xe7, 0x8c, 0x44, 0xd8,
	0x8f, 0xeb, 0x3a, 0x70, 0xe1, 0x38, 0x56, 0xe9, 0x74, 0xd2, 0xdc, 0x04, 0x03, 0x38, 0x69, 0x7a,
	0xad, 0x78, 0xa1, 0x29, 0xb8, 0x87, 0x41, 0x6b, 0x6b, 0x47, 0x1b, 0x9f, 0x94, 0x25, 0x35, 0xce,
	0x75, 0x8d, 0x1e, 0x74, 0x4b, 0xd2, 0x3a, 0xc9, 0xa8, 0xb1, 0xdf, 0xb6, 0xe3, 0xa3, 0xef, 0x8f,
	0x61, 0x57, 0xdb, 0x77, 0xf9, 0x25, 0xda, 0x94, 0x4f, 0x54, 0xbd, 0xe7, 0x29, 0xe1, 0x1d, 0x74,
	0x36, 0x58, 0x8c, 0xc3, 0xa8, 0xc5, 0x8b, 0x76, 0x49, 0x7c, 0xff, 0x3f, 0xa9, 0x09, 0xb8, 0x87,
	0x37, 0xe0, 0x4c, 0x27, 0x78, 0xb6, 0xbb, 0xd3, 0x22, 0xf9, 0xde, 0x5f, 0x61, 0x7b, 0x1a, 0x8a,
	0x0b, 0x31, 0xbe, 0x85, 0x7e, 0xb9, 0x7e, 0x61, 0x65, 0x72, 0x5e, 0xe0, 0x79, 0x94, 0x31, 0x67,
	0x05, 0xd9, 0x4f, 0x9d, 0x2d, 0x5f, 0xa3, 0x07, 0xcb, 0xf1, 0x58, 0xeb, 0xda, 0xfb, 0xf9, 0xdc,
	0xaf, 0xf9, 0x7a, 0xe5, 0xda, 0x8e, 0x66, 0x87, 0xf5, 0xea, 0xd5, 0x6f, 0x00, 0x00, 0x00, 0xff,
	0xff, 0x19, 0x8a, 0xcf, 0x3b, 0xa8, 0x01, 0x00, 0x00,
}
