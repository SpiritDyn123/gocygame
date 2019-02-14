# -*- coding: utf-8 -*-

import  sys
from main_window import *
from PyQt5.QtWidgets import *
from PyQt5 import *
from PyQt5.QtCore import *
from PyQt5.QtGui import *
from PyQt5.QtWidgets import *
from enum import *
from config import *
from output import *
from net.client import client as tcpClient
from net.codec import *
from net.http_login import *
import Crypto.PublicKey.RSA
import Crypto.Cipher.PKCS1_v1_5
import threading
import datetime
import time
from google.protobuf.json_format import MessageToDict
import json
from PyQt5 import sip
from proto.login_pb2 import  *
from proto.cs_protoid_pb2 import *
from google.protobuf import symbol_database as _symbol_database
_sym_db = _symbol_database.Default()

class BtnState(Enum):
    Environment = 1
    Login = 2
    Send = 3


LABEL_OPTIONAL = 1
LABEL_REQUIRED = 2
LABEL_REPEATED = 3


class ClientWin(QMainWindow):
    def __init__(self, logic_win):
        self.__win = logic_win
        QMainWindow.__init__(self)

    def closeEvent(self, ev):
        self.__win.exit()
        ev.accept()

class MsgWin(object):
    def __init__(self):
        self.winApp = QApplication(sys.argv)
        self.__ui = Ui_MainWindow()
        self.__mainWindow = ClientWin(self)
        self.__ui.setupUi(self.__mainWindow)
        self.__gb_layout = QGridLayout()
        self.__gb_layout.setAlignment(Qt.AlignTop)
        self.__gb_layout.setVerticalSpacing(15)
        self.__ui.groupBox.setLayout(self.__gb_layout)

        #注册信号
        self.__ui.pushButton.clicked.connect(self.on_click)
        self.__ui.reset_btn.clicked.connect(self.__onClickReset)

        #定时器
        self.__timer = QTimer()
        self.__timer.timeout.connect(self.__onTimer)
        self.__timer.start(500)

        #状态初始化
        self.__cli = None
        self.__recv_thread = None
        self.__initUIAndNet()

        #日志初始化
        Log_Init(Log_file)
    def __onclick_recvmsg_btn(self):
        print("__onclick_recvmsg_btn")

    @property
    def RECV_MSG_CNT(self):
        if not hasattr(self, "__recv_msg_cnt"):
            self.__recv_msg_cnt = 5
        return self.__recv_msg_cnt

    def __new__(cls):
        # 关键在于这，每一次实例化的时候，我们都只会返回这同一个instance对象
        if not hasattr(cls, 'instance'):
            cls.instance = super(MsgWin, cls).__new__(cls)
        return cls.instance

    def __initUIAndNet(self):
        # UI初始化
        self.__ui.user_gb.setVisible(False)

        self.SetBtn(st=BtnState.Environment)

        # 修改标题
        self.__mainWindow.setWindowTitle("欢迎使用")

        #变量初始化
        self.__curRow = 0
        self.__cur_msg = None
        self.__recv_msg_queque_main = []
        self.__logined = False
        self.__last_send_msg = time.time()
        self.__seq = 0
        self.__seq2msg = {}

        self.__uid = None
        self.RUN_ENVIREMENT = ""
        self.__clear_msg()

        if self.__cli is not None:
            self.__cli.Close()
            self.__cli = None
        if self.__recv_thread is not None:
            self.__recv_thread.join()
            self.__recv_thread = None

    def exit(self):
        self.__initUIAndNet()
        Log_Close()

    def __onClickReset(self):
        msg = QMessageBox.question(self.__mainWindow, "消息", "确定重新选择环境？", QMessageBox.Yes | QMessageBox.No,
                                   QMessageBox.No)  # 这里是固定格式，yes/no不能动
        if msg == QMessageBox.Yes:
            self.__initUIAndNet()

    def runloop(self):
        self.__mainWindow.show()
        sys.exit(self.winApp.exec_())

    def env_choice_changed(self):
        if self.__btnState != BtnState.Environment:
            return
        #svr_key = self.__ui.comboBox.currentData()

    def login_choice_changed(self):
        if self.__btnState != BtnState.Login:
            return
        login_type = self.__ui.comboBox.currentData()
        if login_type == 1: #游客登录
            self.__ui.user_gb.setVisible(False)
        else: #账户登录
            self.__ui.user_gb.setVisible(True)

    def msg_choice_changed(self):
        if self.__btnState != BtnState.Send:
            return
        self.__clear_msg()
        msg_content = self.__ui.comboBox.currentText()
        if len(msg_content) == 0 :
            self.__cur_msg = None
            return
        #print (msg_content)

        msg_id = int(msg_content.split(":")[0])
        descrip = Proto_msg_map[msg_id][0].DESCRIPTOR
        index = 0
        self.__cur_msg = {"body": Proto_msg_map[msg_id][0], "id": msg_id}
        for k, v in descrip.__dict__["fields_by_name"].items():
            lb_str = k
            str_type = type(getattr(self.__cur_msg["body"](), k)).__name__
            try:
                if str_type.lower().index("repeated") >= 0:
                    str_type = "array"
                    if v.message_type is not None:
                        str_type = str(v.message_type.name)
            except Exception as None_e:
                pass
            lb_str += "[%s]" % str_type

            if v.label == LABEL_OPTIONAL:
                lb_str += "(opt)"
            elif v.label == LABEL_REQUIRED:
                lb_str += "(req)"
            elif v.label == LABEL_REPEATED:
                lb_str += "(repeat)"
            else:
                lb_str += "(unknow)"

            self.__gb_layout.addWidget(QLabel(lb_str), index, 0)
            self.__gb_layout.addWidget(QLineEdit(), index, 1)
            index += 1

    def __clear_msg(self):
        for i in range(0, self.__gb_layout.count()):
            item = self.__gb_layout.takeAt(0)
            self.__gb_layout.removeWidget(item.widget())
            item.widget().deleteLater()

    def SetBtn(self, st=BtnState.Environment):
        self.__btnState = st
        if self.__btnState == BtnState.Environment:
            self.__ui.pushButton.setText("选择环境")
            self.__ui.comboBox.clear()
            for (svr_key, svr_info) in Server_info.items():
                self.__ui.comboBox.addItem(svr_info["name"], svr_key)
            self.__ui.comboBox.currentIndexChanged.connect(self.env_choice_changed)
        elif self.__btnState == BtnState.Send:
            self.__ui.pushButton.setText("发送")

            self.__ui.comboBox.clear()
            self.__ui.comboBox.addItem("")
            for (msg_id, msg_desc) in Proto_msg_map.items():
                c_line = str(msg_id) + ":" + msg_desc[0].__name__
                self.__ui.comboBox.addItem(c_line)
            self.__ui.comboBox.currentIndexChanged.connect(self.msg_choice_changed)
        else:
            self.__ui.pushButton.setText("登陆")
            self.__ui.comboBox.clear()
            self.__ui.comboBox.addItem("游客登录", Login_type_visitor)
            self.__ui.comboBox.addItem("账户登录", Login_type_user)
        self.__ui.comboBox.currentIndexChanged.connect(self.login_choice_changed)

    def on_click(self):
        if self.__btnState == BtnState.Environment:
            self.RUN_ENVIREMENT = self.__ui.comboBox.currentData()
            self.__mainWindow.setWindowTitle(Server_name(self.RUN_ENVIREMENT))
            self.SetBtn(BtnState.Login)
            Log_Info("*******************当前环境：%s****************" %(Server_name(self.RUN_ENVIREMENT)))

        elif self.__btnState == BtnState.Send:
            if self.__cur_msg is None :
                return
            #self.SetBtn(BtnState.Search)
            mh = msgHead()
            mh.cmd = self.__cur_msg["id"]
            mh.uid = self.__uid
            self.__seq += 1
            mh.seq = self.__seq

            msg_body = self.__cur_msg["body"]()
            try:
                for i in range(0, self.__gb_layout.count()):
                    if i % 2 == 1:
                        key = self.__gb_layout.itemAt(i-1).widget().text()
                        key = key[:key.index("[")]
                        value = self.__gb_layout.itemAt(i).widget().text()
                        field_type = type(getattr(msg_body, key))
                        field_set = msg_body.DESCRIPTOR.__dict__["fields_by_name"][key]

                        def __setJsonField(obj, js_value):
                            if type(js_value) == dict:
                                for ok, ov in js_value.items():
                                    if type(ov) == dict:
                                        ch_obj = getattr(obj, ok)
                                        __setJsonField(ch_obj, ov)
                                    elif type(ov) == list:
                                        ch_obj = getattr(obj, ok)
                                        for item in ov:
                                            if type(item) == dict:
                                                tt = ch_obj.add()
                                                __setJsonField(tt, ov)
                                            else:
                                                ch_obj.append(item)
                                    else:
                                        setattr(obj, ok, ov)
                            else:  # list 对应的是repeat结构
                                for item in js_value:
                                    if type(item) == dict:
                                        tt = obj.add()
                                        __setJsonField(tt, item)
                                    else:
                                        obj.append(item)

                        #optional 非str字段 空字符串不赋值
                        if value == "" and field_set.label == LABEL_OPTIONAL and field_type != str:
                            continue
                        if field_type.__name__ in ["int", "str", "float", "bool"]:
                            setattr(msg_body, key, field_type(value))
                        else:
                            js_value = json.loads(value)
                            obj = getattr(msg_body, key)
                            __setJsonField(obj, js_value)

                err = self.__cli.Send(mh, msg_body)
                if err:
                    self.__addMsg({"time": datetime.datetime.now(), "msg": [10, str(mh.cmd) + "发送失败:"+str(err)]})
                else:
                    self.__seq2msg[mh.seq] = time.time()
            except Exception as e:
                self.__addMsg({"time": datetime.datetime.now(), "msg": [10, str(mh.cmd) + "消息类型赋值失败:" + str(e)]})
        else:
            login_type = self.__ui.comboBox.currentData()
            login_desc = "游客登录"
            err = None
            if login_type == Login_type_visitor:  # 游客登录
                #err = self.__loginGuest()
                pass
            elif login_type == Login_type_user:
                # name = self.__ui.name_input.text()
                # if name == "":
                #     return
                # pwd = self.__ui.pwd_input.text()
                # if pwd == "":
                #     return
                login_desc = "账号登陆"
                #err = self.__loginUser(name, pwd)
            err = self.__login(login_type)

            if not err:
                self.__ui.comboBox.setVisible(True)
                self.SetBtn(BtnState.Send)
                self.__recv_msg_queque_thread = []
                self.__queque_lock = threading.Lock()

                self.__recv_thread = threading.Thread(target=self.__recvMsgThreadFunc,args=(1,))
                self.__recv_thread.start()

                self.__seq = 0
                Log_Info("%d登陆成功"%self.__uid)
                self.__ui.user_gb.setVisible(False)
                self.__mainWindow.setWindowTitle(Server_name(self.RUN_ENVIREMENT) + "-" + login_desc + "-" +"uid_" + str(self.__uid))
            else:
                self.__addMsg({"time": datetime.datetime.now(), "msg": [1, str(err)]})
    def __updateRecvMsgList(self):
        msg_spliter_str = "========================================\n"
        for i, msg_item in enumerate(self.__recv_msg_queque_main):
            line = msg_spliter_str + str(msg_item["time"])
            if msg_item["msg"][0] == 0:
                line += " [INFO]"
                line += "[cmd:" + str(msg_item["msg"][1][0]) + ":" \
                        + Proto_msg_map[msg_item["msg"][1][0]][1].__name__

                if len(msg_item["msg"][1]) > 3:
                    line += " 耗时:" + str(int(msg_item["msg"][1][3] * 1000)) + "ms]"
                else:
                    line += "推送]"

                pb_msg_len = msg_item["msg"][1][2]
                if pb_msg_len > Msg_show_len_limit :
                    line += "length:" + str(pb_msg_len) + "消息过大，前往日志查看"
                else:
                    # proto_msg_str = MessageToJson(msg_item["msg"][1][1], False, True, 0)  # .replace("\n", " ")
                    # line += proto_msg_str
                    line += self.__proto_to_str(msg_item["msg"][1][1])
            else:
                line +=" [ERROR]"
                line += str(msg_item["msg"][1])
            self.__ui.msg_browser.append(line)

        self.__recv_msg_queque_main = []
        #检查文本框大小
        all_txt_arr = self.__ui.msg_browser.toPlainText().split(msg_spliter_str)
        if len(all_txt_arr) > 10:
            all_txt_arr = all_txt_arr[len(all_txt_arr) - 20:]
            all_txt_arr.insert(0, "更多消息请前往log查看\n")
            new_txt = msg_spliter_str.join(all_txt_arr)
            self.__ui.msg_browser.setText(new_txt)

        bar = self.__ui.msg_browser.verticalScrollBar()
        bar.setValue(bar.maximum())

    def __onTimer(self):
        if self.__logined is True:
            self.__queque_lock.acquire()
            msg_queque = self.__recv_msg_queque_thread
            self.__recv_msg_queque_thread = []
            self.__queque_lock.release()
            if len(msg_queque) > 0:
                nowstr = datetime.datetime.now()
                for msg in msg_queque:
                    self.__addMsg({"msg":msg, "time":nowstr}, True)
            now  = time.time()
            #心跳
            if now - self.__last_send_msg > 15:
                self.__last_send_msg = now
                mh = msgSympolHead()
                mh.cmd = CS_MSG_HEART_BEAT
                mh.uid = self.__uid
                self.__cli.Send(mh)

    def __parseRecvMsg(self, cmd, str_data):
        if cmd in Proto_msg_map:
            recv_msg = Proto_msg_map[cmd][1]()
            err = recv_msg.ParseFromString(str_data)
            if err:
                return None, err
            return recv_msg, None
        return None, "unregister cmd:" + str(cmd)

    def __proto_to_str(self, msg):
        dict_data = MessageToDict(msg, preserving_proto_field_name=True, including_default_value_fields=True)
        return json.dumps(dict_data, ensure_ascii=False, indent=2)

    def __addMsg(self, msg, update_ui=True):
        if msg["msg"][0] == 0:
            if len(msg["msg"][1]) > 2:
                Log_Info("收到消息%d %s 耗时:%dms %s" % (
                msg["msg"][1][0], Proto_msg_map[msg["msg"][1][0]][1].__name__, msg["msg"][1][2] * 1000, self.__proto_to_str(msg["msg"][1][1])))
            else:
                Log_Info("收到推送消息%d %s %s"%(msg["msg"][1][0], Proto_msg_map[msg["msg"][1][0]][1].__name__, self.__proto_to_str(msg["msg"][1][1])))
        else:
            Log_Error("发生错误:%s"%(str(msg["msg"][1])))

        self.__recv_msg_queque_main.append(msg)
        if update_ui:
            self.__updateRecvMsgList()

    def __login(self, login_type):
        login_msg = PbCsPlayerLoginReqMsg()
        login_msg.login_type = login_type
        if login_type == Login_type_user:
            name = self.__ui.name_input.text()
            if name == "":
                return "用户名不能为空"
            pwd = self.__ui.pwd_input.text()
            if pwd == "":
                return "密码不能为空"
            login_msg.user_name = name
            login_msg.user_name = pwd

        cc = clientNoEncryptCodec()
        self.__cli = tcpClient(Tcp_ser_addr(self.RUN_ENVIREMENT), codec=cc)
        err = self.__cli.Connect()
        if err:
            return err

        mh = msgSympolHead()
        mh.cmd = CS_MSG_PLAYER_LOGIN
        mh.seq = 0
        mh.uid = 111

        bt = time.time()
        err = self.__cli.Send(mh, login_msg)
        if err:
            return err

        msgs, err = self.__cli.Read()
        if err:
            return err
        recv_msg, err = self.__parseRecvMsg(msgs[0].cmd, msgs[1])
        if err:
            return err

        t_p = time.time() - bt
        self.__addMsg({"time": datetime.datetime.now(), "msg": [0, [msgs[0].cmd, recv_msg, len(msgs[1]), t_p]]})

        if recv_msg.ret.err_code != 0:
            return "login ret=" + str(recv_msg.ret.err_code) + ",msg=" + recv_msg.ret.err_msg

        self.__uid = recv_msg.uid
        self.__logined = True
        self.__last_send_msg = time.time()

        return None

    #游客登录
    # def __loginGuest(self):
    #     # 初始化
    #     token_data, err = HttpGuestLogin(Http_guest_url(self.RUN_ENVIREMENT))
    #     if err:
    #         return err
    #     return self.__login(token_data)
    #
    # #用户登录
    # def __loginUser(self, name, pwd):
    #     token_data, err = HttpUserLogin(Http_user_url(self.RUN_ENVIREMENT), Rsa_pub_Key, name, pwd)
    #     if err:
    #         return err
    #    return self.__login(token_data)

    def __recvMsgThreadFunc(self, data):
        while True:
            msgs, err = self.__cli.Read()
            now = time.time()
            if err:
                self.__queque_lock.acquire()
                self.__recv_msg_queque_thread.append([1, err])
                self.__queque_lock.release()
                return
            else:
                if msgs[0].cmd == CS_MSG_HEART_BEAT or len(msgs) < 2:#heartbeat丢弃
                    continue

                recv_msg, err = self.__parseRecvMsg(msgs[0].cmd, msgs[1])
                self.__queque_lock.acquire()
                if err:
                    self.__recv_msg_queque_thread.append([2, err])
                else:
                    mi = [0, [msgs[0].cmd, recv_msg, len(msgs[1])]]
                    if msgs[0].seq in self.__seq2msg.keys():
                        mi[1].append( now- self.__seq2msg[msgs[0].seq])
                        self.__seq2msg[msgs[0].seq] = now
                    self.__recv_msg_queque_thread.append(mi)
                self.__queque_lock.release()




