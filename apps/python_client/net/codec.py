# -*- coding: utf-8 -*-
import struct
from net.client import *
from Crypto.Cipher import AES
from binascii import  *

class msgHead(object):
    def __init__(self):
        self.cmd = 0  # uint16 h
        self.ver = 0  # uint8 B
        self.type = 1  # uint8 B
        self.seq = 0  # uint32 I
        self.uid = 0  # uint64 Q

class AesCipher(object):
    def __init__(self, key):
        self.__key = key
        self.__cipher = AES.new(self.__key, AES.MODE_ECB)

    def encrypt(self, text):
        if isinstance(text, str):
            text = bytes(text, encoding="utf-8")
        padding_len = 16 - len(text)%16
        if padding_len > 0:
            padding_text = ""
            for i in range(padding_len):
                padding_text += chr(padding_len)
            text += bytes(padding_text, encoding="utf-8")

        return self.__cipher.encrypt(text)

    def decrypt(self, text):
        pass

class clientCodec(defaultCodec):
    def __init__(self):
        self.__rsa = None
        self.__aes = None

    def SetRsa(self, rsa_cipher):
        self.__rsa = rsa_cipher

    def SetAes(self, aes_cipher):
        self.__aes = aes_cipher

    def Marshal(self, *msgs):
        if len(msgs) < 1:
            return None, "msgs must > 1"
        data = struct.pack("!HBBIQ", msgs[0].cmd, msgs[0].ver, msgs[0].type, msgs[0].seq, msgs[0].uid)
        if len(msgs) > 1:
            proto_data = msgs[1].SerializeToString()
            if msgs[0].cmd == 100:
                if self.__rsa:
                    data += self.__rsa.encrypt(proto_data)
                else:
                    data += proto_data
            else:
                if self.__aes:
                    data += self.__aes.encrypt(proto_data)
                else:
                    data += proto_data

        return data, None

    def Unmarshal(self, data):
        if len(data) < 16:
            return None, "msg head len <16"

        msg_head = msgHead()
        (msg_head.cmd, msg_head.ver, msg_head.type, msg_head.seq, msg_head.uid) = struct.unpack("!HBBIQ", data[:16])
        msgs = [msg_head]
        if len(data) > 16:
            msgs.append(data[16:])
        return msgs, None

class msgSympolHead(object):
    def __init__(self):
        self.cmd = 0  # uint32 I
        self.seq = 0  # uint32 I
        self.uid = 0  # uint64 Q

class clientNoEncryptCodec(defaultCodec):
    def __init__(self):
        pass
    def Marshal(self, *msgs):
        if len(msgs) < 1:
            return None, "msgs must > 1"
        data = struct.pack("!IIQ", msgs[0].cmd, msgs[0].seq, msgs[0].uid)
        if len(msgs) > 1:
            proto_data = msgs[1].SerializeToString()
            data += proto_data

        return data, None

    def Unmarshal(self, data):
        if len(data) < 16:
            return None, "msg head len <16"

        msg_head = msgHead()
        (msg_head.cmd, msg_head.seq, msg_head.uid) = struct.unpack("!IIQ", data[:16])
        msgs = [msg_head]
        if len(data) > 16:
            msgs.append(data[16:])
        return msgs, None
