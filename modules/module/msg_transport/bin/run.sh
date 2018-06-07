#!/bin/bash

killall -9 nginx
killall -9 access.cgi

LOGS="./logs"
if [ ! -d "$LOGS" ]; then
	mkdir -p $LOGS
#else
	#rm $UPLOAD_DIR/* -rf
fi

ulimit -HSn 65536
ulimit -c unlimited

CURDIR=`pwd`
./msg_trans

