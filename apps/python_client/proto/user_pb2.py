# Generated by the protocol buffer compiler.  DO NOT EDIT!
# source: user.proto

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
  name='user.proto',
  package='QuoteProto',
  syntax='proto2',
  serialized_pb=_b('\n\nuser.proto\x12\nQuoteProto\x1a\x0eret_base.proto\x1a\x14public_message.proto\"%\n\x13\x46irstConnectRequest\x12\x0e\n\x06\x61\x65skey\x18\x01 \x02(\t\"D\n\x14\x46irstConnectResponse\x12\x1c\n\x03ret\x18\x01 \x02(\x0b\x32\x0f.QuoteProto.Ret\x12\x0e\n\x06pubkey\x18\x02 \x01(\t\"@\n\x10UserLoginRequest\x12\x0b\n\x03uid\x18\x01 \x02(\x04\x12\x10\n\x08password\x18\x02 \x01(\t\x12\r\n\x05token\x18\x03 \x01(\t\"\xa5\x01\n\x11UserLoginResponse\x12\x1c\n\x03ret\x18\x01 \x02(\x0b\x32\x0f.QuoteProto.Ret\x12\x0b\n\x03uid\x18\x02 \x02(\x04\x12\x30\n\tkick_info\x18\x03 \x01(\x0b\x32\x1d.QuoteProto.UserLoginKickInfo\x12\x33\n\x0bpermissions\x18\x04 \x01(\x0b\x32\x1e.QuoteProto.UserPermissionInfo\"?\n\x0bUserKickMsg\x12\x30\n\tkick_info\x18\x01 \x02(\x0b\x32\x1d.QuoteProto.UserLoginKickInfo\"H\n\x16UploadLoginInfoRequest\x12.\n\x0blogin_users\x18\x01 \x03(\x0b\x32\x19.QuoteProto.UserLoginInfo\"G\n\x14UserPermissionErrMsg\x12\x0b\n\x03\x63md\x18\x01 \x01(\r\x12\x0b\n\x03seq\x18\x02 \x01(\r\x12\x15\n\rpermission_id\x18\x03 \x01(\x05*.\n\x0cLoginRetType\x12\x0b\n\x07success\x10\x00\x12\x11\n\rdevice_repeat\x10\x01')
  ,
  dependencies=[ret__base__pb2.DESCRIPTOR,public__message__pb2.DESCRIPTOR,])

_LOGINRETTYPE = _descriptor.EnumDescriptor(
  name='LoginRetType',
  full_name='QuoteProto.LoginRetType',
  filename=None,
  file=DESCRIPTOR,
  values=[
    _descriptor.EnumValueDescriptor(
      name='success', index=0, number=0,
      options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='device_repeat', index=1, number=1,
      options=None,
      type=None),
  ],
  containing_type=None,
  options=None,
  serialized_start=619,
  serialized_end=665,
)
_sym_db.RegisterEnumDescriptor(_LOGINRETTYPE)

LoginRetType = enum_type_wrapper.EnumTypeWrapper(_LOGINRETTYPE)
success = 0
device_repeat = 1



