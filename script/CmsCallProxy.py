#coding=utf-8

import time
import os
import mysql.connector
from mysql.connector import errorcode

db = None

cwd = None

def ConnectDB(db_user, db_pwd, db_host, db_name):
    try:
        cnx = mysql.connector.connect(
            user=db_user, password=db_pwd,
            host=db_host, database=db_name, buffered=True, connection_timeout=1000, ssl_disabled=True)
    except mysql.connector.Error as err:
        print(err)
        print(err.errno)
        if err.errno == errorcode.ER_ACCESS_DENIED_ERROR:
            print("Something is wrong with your user name or password")
        elif err.errno == errorcode.ER_BAD_DB_ERROR:
            print("Database does not exist")
        else:
            print(err)
        return None
    else:
        cnx.autocommit = True
        return cnx


def InitDB():
    global db
    db = ConnectDB('root', '1qaz@WSX', '172.20.120.22', 'PlatformDB')
    if db is None:
        return False

    return True


def GetCmsAddressInfo():
    cursor = db.cursor()
    sql = "select distinct port from t_cms_call_info"

    try:
        cursor.execute(sql)
    except mysql.connector.Error as err:
        print(err)
        db.close()
        return None

    if cursor.rowcount == 0:
        print("Port not founded")
        cursor.close()
        db.close()
        return None

    ports = []
    for (p,) in cursor:
        print 'Get port is %s' % p
        ports.append(p)

    cursor.close()
    return ports


def GetCmsAddressInfinite():
    while True:
        time.sleep(1.5)
        ports = GetCmsAddressInfo()
        yield ports

def GetRuntimeProxyInfo():
    Info = []
    PsResult = os.popen('ps -ef | grep proxy_cms_call').readlines()
    for line in PsResult:
        if -1 != line.find('grep'):
            continue
        if -1 != line.find('proxy_cms_call'):
            fields = line.split()
            pid = fields[1]
            port = fields[8]
            Info.append((pid, port))
    return Info

def KillProxy(pid):
    os.popen('kill -9 %s' % pid)

def RemoveProxyFile(rootpath, port):
    path = './' + rootpath + '/' + port
    if not os.path.exists(path):
        return
    #os.rmdir(path)
    os.popen('rm -fr %s' % path)

def ReleaseProxyFileAndRun(rootpath, port):
    path = './' + rootpath + '/' + port
    if not os.path.exists(path):
        os.mkdir(path)
    os.popen('tar -xzf cms_proxy.tar.gz -C %s/' % path)

    os.chdir('./%s/%s/proxy_cms_call/' % (rootpath, port)) #change cwd

    os.popen('./run_proxy.sh %s 2' % port)

    os.chdir(cwd) #recover cwd

if __name__ == '__main__':

    #global cwd #because current position cwd is already global scope.
    cwd = os.getcwd()

    print 'Current dir:', os.getcwd()

    if not os.path.exists("cms_proxy.tar.gz"):
        print 'File cms_proxy.tar.gz not exists.'
        exit(0)

    if not InitDB():
        print "Init db failed."

    RootPath = 'cms_proxy'
    if not os.path.exists(RootPath):
        os.mkdir(RootPath)

    for dbports in GetCmsAddressInfinite():
        print 'Database port:', dbports

        if dbports is None:
            print 'Get ports from database failed.'
            exit(0)
        if not dbports: #ports is empty list
            continue


        ports_pid_runing = GetRuntimeProxyInfo()
        print 'Runing port and pid:', ports_pid_runing

        #if proxy of running is not in database, kill this proxy and remove file.
        for pid, runport in ports_pid_runing:
            if runport not in dbports:
                KillProxy(pid)
                RemoveProxyFile(RootPath, runport)

        #if port of proxy from database is not in proxys of running, make this port of proxy running.
        runports = []
        for pid, runport in ports_pid_runing:
            runports.append(runport)

        for dbport in dbports:
            if dbport not in runports:
                ReleaseProxyFileAndRun(RootPath, dbport)










