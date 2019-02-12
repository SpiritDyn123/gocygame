# Generated by the protocol buffer compiler.  DO NOT EDIT!
# source: zhubi.proto

import sys
_b=sys.version_info[0]<3 and (lambda x:x) or (lambda x:x.encode('latin1'))
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
  name='zhubi.proto',
  package='QuoteProto',
  syntax='proto2',
  serialized_pb=_b('\n\x0bzhubi.proto\x12\nQuoteProto\x1a\x0eret_base.proto\x1a\x14public_message.proto\"\x84\x01\n\x0fGetZhubiRequest\x12\x12\n\nstock_code\x18\x01 \x02(\t\x12\x13\n\x0bupdate_time\x18\x02 \x02(\x05\x12\r\n\x05\x63ount\x18\x03 \x02(\r\x12\x0f\n\x07is_desc\x18\x04 \x02(\x08\x12(\n\x07zb_type\x18\x05 \x01(\x0e\x32\x17.QuoteProto.emZhubiType\"\xaa\x01\n\x10GetZhubiResponse\x12\x1c\n\x03ret\x18\x01 \x02(\x0b\x32\x0f.QuoteProto.Ret\x12\x13\n\x0btotal_count\x18\x02 \x01(\r\x12%\n\nzhubi_info\x18\x03 \x03(\x0b\x32\x11.QuoteProto.Zhubi\x12\x12\n\nstock_code\x18\x04 \x01(\t\x12(\n\x07zb_type\x18\x05 \x01(\x0e\x32\x17.QuoteProto.emZhubiType\"D\n\x10GetFenJiaRequest\x12\x12\n\nstock_code\x18\x01 \x02(\t\x12\r\n\x05index\x18\x02 \x02(\x05\x12\r\n\x05\x63ount\x18\x03 \x02(\r\"\xc0\x01\n\x11GetFenJiaResponse\x12\x1c\n\x03ret\x18\x01 \x02(\x0b\x32\x0f.QuoteProto.Ret\x12\x13\n\x0btotal_count\x18\x02 \x01(\r\x12\'\n\x0b\x66\x65njia_info\x18\x03 \x03(\x0b\x32\x12.QuoteProto.FenJia\x12\x12\n\nstock_code\x18\x04 \x01(\t\x12\x0e\n\x06volume\x18\x05 \x01(\x04\x12\x12\n\nmax_volume\x18\x06 \x01(\x04\x12\x17\n\x0fpre_close_price\x18\x07 \x01(\x01')
  ,
  dependencies=[ret__base__pb2.DESCRIPTOR,public__message__pb2.DESCRIPTOR,])




