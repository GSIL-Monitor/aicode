#!/usr/local/bin/python
# -*- coding: utf-8 -*-
import os
import sys
import mysql.connector
from mysql.connector import errorcode
import ConfigParser
import subprocess
import threading
import time
import json
import md5
#import hashlib
import random
import re
from struct import *
# from __future__ import print_function


def db_conn(db_user, db_pwd, db_host, db_name):
    try:
        cnx = mysql.connector.connect(
            user=db_user, password=db_pwd,
            host=db_host, database=db_name, buffered=True, connection_timeout=1000)
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


def getProcessData(ServerCmdNames, serverName):
    ps = subprocess.Popen(
        ['ps', 'aux'], stdout=subprocess.PIPE).communicate()[0]
    processes = ps.split('\n')
    # this specifies the number of splits, so the splitted lines
    # will have (nfields+1) elements
    nfields = len(processes[0].split()) - 1
    retval = []
    for cmd in ServerCmdNames:
        for row in processes[1:]:
            if cmd in row:
                rs = row.split(None, nfields)
                nfields2 = len(rs[10].split()) - 1
                rs2 = rs[10].split(None, nfields2)
                if serverName != rs2[len(rs2) - 1]:
                    continue

                # For FCM Agent:
                if "_FCM" in rs2[0]:
                    did = rs2[2].split(":", 2)[0]
                # For Post/Query/Subs Server:
                elif "_SN" in rs2[0] or "_QS" in rs2[0]:
                    did = rs2[3].split(":", 2)[0]

                # (PID, DID, CMD, 0)
                retval.append((rs[1], did, rs[10], 0))

    return retval


def getCmdFromDBforRunning():
    global ServerCmdNames, QS_Cmd, SN_Cmd, FCM_Cmd, serverName
    newCmds = {}
    # DB Connect:
    cnx = db_conn(Config.get("DBConfig", "UserName"), Config.get(
        "DBConfig", "Password"), Config.get("DBConfig", "IP"), Config.get("DBConfig", "DBName"))
    if cnx is None:
        return None

    cursor = cnx.cursor()
    query = 'SELECT AG_Name, App_Name, DID, NDT_License, p.initString as NDT_Init, p.AES128Key as NDT_AES128Key, extraInfo FROM WiPN_Table as t, Server_Table as s, Prefix_Table as p WHERE s.name LIKE "{0}" AND t.Server_Id=s._id AND t.DID LIKE CONCAT(p.prefix ,"%%")'.format(
        serverName)

    try:
        cursor.execute(query)
    except mysql.connector.Error as err:
        print(err)
        return None

    if cursor.rowcount == 0:
        print("No WiPN Cmd to execute for %s." % serverName)
        cursor.close()
        return None

    for (AG_Name, App_Name, DID, NDT_License, NDT_Init, NDT_AES128Key, extraInfo) in cursor:
        # print(AG_Name, App_Name, DID, NDT_License, NDT_Init, NDT_AES128Key, extraInfo)
        if extraInfo is not None:  # or len(extraInfo) > 0
            try:
                extra = json.loads(extraInfo)
            except ValueError, e:
                # print e
                extra = None
                pass
        else:
            extra = None

        if (AG_Name == "QS"):
            cmd = "./{} {} {} {}:{} {}".format(
                QS_Cmd, NDT_Init, NDT_AES128Key, DID, NDT_License, serverName)
        elif (AG_Name == "SN"):
            cmd = "./{} {} {} {}:{} {}".format(
                SN_Cmd, NDT_Init, NDT_AES128Key, DID, NDT_License, serverName)
        elif (AG_Name == "FCM"):
            cmd = "./{} {} {}:{} {} {} {}".format(
                FCM_Cmd, extra["APIKey"], DID, NDT_License, NDT_Init, NDT_AES128Key, serverName)
        else:
            continue

        newCmds[DID] = cmd

    cursor.close()
    cnx.close()
    return newCmds


