#coding:utf-8

import sys
import time
import datetime

#used in python 2.6
def total_seconds(time_delta):
    return 1.0 * (time_delta.microseconds + (time_delta.seconds + time_delta.days * 24 * 3600) * 10**6) / 10**6

TimelineMap = {} #key time, value json

ProcessHandlerMap = {} #key action, value func

ActionFlag = 'find action'
UserOrDevFlag = 'find user_or_dev'
ConnectRemoteFlag = 'connect remote'
ResultFlag = 'find result'


def ParseFile(filename, begintime=None, endtime=None, uid=None):
    ShowString = ''
    userid = None
    devid = None

    Status = ActionFlag
    with open(filename, 'r') as f:
        for line in f:
            if Status == ActionFlag:
                if -1 != line.find('Param info: key=[ACTION]'):
                    posflag = line.find('[')
                    action_time = line[0 : posflag - 1]

                    if begintime is not None:
                        posflag = action_time.find('.')
                        beginaction = action_time[:posflag]
                        beginaction = datetime.datetime.strptime(beginaction,"%Y%m%d %H:%M:%S")
                        timevalue = beginaction - begintime
                        if 0 > total_seconds(timevalue): #timevalue.total_seconds():
                            continue

                    posflag = line.find('thread')
                    posflag2 = line.find(']', posflag)
                    threadnum = line[posflag + 7 : posflag2 - 1]

                    posflag = line.find('value=[')
                    posflag2 = line.find(']', posflag)
                    action = line[posflag + 7 : posflag2]

                    ShowString = 'Action: %30s, begintime: %s, threadnum: %s ' % (action, action_time, threadnum)

                    if endtime is not None:
                        posflag = action_time.find('.')
                        beginaction = action_time[:posflag]
                        beginaction = datetime.datetime.strptime(beginaction,"%Y%m%d %H:%M:%S")
                        timevalue = beginaction - endtime
                        if 0 < total_seconds(timevalue): #timevalue.total_seconds():
                            break

                    Status = UserOrDevFlag

            elif Status == UserOrDevFlag:

                if -1 != line.find('Param info: key=[devid]'):
                    posflag = line.find('value=[')
                    posflag2 = line.find(']', posflag)
                    devid = line[posflag + 7: posflag2]

                    ShowString += 'device id: %s ' % devid

                elif -1 != line.find('Param info: key=[userid]'):
                    posflag = line.find('value=[')
                    posflag2 = line.find(']', posflag)
                    userid = line[posflag + 7: posflag2]

                    ShowString += 'user   id: %s ' % userid

                elif -1 != line.find('Param info: key=[username]'):
                    posflag = line.find('value=[')
                    posflag2 = line.find(']', posflag)
                    username = line[posflag + 7: posflag2]

                    ShowString += 'user name: %s ' % username

                elif -1 != line.find('Param info: key=[REMOTE_ADDR]'):
                    posflag = line.find('value=[')
                    posflag2 = line.find(']', posflag)
                    remoteip = line[posflag + 7: posflag2]

                    ShowString += 'remote ip: %s ' % remoteip

                elif -1 != line.find('Connect succeed and remote ip'):

                    Status = ResultFlag

            elif Status == ResultFlag:
                if -1 != line.find('Return msg is writed and result is'):
                    posflag = line.find('[')
                    end_time = line[0: posflag - 1]

                    posflag = line.find('Return msg is writed and result is')
                    posflag2 = line.find('-', posflag)
                    result = line[posflag + 35 : posflag2 - 1]

                    ShowString += 'endtime %s result: %s' % (end_time, result)

                    if uid is not None:
                        if userid is not None:
                            local_uid = userid
                            userid = None

                            if uid != local_uid:
                                Status = ActionFlag  # reset loop
                                continue
                        else:
                            Status = ActionFlag  # reset loop
                            continue

                    print ShowString

                    Status = ActionFlag

def ValidInput(begintime, endtime):
    if begintime is not None and endtime is not None:
        #begintime = datetime.datetime.strptime(begintime, "%Y-%m-%d %H:%M:%S")
        #endtime = datetime.datetime.strptime(endtime, "%Y-%m-%d %H:%M:%S")
        deltatime = endtime - begintime
        delta = total_seconds(deltatime)
        if 0 > total_seconds(deltatime):  # total_seconds():
            print 'End time is less than begin time.'
            return False

    return True


if __name__ == "__main__":
    print 'Runing at ', time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(time.time()))

    if 1 >= len(sys.argv):
        print 'Please give file name first. Parse.py filename [begintime] [endtime] [userid] [deviceid]'

    else:
        begintime = None
        endtime = None
        uid = None

        if 3 <= len(sys.argv):
            begintime = sys.argv[2]
            begintime = datetime.datetime.strptime(begintime, "%Y-%m-%d %H:%M:%S")

        if 4 <= len(sys.argv):
            endtime = sys.argv[3]
            endtime = datetime.datetime.strptime(endtime, "%Y-%m-%d %H:%M:%S")

        if 5 <= len(sys.argv):
            uid = sys.argv[4]

        if (ValidInput(begintime=begintime, endtime=endtime)):

            ParseFile(sys.argv[1], begintime=begintime, endtime=endtime, uid=uid)
            print 'Completed!'






