#!/bin/bash

HOST=$1
SO=libdbsvr.so
DST_DIR=$2/DBServer

scp ./bin/$SO $HOST:~/$DST_DIR

CMD="cd $DST_DIR; rm bin/$SO; mv $SO bin; ./startup c"
sshpass ssh $HOST -o StrictHostKeyChecking=no $CMD