def main_loop():
    global ServerCmdNames, QS_Cmd, SN_Cmd, FCM_Cmd, serverName

    lastCleanLogTime = 0
    lastUpdateCmdTime = 0
    lastCalLoadingTime = time.time()
    while(True):
        # Clean Log files every 24hr, Only keep 2 weeks logs.
        if (time.time() - lastCleanLogTime) > 86400:  # 24hr
            try:
                print("Clean Log for mtime > 14 days")
                os.system('find ./*Log*/*.log -mtime +14 -exec rm -f {} \;')
            except ValueError, e:
                print("")

            lastCleanLogTime = time.time()

        # Update Cmds from DB every 5 min.
        if (time.time() - lastUpdateCmdTime) > 5*60:  # 5min
            newCmds = getCmdFromDBforRunning()

        # Update psCmds from System every round.
        psCmds = getProcessData(ServerCmdNames, serverName)

        if len(psCmds) == 0:
            # Run all newCmds:
            for did, cmd in newCmds.iteritems():
                print(" [+] %s" % cmd[:])
                os.system(cmd+"&")
                time.sleep(0.1)
        else:
            # Compare psCmds & newCmds:
            for did, cmd in newCmds.iteritems():
                found = False
                for runningCmd in psCmds:
                    # runningCmd -> (PID, DID, CMD, 0)
                    if did == runningCmd[1]:
                        found = True
                        if md5.new(cmd).digest() != md5.new(runningCmd[2]).digest():
                            os.system("kill %s" % runningCmd[0])
                            print(" [-] %s" % runningCmd[2][:])
                            time.sleep(2)
                            print(" [+] %s" % cmd[:])
                            os.system(cmd+"&")
                # Run new Cmd:
                if found is False:
                    print(" [+] %s" % cmd[:])
                    os.system(cmd+"&")
                    time.sleep(0.2)

            # Find abandon cmds:
            for runningCmd in psCmds:
                found = False
                for did, cmd in newCmds.iteritems():
                    if did == runningCmd[1]:
                        found = True
                        break
                if found is False:
                    os.system("kill %s" % runningCmd[0])
                    print(" [-] %s" % runningCmd[2])
                    time.sleep(0.2)

        # Calculate SN Loading every 10 min.
        # if (time.time() - lastCalLoadingTime) > 5 * 60: # 10min

        # psCmds = getProcessData(ServerCmdNames, serverName)
        # snCount = 0
        # snProcCount = 0.0
        # for runningCmd in psCmds:
        #     # print runningCmd
        #     #  runningCmd -> (PID, DID, CMD, 0)

        #     if "_QS" in runningCmd[2]:
        #         try:
        #             f = open("/dev/shm/QS_" + runningCmd[1] + "-Count.log", "rb")
        #             data = f.read(72) # version(4) + utct(4) + 10 count
        #             val = unpack(('>IIIIIIIIIIIIIIIIII'), data)
        #             # print "<QS>", runningCmd[1], format(val[1], '08x'),val[8:]
        #         except Exception:
        #             val = 0
        #             # print "<QS>", runningCmd[1], format(val, '08x'),val
        #             pass

        #     if "_SN" in runningCmd[2]:
        #         snCount += 1
        #         try:
        #             f = open("/dev/shm/SN_" + runningCmd[1] + "-PCount.log", "rb")
        #             data = f.read(72) # version(4) + utct(4) + 10 count
        #             val = unpack(('>IIIIIIIIIIIIIIIIII'), data)
        #             # print "<SN>", runningCmd[1], format(val[1], '08x'),val[8:]
        #             snProcCount += float(sum(val[8:]))/float(10)
        #         except Exception:
        #             val = 0
        #             # print "<SN>", runningCmd[1], format(val, '08x'),val
        #             snProcCount += 0
        #             pass

        # print snProcCount, snCount
        # result = round(snProcCount/snCount, 3)
        # print "Average Loading:", result, "p/s for", snCount, "SN."
            # lastCalLoadingTime = time.time()

        time.sleep(15)


if __name__ == "__main__":
    global Config, ServerCmdNames, QS_Cmd, SN_Cmd, FCM_Cmd, serverName

    if(len(sys.argv) < 2):
        print("Usage:\n {} ServerName\n".format(sys.argv[0]))
        sys.exit()

    serverName = sys.argv[1]
    machine = subprocess.Popen(
        ["file", "/sbin/init"], stdout=subprocess.PIPE).communicate()[0]
    x86 = re.search('80386', machine)

    # Parsing Configure file for cmd name:
    Config = ConfigParser.ConfigParser()
    Config.read("WiPN.conf")
    if x86:
        QS_Cmd = Config.get("ServerCmd", "QS")
        SN_Cmd = Config.get("ServerCmd", "SN")
        FCM_Cmd = Config.get("ServerCmd", "FCM")
    else:
        QS_Cmd = Config.get("ServerCmd64", "QS64")
        SN_Cmd = Config.get("ServerCmd64", "SN64")
        FCM_Cmd = Config.get("ServerCmd64", "FCM64")

    ServerCmdNames = []
    ServerCmdNames.append(QS_Cmd)
    ServerCmdNames.append(SN_Cmd)
    ServerCmdNames.append(FCM_Cmd)

    main = threading.Thread(target=main_loop(), name="main loop")
    main.start()

    while 1:
        pass
