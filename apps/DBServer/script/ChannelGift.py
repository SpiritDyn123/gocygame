#!/usr/bin/env python
# -*- coding:UTF-8 -*-
import struct
import os
import sys
import time
from optparse import OptionParser
from redis import StrictRedis
from redis.exceptions import ConnectionError, ResponseError
import proto.CommonMsg_pb2
import csv

class Profier(object):
    def __init__(self):
        self.redis = None

    def ConnectRedis(self, host, port, db, password):
       try:
            self.redis = StrictRedis(host=host, port=port, db=db, password=password)
       except ConnectionError as e:
            sys.stderr.write('Could not connect to Redis Server : %s\n' % e)
            sys.exit(-1)
       except ResponseError as e:
            sys.stderr.write('Could not connect to Redis Server : %s\n' % e)
            sys.exit(-1)
    def PutData(self, filename):
        with open(filename) as f:
            reader = csv.reader(f)
            fgifts = list(reader)
            gifts = {}
            for line in fgifts[3:] :
                mail = proto.CommonMsg_pb2.MailBase()
                mail.mail_type = int(line[1])
                items = line[1].split('|')
                for l in items :
                    item = l.split('#')
                    reward = mail.attach.add()
                    reward.type = int(l[0])
                    reward.id = int(l[1])
                    reward.cnt = int(l[2])
                gifts[line[3]] = mail.SerializeToString() 

            self.redis.hmset("kKeyChannelGiftInfo", gifts)



def main():
    usage = """usage: %prog [options]
            Examples :
            %prog -s localhost -p 6379
            """
    parser = OptionParser(usage=usage)
    parser.add_option("-s", "--server", dest="host", default="172.19.68.57", 
            help="Redis Server hostname. Defaults to 172.19.68.57")
    parser.add_option("-p", "--port", dest="port", default=19000, type="int", 
            help="Redis Server port. Defaults to 19000")
    parser.add_option("-d", "--db", dest="db", default=0,
            help="Database number, defaults to 0")
    parser.add_option("-a", "--password", dest="password", 
            help="Password to use when connecting to the server")
    parser.add_option("-f", "--file", dest="file", 
            help="csv file to parse")
    (options, args) = parser.parse_args()

    profiler = Profier()
    profiler.ConnectRedis(options.host, options.port, options.db, options.password)
    profiler.PutData(options.file)



if __name__ == '__main__' :
    main()
