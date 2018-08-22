# coding=utf-8
import mysql.connector
from mysql.connector import errorcode
import time

db = None
db_patrol = None

DB_FAILED = 'DB_FAILED'
PATROL_USER_TYPE = 2

ACCESS_USER_STATUS = '1'
COMPANY_USER_STATUS = '2'
ROLE_USER_STATUS = '3'

loop_status = ACCESS_USER_STATUS

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


def InitDB(IsPatrol=True):
    if IsPatrol:
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

def GetPatrolUserIDList():
    sql = "select userid from t_user_info where typeinfo = %d and status = 0" % PATROL_USER_TYPE
    return GetIDList(sql, db)

def GetAccessPatrolUserIDList():
    sql = "select userid from t_access_user_business_info"
    return GetIDList(sql, db)

def GetAllPatrolUserIDList():
    sql = "select user_id from t_company_user_info"
    return GetIDList(sql, db_patrol)

def GetAllRoleUserIDList():
    sql = "select user_id from t_user_role_association"
    return GetIDList(sql, db_patrol)

def GetPatrolIDlistInfinite():
    while True:
        time.sleep(1.5)

        PatrolUidList = GetPatrolUserIDList()
        if PatrolUidList is None or not PatrolUidList:
            print("Patrol user id list is empty.")
            continue

        if DB_FAILED == PatrolUidList:
            if not InitDB():
                print "Init db failed."
                exit(0)
            continue

        global loop_status
        local_status = loop_status
        AccessPatrolUidList = []
        if ACCESS_USER_STATUS == loop_status:
            AccessPatrolUidList = GetAccessPatrolUserIDList()
            loop_status = COMPANY_USER_STATUS

        elif COMPANY_USER_STATUS == loop_status:
            AccessPatrolUidList = GetAllPatrolUserIDList()
            loop_status = ROLE_USER_STATUS

        elif ROLE_USER_STATUS == loop_status:
            AccessPatrolUidList = GetAllRoleUserIDList()
            loop_status = ACCESS_USER_STATUS

        if AccessPatrolUidList is None:
            AccessPatrolUidList = []

        #if AccessPatrolUidList is None or not AccessPatrolUidList:
        #    print("Access patrol user id list is empty.")
        #    continue

        if DB_FAILED == AccessPatrolUidList:
            if not InitDB():
                print "Init db failed."
                exit(0)
            continue

        #In PatrolUidList and not in AccessPatrolUidList
        PatrolUidListDiff = list(set(PatrolUidList).difference(set(AccessPatrolUidList)))

        #In AccessPatrolUidList and not in PatrolUidList
        AccessPatrolUidListDiff = list(set(AccessPatrolUidList).difference(set(PatrolUidList)))

        yield PatrolUidListDiff, AccessPatrolUidListDiff, local_status

def AddAccessPatrol(uid):
    try:
        cursor = db.cursor()
        sql = 'insert into t_access_user_business_info(id, userid, access_domain, business_type)' \
              "values(uuid(), '%s', '%s', 0)" % (uid, 'http://xvripc.net:8988')
        cursor.execute(sql)
        cursor.close()
    except mysql.connector.Error as err:
        print(err)
        db.close()

        time.sleep(1.5)
        if not InitDB():
            print "Init db failed."
            exit(0)
        return DB_FAILED
    return  True

def RemoveAccessPatrol(uid):
    cursor = db.cursor()
    sql = "delete from t_access_user_business_info where userid = '%s'" % uid

    try:
        cursor.execute(sql)
    except mysql.connector.Error as err:
        print(err)
        db.close()
        return DB_FAILED

    cursor.close()
    return True

def AddCompanyUserInfo(uid):
    try:
        cursor2 = db_patrol.cursor()
        sql2 = 'insert into t_company_user_info(id, company_id, user_id, create_date)' \
               "values(uuid(), 'annidev', '%s', CURRENT_TIMESTAMP())" % uid
        cursor2.execute(sql2)
        cursor2.close()

    except mysql.connector.Error as err:
        print(err)
        db_patrol.close()

        time.sleep(1.5)
        if not InitDB():
            print "Init db failed."
            exit(0)
        return DB_FAILED
    return True

def RemoveCompanyUserInfo(uid):
    cursor2 = db_patrol.cursor()
    sql2 = "delete from t_company_user_info where user_id = '%s'" % uid
    try:
        cursor2.execute(sql2)
    except mysql.connector.Error as err:
        print(err)
        db_patrol.close()
        return DB_FAILED

    cursor2.close()
    return True

def AddRoleUserInfo(uid):
    try:
        cursor2 = db_patrol.cursor()
        sql2 = 'insert into t_user_role_association(id, user_id, role_id, create_date)' \
               "values(uuid(), '%s', 'administrator', CURRENT_TIMESTAMP())" % uid
        cursor2.execute(sql2)
        cursor2.close()

    except mysql.connector.Error as err:
        print(err)
        db_patrol.close()

        time.sleep(1.5)
        if not InitDB():
            print "Init db failed."
            exit(0)
        return DB_FAILED
    return True

def RemoveRoleUserInfo(uid):
    cursor2 = db_patrol.cursor()
    sql2 = "delete from t_user_role_association where user_id = '%s'" % uid
    try:
        cursor2.execute(sql2)
    except mysql.connector.Error as err:
        print(err)
        db_patrol.close()
        return DB_FAILED

    cursor2.close()
    return True

def main():
    if not InitDB() or not InitDB(False):
        print ('Init db failed')
        return  False

    for PatrolUidListDiff, AccessPatrolUidListDiff, status in GetPatrolIDlistInfinite():
        for puid in PatrolUidListDiff:
            result = None
            if ACCESS_USER_STATUS == status:
                result = AddAccessPatrol(puid)
            elif COMPANY_USER_STATUS == status:
                result = AddCompanyUserInfo(puid)
            elif ROLE_USER_STATUS == status:
                result = AddRoleUserInfo(puid)

            if DB_FAILED == result:
                if not InitDB():
                    print "Init db failed."
                    exit(0)
                continue
            if result:
                print("Add access patrol success.")

        for apuid in AccessPatrolUidListDiff:
            result = None

            if ACCESS_USER_STATUS == status:
                result = RemoveAccessPatrol(apuid)
            elif COMPANY_USER_STATUS == status:
                result = RemoveCompanyUserInfo(apuid)
            elif ROLE_USER_STATUS == status:
                result = RemoveRoleUserInfo(apuid)

            if DB_FAILED == result:
                if not InitDB():
                    print "Init db failed."
                    exit(0)
                continue
            if result:
                print("Remove access patrol success.")


if __name__ == '__main__':
    main()