_GETZHUBIREQUEST = _descriptor.Descriptor(
  name='GetZhubiRequest',
  full_name='QuoteProto.GetZhubiRequest',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
    _descriptor.FieldDescriptor(
      name='stock_code', full_name='QuoteProto.GetZhubiRequest.stock_code', index=0,
      number=1, type=9, cpp_type=9, label=2,
      has_default_value=False, default_value=_b("").decode('utf-8'),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='update_time', full_name='QuoteProto.GetZhubiRequest.update_time', index=1,
      number=2, type=5, cpp_type=1, label=2,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='count', full_name='QuoteProto.GetZhubiRequest.count', index=2,
      number=3, type=13, cpp_type=3, label=2,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='is_desc', full_name='QuoteProto.GetZhubiRequest.is_desc', index=3,
      number=4, type=8, cpp_type=7, label=2,
      has_default_value=False, default_value=False,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='zb_type', full_name='QuoteProto.GetZhubiRequest.zb_type', index=4,
      number=5, type=14, cpp_type=8, label=1,
      has_default_value=False, default_value=1,
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
  serialized_start=66,
  serialized_end=198,
)


_GETZHUBIRESPONSE = _descriptor.Descriptor(
  name='GetZhubiResponse',
  full_name='QuoteProto.GetZhubiResponse',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
    _descriptor.FieldDescriptor(
      name='ret', full_name='QuoteProto.GetZhubiResponse.ret', index=0,
      number=1, type=11, cpp_type=10, label=2,
      has_default_value=False, default_value=None,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='total_count', full_name='QuoteProto.GetZhubiResponse.total_count', index=1,
      number=2, type=13, cpp_type=3, label=1,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='zhubi_info', full_name='QuoteProto.GetZhubiResponse.zhubi_info', index=2,
      number=3, type=11, cpp_type=10, label=3,
      has_default_value=False, default_value=[],
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='stock_code', full_name='QuoteProto.GetZhubiResponse.stock_code', index=3,
      number=4, type=9, cpp_type=9, label=1,
      has_default_value=False, default_value=_b("").decode('utf-8'),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='zb_type', full_name='QuoteProto.GetZhubiResponse.zb_type', index=4,
      number=5, type=14, cpp_type=8, label=1,
      has_default_value=False, default_value=1,
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
  serialized_start=201,
  serialized_end=371,
)


_GETFENJIAREQUEST = _descriptor.Descriptor(
  name='GetFenJiaRequest',
  full_name='QuoteProto.GetFenJiaRequest',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
    _descriptor.FieldDescriptor(
      name='stock_code', full_name='QuoteProto.GetFenJiaRequest.stock_code', index=0,
      number=1, type=9, cpp_type=9, label=2,
      has_default_value=False, default_value=_b("").decode('utf-8'),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='index', full_name='QuoteProto.GetFenJiaRequest.index', index=1,
      number=2, type=5, cpp_type=1, label=2,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='count', full_name='QuoteProto.GetFenJiaRequest.count', index=2,
      number=3, type=13, cpp_type=3, label=2,
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
  serialized_start=373,
  serialized_end=441,
)


_GETFENJIARESPONSE = _descriptor.Descriptor(
  name='GetFenJiaResponse',
  full_name='QuoteProto.GetFenJiaResponse',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
    _descriptor.FieldDescriptor(
      name='ret', full_name='QuoteProto.GetFenJiaResponse.ret', index=0,
      number=1, type=11, cpp_type=10, label=2,
      has_default_value=False, default_value=None,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='total_count', full_name='QuoteProto.GetFenJiaResponse.total_count', index=1,
      number=2, type=13, cpp_type=3, label=1,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='fenjia_info', full_name='QuoteProto.GetFenJiaResponse.fenjia_info', index=2,
      number=3, type=11, cpp_type=10, label=3,
      has_default_value=False, default_value=[],
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='stock_code', full_name='QuoteProto.GetFenJiaResponse.stock_code', index=3,
      number=4, type=9, cpp_type=9, label=1,
      has_default_value=False, default_value=_b("").decode('utf-8'),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='volume', full_name='QuoteProto.GetFenJiaResponse.volume', index=4,
      number=5, type=4, cpp_type=4, label=1,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='max_volume', full_name='QuoteProto.GetFenJiaResponse.max_volume', index=5,
      number=6, type=4, cpp_type=4, label=1,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='pre_close_price', full_name='QuoteProto.GetFenJiaResponse.pre_close_price', index=6,
      number=7, type=1, cpp_type=5, label=1,
      has_default_value=False, default_value=float(0),
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
  serialized_start=444,
  serialized_end=636,
)

_GETZHUBIREQUEST.fields_by_name['zb_type'].enum_type = public__message__pb2._EMZHUBITYPE
_GETZHUBIRESPONSE.fields_by_name['ret'].message_type = ret__base__pb2._RET
_GETZHUBIRESPONSE.fields_by_name['zhubi_info'].message_type = public__message__pb2._ZHUBI
_GETZHUBIRESPONSE.fields_by_name['zb_type'].enum_type = public__message__pb2._EMZHUBITYPE
_GETFENJIARESPONSE.fields_by_name['ret'].message_type = ret__base__pb2._RET
_GETFENJIARESPONSE.fields_by_name['fenjia_info'].message_type = public__message__pb2._FENJIA
DESCRIPTOR.message_types_by_name['GetZhubiRequest'] = _GETZHUBIREQUEST
DESCRIPTOR.message_types_by_name['GetZhubiResponse'] = _GETZHUBIRESPONSE
DESCRIPTOR.message_types_by_name['GetFenJiaRequest'] = _GETFENJIAREQUEST
DESCRIPTOR.message_types_by_name['GetFenJiaResponse'] = _GETFENJIARESPONSE
_sym_db.RegisterFileDescriptor(DESCRIPTOR)

GetZhubiRequest = _reflection.GeneratedProtocolMessageType('GetZhubiRequest', (_message.Message,), dict(
  DESCRIPTOR = _GETZHUBIREQUEST,
  __module__ = 'zhubi_pb2'
  # @@protoc_insertion_point(class_scope:QuoteProto.GetZhubiRequest)
  ))
_sym_db.RegisterMessage(GetZhubiRequest)

GetZhubiResponse = _reflection.GeneratedProtocolMessageType('GetZhubiResponse', (_message.Message,), dict(
  DESCRIPTOR = _GETZHUBIRESPONSE,
  __module__ = 'zhubi_pb2'
  # @@protoc_insertion_point(class_scope:QuoteProto.GetZhubiResponse)
  ))
_sym_db.RegisterMessage(GetZhubiResponse)

GetFenJiaRequest = _reflection.GeneratedProtocolMessageType('GetFenJiaRequest', (_message.Message,), dict(
  DESCRIPTOR = _GETFENJIAREQUEST,
  __module__ = 'zhubi_pb2'
  # @@protoc_insertion_point(class_scope:QuoteProto.GetFenJiaRequest)
  ))
_sym_db.RegisterMessage(GetFenJiaRequest)

GetFenJiaResponse = _reflection.GeneratedProtocolMessageType('GetFenJiaResponse', (_message.Message,), dict(
  DESCRIPTOR = _GETFENJIARESPONSE,
  __module__ = 'zhubi_pb2'
  # @@protoc_insertion_point(class_scope:QuoteProto.GetFenJiaResponse)
  ))
_sym_db.RegisterMessage(GetFenJiaResponse)


# @@protoc_insertion_point(module_scope)
