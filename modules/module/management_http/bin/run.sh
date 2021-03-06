#!/bin/bash

killall -9 nginx
killall -9 management.cgi

LOGS="./logs"
if [ ! -d "$LOGS" ]; then
	mkdir -p $LOGS
#else
	#rm $UPLOAD_DIR/* -rf
fi

ulimit -HSn 65536
ulimit -c unlimited

CURDIR=`pwd`
./nginx_1.10.2/sbin/nginx -p $CURDIR/nginx_1.10.2
./spawn-fcgi -p 9000 -F 1 -f "$CURDIR/management.cgi"

