# Generated by the protocol buffer compiler.  DO NOT EDIT!
# source: zhibiao.proto

import sys
_b=sys.version_info[0]<3 and (lambda x:x) or (lambda x:x.encode('latin1'))
from google.protobuf.internal import enum_type_wrapper
from google.protobuf import descriptor as _descriptor
from google.protobuf import message as _message
from google.protobuf import reflection as _reflection
from google.protobuf import symbol_database as _symbol_database
from google.protobuf import descriptor_pb2
# @@protoc_insertion_point(imports)

_sym_db = _symbol_database.Default()


import ret_base_pb2 as ret__base__pb2
import public_message_pb2 as public__message__pb2


DESCRIPTOR = _descriptor.FileDescriptor(
  name='zhibiao.proto',
  package='QuoteProto',
  syntax='proto2',
  serialized_pb=_b('\n\rzhibiao.proto\x12\nQuoteProto\x1a\x0eret_base.proto\x1a\x14public_message.proto\"3\n\tDotResult\x12\x17\n\x0fupserttime_time\x18\x01 \x02(\x05\x12\r\n\x05stype\x18\x02 \x02(\x05\"V\n\x0c\x43olumnResult\x12\x12\n\ncolor_type\x18\x01 \x02(\x05\x12\x17\n\x0fupserttime_time\x18\x02 \x02(\x05\x12\x0c\n\x04high\x18\x03 \x02(\x05\x12\x0b\n\x03low\x18\x04 \x01(\x05\"u\n\x11GetZhibiaoRequest\x12\x12\n\nstock_code\x18\x01 \x02(\t\x12&\n\x05ztype\x18\x02 \x02(\x0e\x32\x17.QuoteProto.ZhibiaoType\x12\x12\n\nstart_time\x18\x03 \x02(\x05\x12\x10\n\x08\x65nd_time\x18\x04 \x02(\x05\"\xaf\x01\n\x12GetZhibiaoResponse\x12\x1c\n\x03ret\x18\x01 \x02(\x0b\x32\x0f.QuoteProto.Ret\x12&\n\x05ztype\x18\x02 \x02(\x0e\x32\x17.QuoteProto.ZhibiaoType\x12\'\n\x08\x64ot_info\x18\x03 \x03(\x0b\x32\x15.QuoteProto.DotResult\x12*\n\x08\x63ol_info\x18\x04 \x03(\x0b\x32\x18.QuoteProto.ColumnResult*z\n\x0bZhibiaoType\x12\x08\n\x04JDCJ\x10\x01\x12\x08\n\x04\x43MFB\x10\x02\x12\x08\n\x04QSGD\x10\x03\x12\x08\n\x04HLQJ\x10\x04\x12\x08\n\x04ZJKP\x10\x05\x12\x08\n\x04\x43PTX\x10\x06\x12\x07\n\x03HPM\x10\x07\x12\x08\n\x04XQQX\x10\x08\x12\x08\n\x04\x44QKJ\x10\t\x12\x08\n\x04\x44\x42GJ\x10\n\x12\x08\n\x04\x44\x42\x44J\x10\x0b')
  ,
  dependencies=[ret__base__pb2.DESCRIPTOR,public__message__pb2.DESCRIPTOR,])

_ZHIBIAOTYPE = _descriptor.EnumDescriptor(
  name='ZhibiaoType',
  full_name='QuoteProto.ZhibiaoType',
  filename=None,
  file=DESCRIPTOR,
  values=[
    _descriptor.EnumValueDescriptor(
      name='JDCJ', index=0, number=1,
      options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='CMFB', index=1, number=2,
      options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='QSGD', index=2, number=3,
      options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='HLQJ', index=3, number=4,
      options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='ZJKP', index=4, number=5,
      options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='CPTX', index=5, number=6,
      options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='HPM', index=6, number=7,
      options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='XQQX', index=7, number=8,
      options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='DQKJ', index=8, number=9,
      options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='DBGJ', index=9, number=10,
      options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='DBDJ', index=10, number=11,
      options=None,
      type=None),
  ],
  containing_type=None,
  options=None,
  serialized_start=505,
  serialized_end=627,
)
_sym_db.RegisterEnumDescriptor(_ZHIBIAOTYPE)

ZhibiaoType = enum_type_wrapper.EnumTypeWrapper(_ZHIBIAOTYPE)
JDCJ = 1
CMFB = 2
QSGD = 3
HLQJ = 4
ZJKP = 5
CPTX = 6
HPM = 7
XQQX = 8
DQKJ = 9
DBGJ = 10
DBDJ = 11



