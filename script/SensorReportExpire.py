# coding=utf-8
import mysql.connector
from mysql.connector import errorcode
import time

db = None
db_patrol = None

DB_FAILED = 'DB_FAILED'

REMOVE_THRESHOLD = 3000

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


def InitDB(IsPatrol=False):
    if not IsPatrol:
        global db
        db = ConnectDB('root', '1qaz@WSX', '47.91.178.4', 'PlatformDB')
        if db is None:
            return False
        return True
    else:
        global db_patrol
        db_patrol = ConnectDB('root', '1qaz@WSX', '47.91.178.4', 'CustomerFlowDB')
        if db_patrol is None:
            return False
        return True

def GetIDList(sql, db_local):
    cursor = db_local.cursor()

    try:
        cursor.execute(sql)
    except mysql.connector.Error as err:
        print(err)
        db_local.close()
        return DB_FAILED

    if cursor.rowcount == 0:
        print("id not founded")
        cursor.close()
        # db.close()
        return None

    IDList = []
    for (id,) in cursor:
        print 'Get id is %s' % id
        IDList.append(id)

    cursor.close()
    return IDList

def GetReportSensorIDList():
    sql = "select distinct sensor_id from t_sensor_value"
    return GetIDList(sql, db_patrol)

def GetSensorReportCount(sensorid):
    sql = "select count(id) from t_sensor_value where sensor_id ='%s'" % sensorid
    idlist = GetIDList(sql, db_patrol)
    if idlist is None:
        idlist = []
    if DB_FAILED == idlist:
        if not InitDB(True):
            print "Init db failed."
            exit(0)
        idlist = []

    if idlist:
        sr_count = int(idlist.pop())
        return sr_count
    return 0

def RemoveReportExpired(sensorid, count):
    sql = "delete from t_sensor_value where sensor_id = '%s' " \
          "order by create_date asc limit %d" % (sensorid, count)

    try:
        cursor = db_patrol.cursor()
        cursor.execute(sql)
        cursor.close()
    except mysql.connector.Error as err:
        print(err)
        db.close()
        return DB_FAILED
    return True

def GetReportSensorIDListInfinite():
    while True:
        time.sleep(1.5)

        sensoridlist = GetReportSensorIDList()
        if sensoridlist is None or not sensoridlist:
            print("Sensor id id list is empty.")
            continue

        if DB_FAILED == sensoridlist:
            if not InitDB(True):
                print "Init db failed."
                exit(0)
            continue

        for sensorid in sensoridlist:
            yield sensorid


def main():
    if not InitDB(True):
        print ('Init db failed')
        return False

    for sensorid in GetReportSensorIDListInfinite():
        count = GetSensorReportCount(sensorid)
        if not count:
            continue

        need_delete_count = 0
        if REMOVE_THRESHOLD < count:
            need_delete_count = count - REMOVE_THRESHOLD
        else:
            continue

        result = RemoveReportExpired(sensorid, need_delete_count)

        if DB_FAILED == result:
            if not InitDB(True):
                print "Init db failed."
                exit(0)
            continue
        if result:
            print("Remove sensor report expired success.")

if __name__ == '__main__':
    main()









