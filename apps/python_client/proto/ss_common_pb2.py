# Generated by the protocol buffer compiler.  DO NOT EDIT!
# source: ss_common.proto

import sys
_b=sys.version_info[0]<3 and (lambda x:x) or (lambda x:x.encode('latin1'))
from google.protobuf import descriptor as _descriptor
from google.protobuf import message as _message
from google.protobuf import reflection as _reflection
from google.protobuf import symbol_database as _symbol_database
from google.protobuf import descriptor_pb2
# @@protoc_insertion_point(imports)

_sym_db = _symbol_database.Default()


import cluster_pb2 as cluster__pb2


DESCRIPTOR = _descriptor.FileDescriptor(
  name='ss_common.proto',
  package='ProtoMsg',
  syntax='proto3',
  serialized_pb=_b('\n\x0fss_common.proto\x12\x08ProtoMsg\x1a\rcluster.proto\"m\n\x1aPbSvrCommonPushPlayerSvrId\x12\x0b\n\x03uid\x18\x01 \x01(\x04\x12%\n\x08svr_type\x18\x02 \x01(\x0e\x32\x13.ProtoMsg.emSvrType\x12\x0e\n\x06svr_id\x18\x03 \x01(\x05\x12\x0b\n\x03\x61\x64\x64\x18\x04 \x01(\x08\x62\x06proto3')
  ,
  dependencies=[cluster__pb2.DESCRIPTOR,])




_PBSVRCOMMONPUSHPLAYERSVRID = _descriptor.Descriptor(
  name='PbSvrCommonPushPlayerSvrId',
  full_name='ProtoMsg.PbSvrCommonPushPlayerSvrId',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
    _descriptor.FieldDescriptor(
      name='uid', full_name='ProtoMsg.PbSvrCommonPushPlayerSvrId.uid', index=0,
      number=1, type=4, cpp_type=4, label=1,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='svr_type', full_name='ProtoMsg.PbSvrCommonPushPlayerSvrId.svr_type', index=1,
      number=2, type=14, cpp_type=8, label=1,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='svr_id', full_name='ProtoMsg.PbSvrCommonPushPlayerSvrId.svr_id', index=2,
      number=3, type=5, cpp_type=1, label=1,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='add', full_name='ProtoMsg.PbSvrCommonPushPlayerSvrId.add', index=3,
      number=4, type=8, cpp_type=7, label=1,
      has_default_value=False, default_value=False,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None, file=DESCRIPTOR),
  ],
  extensions=[
  ],
  nested_types=[],
  enum_types=[
  ],
  options=None,
  is_extendable=False,
  syntax='proto3',
  extension_ranges=[],
  oneofs=[
  ],
  serialized_start=44,
  serialized_end=153,
)

_PBSVRCOMMONPUSHPLAYERSVRID.fields_by_name['svr_type'].enum_type = cluster__pb2._EMSVRTYPE
DESCRIPTOR.message_types_by_name['PbSvrCommonPushPlayerSvrId'] = _PBSVRCOMMONPUSHPLAYERSVRID
_sym_db.RegisterFileDescriptor(DESCRIPTOR)

PbSvrCommonPushPlayerSvrId = _reflection.GeneratedProtocolMessageType('PbSvrCommonPushPlayerSvrId', (_message.Message,), dict(
  DESCRIPTOR = _PBSVRCOMMONPUSHPLAYERSVRID,
  __module__ = 'ss_common_pb2'
  # @@protoc_insertion_point(class_scope:ProtoMsg.PbSvrCommonPushPlayerSvrId)
  ))
_sym_db.RegisterMessage(PbSvrCommonPushPlayerSvrId)


# @@protoc_insertion_point(module_scope)