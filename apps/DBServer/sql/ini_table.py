#!/usr/bin/python
#-*- coding: utf-8 -*-

#install MySQLdb
#pip install MySQL-python

from warnings import filterwarnings
import MySQLdb
filterwarnings('ignore', category = MySQLdb.Warning)

import json

def DropDB(cursor, prefix, cnt):
	dw = get_num_width(cnt)
	dbfmt = "%%s_%%0%dd"%dw
	for i in range(0,cnt):
		sql = "drop database if exists %s"%(dbfmt%(prefix,i))
		cursor.execute(sql)

def get_num_width(n):
	base = 10
	v = n - 1
	w = 0
	while v > 0:
		w += 1
		v /= base
	if w == 0:
		w = 1
	return w
	
dbCfg = 'tblCfg.json'

with open(dbCfg) as jfp:
	jObj = json.load(jfp)

info = jObj['mysql']
host = info['host']
user = info['user']
pwd = info['pwd']
port = info['port']

mysql = MySQLdb.connect(host=host, port=port, user=user, passwd=pwd)
cursor = mysql.cursor()

for tInfo in jObj['table_info']:
	dbCnt = tInfo['db_cnt']
	tblCnt = tInfo['tbl_cnt']
	dbPrefix = tInfo['db_prefix']
	sqlPath = tInfo['sql_path']
	
	dw = get_num_width(dbCnt)
	tw = get_num_width(tblCnt)
	#DropDB(cursor, dbPrefix, dbCnt)
	#exit(0)
	dbfmt = "%%s_%%0%dd"%dw
	tblfmt = "%%0%dd"%tw
	
	sqlTpl = ""
	with open(sqlPath) as fp:
		sqlTpl = fp.read()
	for i in range(0, dbCnt):
		#if db not exists create
		dbName = dbfmt%(dbPrefix, i)
		sql = "create database if not exists %s"%(dbName)
		try:
			cursor.execute(sql)
		except MySQLdb.Error,e:
			print("Mysql Error %d: %s" % (e.args[0], e.args[1]))
		mysql.select_db(dbName)
		for j in range(0, tblCnt):
			#if tbl not exists create
			sql = sqlTpl%(tblfmt%j)
			cursor.execute(sql)
mysql.commit()

