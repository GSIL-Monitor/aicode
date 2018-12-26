# coding=utf-8
import sys
import time

import socket


import os
import logging
import inspect

logger = logging.getLogger('[PythonService]')

this_file = inspect.getfile(inspect.currentframe())
dirpath = os.path.abspath(os.path.dirname(this_file))
handler = logging.FileHandler(os.path.join(dirpath, "ipservice.log"))

formatter = logging.Formatter('%(asctime)s %(name)-12s %(levelname)-8s %(message)s')
handler.setFormatter(formatter)

logger.addHandler(handler)
logger.setLevel(logging.INFO)

def RestartRinetd():
    pid = ''
    PsResult = os.popen('ps -ef | grep rinetd').readlines()
    for line in PsResult:
        if -1 != line.find('r.conf'):
            fields = line.split()
            pid = fields[1]
            break
    if not pid:
        os.popen('rinetd -c r.conf')
    else:
        os.popen('kill -9 %s' % pid)
        os.popen('rinetd -c r.conf')

def CheckRinetd():
    pid = ''
    PsResult = os.popen('ps -ef | grep rinetd').readlines()
    for line in PsResult:
        if -1 != line.find('r.conf'):
            fields = line.split()
            pid = fields[1]
            break
    if not pid:
        os.popen('rinetd -c r.conf')


def rinetd(ip):
    logger.info('IP is %s' % ip)

    flag = False
    PsResult = os.popen('cat r.conf').readlines()
    for line in PsResult:
        if -1 == line.find(ip):
            change_cfg = 'echo "0.0.0.0 9114 %s 3389" > r.conf' % ip
            os.popen(change_cfg)
            RestartRinetd()
            flag = True
            logger.info('rinetd restarted')
            break

    #表示检查进程是否存在
    if not flag:
        CheckRinetd()


class ClientMsgHandler(object):
    def __init__(self, host, port):
        self.host = host
        self.port = port
        self.skt = socket.socket(socket.AF_INET,socket.SOCK_STREAM)

    def Connect(self):
        self.skt.connect((self.host, self.port))

    def Send(self, msg):
        self.skt.sendall(msg)
        ret = self.skt.recv(1024)
        logger.info(ret)

    def Close(self):
        self.skt.close()

class ServerMsgHandler(object):
    def __init__(self, host, port):
        self.host = host
        self.port = port
        self.skt = socket.socket(socket.AF_INET,socket.SOCK_STREAM)

    def Run(self, process):
        self.skt.bind((self.host, self.port))
        self.skt.listen(100)
        while True:
            try:
                conn, addr = self.skt.accept()
                logger.info("Client connect from %s" % str(addr))
                msg = conn.recv(1024)
                process(msg)
                conn.sendall(msg)
            except socket.error as err:
                logger.info('Socket error %s' % str(err))


if __name__ == "__main__":

    if len(sys.argv) == 1:
        print('Please run by parameter server')
    elif 2 <= len(sys.argv):
        if 'server' == sys.argv[1]:
            s = ServerMsgHandler('0.0.0.0', 7700)
            s.Run(rinetd)