_FIRSTCONNECTREQUEST = _descriptor.Descriptor(
  name='FirstConnectRequest',
  full_name='QuoteProto.FirstConnectRequest',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
    _descriptor.FieldDescriptor(
      name='aeskey', full_name='QuoteProto.FirstConnectRequest.aeskey', index=0,
      number=1, type=9, cpp_type=9, label=2,
      has_default_value=False, default_value=_b("").decode('utf-8'),
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
  serialized_start=64,
  serialized_end=101,
)


_FIRSTCONNECTRESPONSE = _descriptor.Descriptor(
  name='FirstConnectResponse',
  full_name='QuoteProto.FirstConnectResponse',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
    _descriptor.FieldDescriptor(
      name='ret', full_name='QuoteProto.FirstConnectResponse.ret', index=0,
      number=1, type=11, cpp_type=10, label=2,
      has_default_value=False, default_value=None,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='pubkey', full_name='QuoteProto.FirstConnectResponse.pubkey', index=1,
      number=2, type=9, cpp_type=9, label=1,
      has_default_value=False, default_value=_b("").decode('utf-8'),
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
  serialized_start=103,
  serialized_end=171,
)


_USERLOGINREQUEST = _descriptor.Descriptor(
  name='UserLoginRequest',
  full_name='QuoteProto.UserLoginRequest',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
    _descriptor.FieldDescriptor(
      name='uid', full_name='QuoteProto.UserLoginRequest.uid', index=0,
      number=1, type=4, cpp_type=4, label=2,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='password', full_name='QuoteProto.UserLoginRequest.password', index=1,
      number=2, type=9, cpp_type=9, label=1,
      has_default_value=False, default_value=_b("").decode('utf-8'),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='token', full_name='QuoteProto.UserLoginRequest.token', index=2,
      number=3, type=9, cpp_type=9, label=1,
      has_default_value=False, default_value=_b("").decode('utf-8'),
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
  serialized_start=173,
  serialized_end=237,
)


_USERLOGINRESPONSE = _descriptor.Descriptor(
  name='UserLoginResponse',
  full_name='QuoteProto.UserLoginResponse',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
    _descriptor.FieldDescriptor(
      name='ret', full_name='QuoteProto.UserLoginResponse.ret', index=0,
      number=1, type=11, cpp_type=10, label=2,
      has_default_value=False, default_value=None,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='uid', full_name='QuoteProto.UserLoginResponse.uid', index=1,
      number=2, type=4, cpp_type=4, label=2,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='kick_info', full_name='QuoteProto.UserLoginResponse.kick_info', index=2,
      number=3, type=11, cpp_type=10, label=1,
      has_default_value=False, default_value=None,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='permissions', full_name='QuoteProto.UserLoginResponse.permissions', index=3,
      number=4, type=11, cpp_type=10, label=1,
      has_default_value=False, default_value=None,
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
  serialized_start=240,
  serialized_end=405,
)


_USERKICKMSG = _descriptor.Descriptor(
  name='UserKickMsg',
  full_name='QuoteProto.UserKickMsg',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
    _descriptor.FieldDescriptor(
      name='kick_info', full_name='QuoteProto.UserKickMsg.kick_info', index=0,
      number=1, type=11, cpp_type=10, label=2,
      has_default_value=False, default_value=None,
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
  serialized_start=407,
  serialized_end=470,
)


_UPLOADLOGININFOREQUEST = _descriptor.Descriptor(
  name='UploadLoginInfoRequest',
  full_name='QuoteProto.UploadLoginInfoRequest',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
    _descriptor.FieldDescriptor(
      name='login_users', full_name='QuoteProto.UploadLoginInfoRequest.login_users', index=0,
      number=1, type=11, cpp_type=10, label=3,
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
  serialized_start=472,
  serialized_end=544,
)


_USERPERMISSIONERRMSG = _descriptor.Descriptor(
  name='UserPermissionErrMsg',
  full_name='QuoteProto.UserPermissionErrMsg',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
    _descriptor.FieldDescriptor(
      name='cmd', full_name='QuoteProto.UserPermissionErrMsg.cmd', index=0,
      number=1, type=13, cpp_type=3, label=1,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='seq', full_name='QuoteProto.UserPermissionErrMsg.seq', index=1,
      number=2, type=13, cpp_type=3, label=1,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='permission_id', full_name='QuoteProto.UserPermissionErrMsg.permission_id', index=2,
      number=3, type=5, cpp_type=1, label=1,
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
  serialized_start=546,
  serialized_end=617,
)

_FIRSTCONNECTRESPONSE.fields_by_name['ret'].message_type = ret__base__pb2._RET
_USERLOGINRESPONSE.fields_by_name['ret'].message_type = ret__base__pb2._RET
_USERLOGINRESPONSE.fields_by_name['kick_info'].message_type = public__message__pb2._USERLOGINKICKINFO
_USERLOGINRESPONSE.fields_by_name['permissions'].message_type = public__message__pb2._USERPERMISSIONINFO
_USERKICKMSG.fields_by_name['kick_info'].message_type = public__message__pb2._USERLOGINKICKINFO
_UPLOADLOGININFOREQUEST.fields_by_name['login_users'].message_type = public__message__pb2._USERLOGININFO
DESCRIPTOR.message_types_by_name['FirstConnectRequest'] = _FIRSTCONNECTREQUEST
DESCRIPTOR.message_types_by_name['FirstConnectResponse'] = _FIRSTCONNECTRESPONSE
DESCRIPTOR.message_types_by_name['UserLoginRequest'] = _USERLOGINREQUEST
DESCRIPTOR.message_types_by_name['UserLoginResponse'] = _USERLOGINRESPONSE
DESCRIPTOR.message_types_by_name['UserKickMsg'] = _USERKICKMSG
DESCRIPTOR.message_types_by_name['UploadLoginInfoRequest'] = _UPLOADLOGININFOREQUEST
DESCRIPTOR.message_types_by_name['UserPermissionErrMsg'] = _USERPERMISSIONERRMSG
DESCRIPTOR.enum_types_by_name['LoginRetType'] = _LOGINRETTYPE
_sym_db.RegisterFileDescriptor(DESCRIPTOR)

FirstConnectRequest = _reflection.GeneratedProtocolMessageType('FirstConnectRequest', (_message.Message,), dict(
  DESCRIPTOR = _FIRSTCONNECTREQUEST,
  __module__ = 'user_pb2'
  # @@protoc_insertion_point(class_scope:QuoteProto.FirstConnectRequest)
  ))
_sym_db.RegisterMessage(FirstConnectRequest)

FirstConnectResponse = _reflection.GeneratedProtocolMessageType('FirstConnectResponse', (_message.Message,), dict(
  DESCRIPTOR = _FIRSTCONNECTRESPONSE,
  __module__ = 'user_pb2'
  # @@protoc_insertion_point(class_scope:QuoteProto.FirstConnectResponse)
  ))
_sym_db.RegisterMessage(FirstConnectResponse)

UserLoginRequest = _reflection.GeneratedProtocolMessageType('UserLoginRequest', (_message.Message,), dict(
  DESCRIPTOR = _USERLOGINREQUEST,
  __module__ = 'user_pb2'
  # @@protoc_insertion_point(class_scope:QuoteProto.UserLoginRequest)
  ))
_sym_db.RegisterMessage(UserLoginRequest)

UserLoginResponse = _reflection.GeneratedProtocolMessageType('UserLoginResponse', (_message.Message,), dict(
  DESCRIPTOR = _USERLOGINRESPONSE,
  __module__ = 'user_pb2'
  # @@protoc_insertion_point(class_scope:QuoteProto.UserLoginResponse)
  ))
_sym_db.RegisterMessage(UserLoginResponse)

UserKickMsg = _reflection.GeneratedProtocolMessageType('UserKickMsg', (_message.Message,), dict(
  DESCRIPTOR = _USERKICKMSG,
  __module__ = 'user_pb2'
  # @@protoc_insertion_point(class_scope:QuoteProto.UserKickMsg)
  ))
_sym_db.RegisterMessage(UserKickMsg)

UploadLoginInfoRequest = _reflection.GeneratedProtocolMessageType('UploadLoginInfoRequest', (_message.Message,), dict(
  DESCRIPTOR = _UPLOADLOGININFOREQUEST,
  __module__ = 'user_pb2'
  # @@protoc_insertion_point(class_scope:QuoteProto.UploadLoginInfoRequest)
  ))
_sym_db.RegisterMessage(UploadLoginInfoRequest)

UserPermissionErrMsg = _reflection.GeneratedProtocolMessageType('UserPermissionErrMsg', (_message.Message,), dict(
  DESCRIPTOR = _USERPERMISSIONERRMSG,
  __module__ = 'user_pb2'
  # @@protoc_insertion_point(class_scope:QuoteProto.UserPermissionErrMsg)
  ))
_sym_db.RegisterMessage(UserPermissionErrMsg)


# @@protoc_insertion_point(module_scope)
