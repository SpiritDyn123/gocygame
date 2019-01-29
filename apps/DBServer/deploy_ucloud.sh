#!/bin/bash

sshpass -p "xxxx" scp -P14315 ./bin/libdbsvr.so doogga@61.174.15.142:~/dbserver
sshpass -p "xxxx" ssh -p14315 doogga@61.174.15.142 -o StrictHostKeyChecking=no 'cd dbserver; ./restart_ucloud.sh'
