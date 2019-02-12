# -*- coding: utf-8 -*-

import socket
import struct

class defaultCodec(object):
    def __init__(self):
        pass
    def Marshal(self, *msgs):
        pass
    def Unmarshal(self, data):
        pass

class client(object):
    __slots__ = ["__socket", "__codec", "__addr", "__head_len", "__include_head",
                 "__big_endian", "__last_read_data"]
    def __init__(self, addr_tuple, head_len=4, big_endian= True, include_head=True, codec=defaultCodec):
        self.__addr = addr_tuple
        self.__head_len = head_len
        [1,2,4,8].index(head_len)

        self.__big_endian = big_endian
        self.__include_head = include_head
        self.__codec = codec
        self.__socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.__last_read_data = []
    def Connect(self):
        try:
            err = self.__socket.connect(self.__addr)
            return err
        except Exception as e:
            return e

    def Close(self):
        self.__socket.close()
        
    #发送一条完整的消息
    def Send(self, *msgs):
        try:
            data, err = self.__codec.Marshal(*msgs)
            if err:
                return err

            msg_len = len(data)
            if self.__include_head:
                msg_len += self.__head_len
            len_pack_fmt = "B"
            if self.__head_len == 2:
                len_pack_fmt = "H"
            elif self.__head_len == 4:
                len_pack_fmt = "I"
            elif self.__head_len == 8:
                len_pack_fmt = "Q"
            if self.__big_endian:
                len_pack_fmt = "!" + len_pack_fmt
            len_data = struct.pack(len_pack_fmt, msg_len)
            data = len_data + data

            self.__socket.sendall(data)

            return None
        except Exception as e:
            return e

    #读取一条完整的信息
    def Read(self):
        try:
            len_data = []
            recved_len = 0
            while True:
                rdata = self.__socket.recv(self.__head_len - recved_len)
                if not rdata:
                    raise Exception("socket recv error")
                len_data += rdata
                recved_len = len(len_data)
                if self.__head_len - recved_len == 0:
                    break

            len_unpack_fmt = "B"
            if self.__head_len == 2:
                len_unpack_fmt = "H"
            elif self.__head_len == 4:
                len_unpack_fmt = "I"
            elif self.__head_len == 8:
                len_unpack_fmt = "Q"
            if self.__big_endian:
                len_unpack_fmt = "!" + len_unpack_fmt
            (msg_len,) = struct.unpack(len_unpack_fmt, bytes(len_data))
            if self.__include_head:
                msg_len = msg_len - self.__head_len

            body_data = []
            recved_len = 0
            while True:
                rdata = self.__socket.recv(msg_len - recved_len)
                if not rdata:
                    raise Exception("socket recv error")
                body_data += rdata
                recved_len = len(body_data)
                if msg_len - recved_len == 0:
                    break

            return self.__codec.Unmarshal(bytes(body_data))
        except Exception as e:
            return None, e