#!/bin/bash

for ip in "10.4.3.239" "10.4.14.43"
do
    sshpass -p "xxxx" scp libdbsvr.so doogga@$ip:~/dbserver
    sshpass -p "xxxx" ssh doogga@$ip -o StrictHostKeyChecking=no 'cd dbserver; rm -v -f ./bin/libdbsvr.so; cp -v libdbsvr.so ./bin; ./startup c > startup.log; ./verify_startup_log.sh'
done
