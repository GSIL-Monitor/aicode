# coding=utf-8
import mysql.connector
from mysql.connector import errorcode
import json
import memcache
import time

db = None
mc = None

DB_FAILED = 'DB_FAILED'

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
    db = ConnectDB('root', '1qaz@WSX', '47.91.178.4', 'PlatformDB')
    if db is None:
        return False
    return True


def InitMemcached():
    global mc
    mc = memcache.Client(['10.144.125.147:11211'])


def GetBlacklist(id_type=0):
    cursor = db.cursor()
    sql = "select black_id from t_user_device_blacklist where id_type = %d and status = 0" % id_type

    try:
        cursor.execute(sql)
    except mysql.connector.Error as err:
        print(err)
        db.close()
        return DB_FAILED

    if cursor.rowcount == 0:
        print("Blacklist id not founded")
        cursor.close()
        # db.close()
        return None

    blackid_list = []
    for (id,) in cursor:
        print 'Get black id is %s' % id
        blackid_list.append(id)

    cursor.close()
    return blackid_list


def GetBlacklistInfinite():
    flag = False
    while True:
        time.sleep(1.5)

        key = "device_blacklist" if flag else "user_blacklist"

        id_type = 1 if flag else 0
        blackid_list = GetBlacklist(id_type=id_type)
        flag = not flag

        yield key, blackid_list



def SetBlacklistToCache(key, blackid_list):
    strjson = json.dumps(blackid_list)
    mc.set(key, strjson, 10)


def main():

    if not InitDB():
        print ('Init db failed')
        return  False

    InitMemcached()

    for key, blackid_list in GetBlacklistInfinite():
        print ("key is %s" % key)

        if blackid_list is None:
            print 'Get black id list from database is empty.'
            #exit(0)
            continue

        if not blackid_list: #black id list is empty list
            continue

        if DB_FAILED == blackid_list:
            if not InitDB():
                print "Init db failed."
                exit(0)
            continue

        SetBlacklistToCache(key, blackid_list)


if __name__ == '__main__':
    main()