_DOTRESULT = _descriptor.Descriptor(
  name='DotResult',
  full_name='QuoteProto.DotResult',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
    _descriptor.FieldDescriptor(
      name='upserttime_time', full_name='QuoteProto.DotResult.upserttime_time', index=0,
      number=1, type=5, cpp_type=1, label=2,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='stype', full_name='QuoteProto.DotResult.stype', index=1,
      number=2, type=5, cpp_type=1, label=2,
      has_default_value=False, default_value=0,
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
  syntax='proto2',
  extension_ranges=[],
  oneofs=[
  ],
  serialized_start=67,
  serialized_end=118,
)


_COLUMNRESULT = _descriptor.Descriptor(
  name='ColumnResult',
  full_name='QuoteProto.ColumnResult',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
    _descriptor.FieldDescriptor(
      name='color_type', full_name='QuoteProto.ColumnResult.color_type', index=0,
      number=1, type=5, cpp_type=1, label=2,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='upserttime_time', full_name='QuoteProto.ColumnResult.upserttime_time', index=1,
      number=2, type=5, cpp_type=1, label=2,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='high', full_name='QuoteProto.ColumnResult.high', index=2,
      number=3, type=5, cpp_type=1, label=2,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='low', full_name='QuoteProto.ColumnResult.low', index=3,
      number=4, type=5, cpp_type=1, label=1,
      has_default_value=False, default_value=0,
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
  syntax='proto2',
  extension_ranges=[],
  oneofs=[
  ],
  serialized_start=120,
  serialized_end=206,
)


_GETZHIBIAOREQUEST = _descriptor.Descriptor(
  name='GetZhibiaoRequest',
  full_name='QuoteProto.GetZhibiaoRequest',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
    _descriptor.FieldDescriptor(
      name='stock_code', full_name='QuoteProto.GetZhibiaoRequest.stock_code', index=0,
      number=1, type=9, cpp_type=9, label=2,
      has_default_value=False, default_value=_b("").decode('utf-8'),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='ztype', full_name='QuoteProto.GetZhibiaoRequest.ztype', index=1,
      number=2, type=14, cpp_type=8, label=2,
      has_default_value=False, default_value=1,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='start_time', full_name='QuoteProto.GetZhibiaoRequest.start_time', index=2,
      number=3, type=5, cpp_type=1, label=2,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='end_time', full_name='QuoteProto.GetZhibiaoRequest.end_time', index=3,
      number=4, type=5, cpp_type=1, label=2,
      has_default_value=False, default_value=0,
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
  syntax='proto2',
  extension_ranges=[],
  oneofs=[
  ],
  serialized_start=208,
  serialized_end=325,
)


_GETZHIBIAORESPONSE = _descriptor.Descriptor(
  name='GetZhibiaoResponse',
  full_name='QuoteProto.GetZhibiaoResponse',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
    _descriptor.FieldDescriptor(
      name='ret', full_name='QuoteProto.GetZhibiaoResponse.ret', index=0,
      number=1, type=11, cpp_type=10, label=2,
      has_default_value=False, default_value=None,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='ztype', full_name='QuoteProto.GetZhibiaoResponse.ztype', index=1,
      number=2, type=14, cpp_type=8, label=2,
      has_default_value=False, default_value=1,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='dot_info', full_name='QuoteProto.GetZhibiaoResponse.dot_info', index=2,
      number=3, type=11, cpp_type=10, label=3,
      has_default_value=False, default_value=[],
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='col_info', full_name='QuoteProto.GetZhibiaoResponse.col_info', index=3,
      number=4, type=11, cpp_type=10, label=3,
      has_default_value=False, default_value=[],
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
  syntax='proto2',
  extension_ranges=[],
  oneofs=[
  ],
  serialized_start=328,
  serialized_end=503,
)

_GETZHIBIAOREQUEST.fields_by_name['ztype'].enum_type = _ZHIBIAOTYPE
_GETZHIBIAORESPONSE.fields_by_name['ret'].message_type = ret__base__pb2._RET
_GETZHIBIAORESPONSE.fields_by_name['ztype'].enum_type = _ZHIBIAOTYPE
_GETZHIBIAORESPONSE.fields_by_name['dot_info'].message_type = _DOTRESULT
_GETZHIBIAORESPONSE.fields_by_name['col_info'].message_type = _COLUMNRESULT
DESCRIPTOR.message_types_by_name['DotResult'] = _DOTRESULT
DESCRIPTOR.message_types_by_name['ColumnResult'] = _COLUMNRESULT
DESCRIPTOR.message_types_by_name['GetZhibiaoRequest'] = _GETZHIBIAOREQUEST
DESCRIPTOR.message_types_by_name['GetZhibiaoResponse'] = _GETZHIBIAORESPONSE
DESCRIPTOR.enum_types_by_name['ZhibiaoType'] = _ZHIBIAOTYPE
_sym_db.RegisterFileDescriptor(DESCRIPTOR)

DotResult = _reflection.GeneratedProtocolMessageType('DotResult', (_message.Message,), dict(
  DESCRIPTOR = _DOTRESULT,
  __module__ = 'zhibiao_pb2'
  # @@protoc_insertion_point(class_scope:QuoteProto.DotResult)
  ))
_sym_db.RegisterMessage(DotResult)

ColumnResult = _reflection.GeneratedProtocolMessageType('ColumnResult', (_message.Message,), dict(
  DESCRIPTOR = _COLUMNRESULT,
  __module__ = 'zhibiao_pb2'
  # @@protoc_insertion_point(class_scope:QuoteProto.ColumnResult)
  ))
_sym_db.RegisterMessage(ColumnResult)

GetZhibiaoRequest = _reflection.GeneratedProtocolMessageType('GetZhibiaoRequest', (_message.Message,), dict(
  DESCRIPTOR = _GETZHIBIAOREQUEST,
  __module__ = 'zhibiao_pb2'
  # @@protoc_insertion_point(class_scope:QuoteProto.GetZhibiaoRequest)
  ))
_sym_db.RegisterMessage(GetZhibiaoRequest)

GetZhibiaoResponse = _reflection.GeneratedProtocolMessageType('GetZhibiaoResponse', (_message.Message,), dict(
  DESCRIPTOR = _GETZHIBIAORESPONSE,
  __module__ = 'zhibiao_pb2'
  # @@protoc_insertion_point(class_scope:QuoteProto.GetZhibiaoResponse)
  ))
_sym_db.RegisterMessage(GetZhibiaoResponse)


# @@protoc_insertion_point(module_scope)
