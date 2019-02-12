# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'main_window.ui'
#
# Created by: PyQt5 UI code generator 5.11.2
#
# WARNING! All changes made in this file will be lost!

from PyQt5 import QtCore, QtGui, QtWidgets

class Ui_MainWindow(object):
    def setupUi(self, MainWindow):
        MainWindow.setObjectName("MainWindow")
        MainWindow.resize(861, 577)
        self.centralwidget = QtWidgets.QWidget(MainWindow)
        self.centralwidget.setObjectName("centralwidget")
        self.groupBox = QtWidgets.QGroupBox(self.centralwidget)
        self.groupBox.setGeometry(QtCore.QRect(30, 120, 341, 411))
        self.groupBox.setAlignment(QtCore.Qt.AlignLeading|QtCore.Qt.AlignLeft|QtCore.Qt.AlignTop)
        self.groupBox.setObjectName("groupBox")
        self.groupBox_2 = QtWidgets.QGroupBox(self.centralwidget)
        self.groupBox_2.setGeometry(QtCore.QRect(30, 10, 341, 101))
        self.groupBox_2.setTitle("")
        self.groupBox_2.setObjectName("groupBox_2")
        self.pushButton = QtWidgets.QPushButton(self.groupBox_2)
        self.pushButton.setGeometry(QtCore.QRect(250, 10, 71, 31))
        self.pushButton.setObjectName("pushButton")
        self.user_gb = QtWidgets.QGroupBox(self.groupBox_2)
        self.user_gb.setGeometry(QtCore.QRect(10, 50, 321, 51))
        self.user_gb.setTitle("")
        self.user_gb.setObjectName("user_gb")
        self.name_lb = QtWidgets.QLabel(self.user_gb)
        self.name_lb.setGeometry(QtCore.QRect(0, 20, 31, 31))
        self.name_lb.setAlignment(QtCore.Qt.AlignRight|QtCore.Qt.AlignTrailing|QtCore.Qt.AlignVCenter)
        self.name_lb.setObjectName("name_lb")
        self.pwd_lb = QtWidgets.QLabel(self.user_gb)
        self.pwd_lb.setGeometry(QtCore.QRect(160, 20, 31, 31))
        self.pwd_lb.setAlignment(QtCore.Qt.AlignRight|QtCore.Qt.AlignTrailing|QtCore.Qt.AlignVCenter)
        self.pwd_lb.setObjectName("pwd_lb")
        self.name_input = QtWidgets.QLineEdit(self.user_gb)
        self.name_input.setGeometry(QtCore.QRect(30, 20, 131, 20))
        self.name_input.setObjectName("name_input")
        self.pwd_input = QtWidgets.QLineEdit(self.user_gb)
        self.pwd_input.setGeometry(QtCore.QRect(190, 20, 121, 20))
        self.pwd_input.setObjectName("pwd_input")
        self.comboBox = QtWidgets.QComboBox(self.groupBox_2)
        self.comboBox.setGeometry(QtCore.QRect(20, 10, 221, 31))
        self.comboBox.setObjectName("comboBox")
        self.groupBox_3 = QtWidgets.QGroupBox(self.centralwidget)
        self.groupBox_3.setGeometry(QtCore.QRect(390, 10, 411, 521))
        self.groupBox_3.setObjectName("groupBox_3")
        self.msg_browser = QtWidgets.QTextBrowser(self.groupBox_3)
        self.msg_browser.setGeometry(QtCore.QRect(10, 20, 391, 491))
        self.msg_browser.setObjectName("msg_browser")
        self.reset_btn = QtWidgets.QPushButton(self.centralwidget)
        self.reset_btn.setGeometry(QtCore.QRect(810, 0, 51, 31))
        self.reset_btn.setObjectName("reset_btn")
        MainWindow.setCentralWidget(self.centralwidget)
        self.menubar = QtWidgets.QMenuBar(MainWindow)
        self.menubar.setGeometry(QtCore.QRect(0, 0, 861, 23))
        self.menubar.setObjectName("menubar")
        MainWindow.setMenuBar(self.menubar)
        self.statusbar = QtWidgets.QStatusBar(MainWindow)
        self.statusbar.setObjectName("statusbar")
        MainWindow.setStatusBar(self.statusbar)

        self.retranslateUi(MainWindow)
        QtCore.QMetaObject.connectSlotsByName(MainWindow)

    def retranslateUi(self, MainWindow):
        _translate = QtCore.QCoreApplication.translate
        MainWindow.setWindowTitle(_translate("MainWindow", "MainWindow"))
        self.groupBox.setTitle(_translate("MainWindow", "消息参数"))
        self.pushButton.setText(_translate("MainWindow", "发送"))
        self.name_lb.setText(_translate("MainWindow", "账号"))
        self.pwd_lb.setText(_translate("MainWindow", "密码"))
        self.groupBox_3.setTitle(_translate("MainWindow", "返回消息"))
        self.reset_btn.setText(_translate("MainWindow", "RESET"))

