# -*- coding: utf-8 -*-
import mysql.connector
from mysql.connector import errorcode
import HttpHandler
import sys
import json

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
        return cnx


def GetDevIDByP2pID(p2pid):
    db = ConnectDB('root', '1qaz@WSX', '47.91.151.24', 'PlatformDB')
    if db is None:
        return None

    cursor = db.cursor()
    sql = "select deviceid from t_device_info where p2pid like 'ANUS-000%s%%' and status = '0'" % p2pid

    try:
        cursor.execute(sql)
    except mysql.connector.Error as err:
        print(err)
        db.close()
        return None

    if cursor.rowcount == 0:
        print("Device id not founded by p2pid %s." % p2pid)
        cursor.close()
        db.close()
        return None
    elif cursor.rowcount > 1:
        print 'Device id founded but not only one and total is %s' % cursor.rowcount
        for (devid,) in cursor:
            print devid
        cursor.close()
        db.close()
        return None

    for (devid,) in cursor:
        print 'Get device id is %s' % devid
        cursor.close()
        db.close()
        return devid


def ResetDevice(devid):
    address = 'www.xvripc.net'
    port = 8888
    url = '/access.cgi?action=user_login'

    fields = [('username', 'zycai@annigroup.com'), ('userpwd', '123456'), ('terminaltype', '9'), ('type', '1')]

    status, data = HttpHandler.PostHttp(address, port, url, fields)

    jsResultData = json.loads(data)

    if (200 != status):
        print 'Login server failed %d, %s, %s' % (status, jsResultData['retcode'], jsResultData['retmsg'])
        return False

    sid = jsResultData['sid']

    url = '/access.cgi?action=query_user_of_device'
    fields = [('sid', sid), ('devid', devid), ('beginindex', '0')]
    status, data = HttpHandler.PostHttp(address, port, url, fields)
    jsRelationResultData = json.loads(data)
    if (200 != status):
        print 'Query user of device failed %d, %s, %s' % \
              (status, jsRelationResultData['retcode'], jsRelationResultData['retmsg'])
        return False

    userid = None
    username = None
    for rel in jsRelationResultData['data']:
        if 0 == rel['relation']:
            userid = rel['userid']
            username = rel['username']
            break

    if userid is None:
        print 'Not found userid by device %s' % devid

    print 'Begin to delete device %s by user id %s name %s' % (devid, userid, username)

    url = '/access.cgi?action=delete_device'
    fields = [('sid', sid), ('devid', devid), ('userid', userid)]
    status, data = HttpHandler.PostHttp(address, port, url, fields)
    jsResult = json.loads(data)
    if (200 != status):
        print 'Delete device device failed %d, %s, %s' % (status, jsResult['retcode'], jsResult['retmsg'])
        return False

    print 'Succeed!'
    return  True

if __name__ == '__main__':
    if 1 >= len(sys.argv):
        print 'Please give p2pid first.'
    else:
        p2pid = sys.argv[1]
        devid = GetDevIDByP2pID(p2pid)
        if devid is None:
            print 'Cannt find device id by p2pid, maybe p2pid is incorrect!'
        else:
            ResetDevice(devid)
