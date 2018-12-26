# coding=utf-8
import sys
import time

import win32api
import win32event
import win32service
import win32serviceutil
import servicemanager

import socket
import psutil

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

def rinetd(ip):
    logger.info('IP is %s' % ip)



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



class MyService(win32serviceutil.ServiceFramework):

    _svc_name_ = "IPService"
    _svc_display_name_ = "IP Service"
    _svc_description_ = "IP Service to get local ip address"

    def __init__(self, args):
        self.log('init')
        win32serviceutil.ServiceFramework.__init__(self, args)
        self.stop_event = win32event.CreateEvent(None, 0, 0, None)
        self.RunFlag = True

    def GetIP(self):
        while self.RunFlag:
            netcard_info = []
            info = psutil.net_if_addrs()
            for k, v in info.items():
                for item in v:
                    if item[0] == 2 and not item[1] == '127.0.0.1':
                        netcard_info.append((k, item[1]))

            time.sleep(10)
            #win32api.Sleep(50, True)

            for i, j in netcard_info:
                if -1 != j.find('172'):
                    #servicemanager.LogInfoMsg(j)
                    yield j

    def SvcDoRun(self):
        self.ReportServiceStatus(win32service.SERVICE_START_PENDING)
        try:
            self.ReportServiceStatus(win32service.SERVICE_RUNNING)
            self.log('start')
            self.start()
            self.log('wait')
            win32event.WaitForSingleObject(self.stop_event, win32event.INFINITE)
            self.log('done')
        except BaseException as e:
            self.log('Exception : %s' % e)
            self.SvcStop()

    def SvcStop(self):
        self.ReportServiceStatus(win32service.SERVICE_STOP_PENDING)
        self.log('stopping')
        self.stop()
        self.log('stopped')
        win32event.SetEvent(self.stop_event)
        self.ReportServiceStatus(win32service.SERVICE_STOPPED)

    def start(self):

        # 获取本机计算机名称
        #hostname = socket.gethostname()
        # 获取本机ip
        #ip = socket.gethostbyname(hostname)
        #self.log(ip)
        logger.info("service is run....")

        for ip in self.GetIP():
            #self.logger.info(ip)

            #servicemanager.LogInfoMsg(ip)

            cli = ClientMsgHandler('172.20.120.23', 7700)
            try:
                cli.Connect()
                cli.Send(ip)
                cli.Close()
            except socket.error as err:
                self.log(err)


        logger.info("service is exit run....")

    def stop(self):
        logger.info("service is stop....")

        self.RunFlag = False
        #pass


    def log(self, msg):
        servicemanager.LogInfoMsg(str(msg))

    #def sleep(self, minute):
    #    win32api.Sleep((minute*1000), True)

if __name__ == "__main__":

    if len(sys.argv) == 1:
        servicemanager.Initialize()
        servicemanager.PrepareToHostSingle(MyService)
        servicemanager.StartServiceCtrlDispatcher()
    elif 2 <= len(sys.argv):
        if 'server' == sys.argv[1]:
            s = ServerMsgHandler('0.0.0.0', 7700)
            s.Run(rinetd)
        else:
            win32serviceutil.HandleCommandLine(MyService)

