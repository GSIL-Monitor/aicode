# coding=utf-8
from __future__ import print_function
import HttpHandler
import json
import random
import time

def SensorReport(sid, devid, info):
    address = 'www.xvripc.net'
    port = 8988
    url = '/passenger_flow.cgi?action=report_sensor_info'
    fields = [('sid', sid), ('deviceid', devid), ('info', info)]
    status, data = HttpHandler.PostHttp(address, port, url, fields)
    jsSensorResult = json.loads(data)
    if (200 != status):
        print('Sensor report failed and return code is %d, return msg %s, %s' %
              (status, jsSensorResult['retcode'], jsSensorResult['retmsg']))
        return False

    print('Sensor report success and device id is %s and value is %s' % (devid, info))
    return True


def GetSessionID():
    address = 'www.xvripc.net'
    port = 8888
    url = '/access.cgi?action=user_login'

    fields = [('username', 'zycai@annigroup.com'), ('userpwd', '123456'), ('terminaltype', '9'), ('type', '1')]

    status, data = HttpHandler.PostHttp(address, port, url, fields)

    jsResultData = json.loads(data)

    if (200 != status):
        print('Login server failed %d, %s, %s' % (status, jsResultData['retcode'], jsResultData['retmsg']))
        return False

    sid = jsResultData['sid']
    return sid


def GenerateSensorInfo():
    sinfolist = []
    for type in range(1, 6):
        random.seed()
        rdmvalue = random.randint(1, 100)

        sinfo = {}
        sinfo['type'] = str(type)
        sinfo['value'] = str(rdmvalue)
        sinfolist.append(sinfo)

    return json.dumps(sinfolist)



def main():
    sid = GetSessionID()
    if not sid:
        print('Get sid failed.')
        return False

    while True:
        jsSinfo = GenerateSensorInfo()

        if not SensorReport(sid, 'bi26fff68a', jsSinfo):
            break

        time.sleep(5)





if __name__ == '__main__':
    main()
