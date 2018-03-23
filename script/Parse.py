#coding=utf-8

import sys
import time
import datetime

"""
import chardet
def _smartCode(item):
    codedetect = chardet.detect(item)["encoding"]
    print codedetect
    try:
        print item
        item = unicode(item, codedetect)
        print item
        return item.encode("utf-8")
    except:
        return u"bad unicode encode try!"
"""

ipdatamap = {}

def IPDataInit():
    with open('iplocal.txt', 'r') as f:
        for line in f:
            #ipdatalist.append(line)
            addressinfo = line.split()
            if not addressinfo:
                continue

            beginaddress = addressinfo.pop(0)
            beginaddresslist = beginaddress.split('.')
            b1 = int(beginaddresslist.pop(0))

            linevalue = ipdatamap.get(b1)
            if linevalue is None:
                linevalue = []
                linevalue.append(line)
                ipdatamap[b1] = linevalue
            else:
                linevalue.append(line)


def GetCountryByIP(ipaddress):
    #with open('iplocal.txt', 'r') as f:
    ipaddresslist = ipaddress.split('.')
    ip1 = int(ipaddresslist.pop(0))

    linevalue = ipdatamap.get(ip1)
    if linevalue is None:
        return None

    for line in linevalue:#f:
        addressinfo = line.split()
        beginaddress = addressinfo.pop(0)
        endaddress = addressinfo.pop(0)

        countryinfo = ' '.join(addressinfo)
        localcountry = countryinfo.decode('gbk').encode('utf-8')

        ipaddresslist = ipaddress.split('.')
        ip1 = int(ipaddresslist.pop(0))
        ip2 = int(ipaddresslist.pop(0))
        ip3 = int(ipaddresslist.pop(0))
        ip4 = int(ipaddresslist.pop(0))

        beginaddresslist = beginaddress.split('.')
        b1 = int(beginaddresslist.pop(0))
        b2 = int(beginaddresslist.pop(0))
        b3 = int(beginaddresslist.pop(0))
        b4 = int(beginaddresslist.pop(0))

        endaddresslist = endaddress.split('.')
        e1 = int(endaddresslist.pop(0))
        e2 = int(endaddresslist.pop(0))
        e3 = int(endaddresslist.pop(0))
        e4 = int(endaddresslist.pop(0))

        if ip1 > b1:
            continue
        elif ip2 > b2 and ip2 > e2:
            continue
        elif ip3 > b3 and ip3 > e3:
            continue
        elif ip4 > b4 and ip4 > e4:
            continue

        return localcountry

    return None


#used in python 2.6
def total_seconds(time_delta):
    return 1.0 * (time_delta.microseconds + (time_delta.seconds + time_delta.days * 24 * 3600) * 10**6) / 10**6

def total_microseconds(time_delta):
    return 1.0 * (time_delta.microseconds + (time_delta.seconds + time_delta.days * 24 * 3600) * 10 ** 6) / 1000

ActionFlag = 'find action'
UserOrDevFlag = 'find user_or_dev'
ConnectRemoteFlag = 'connect remote'
ResultFlag = 'find result'


class Parse(object):
    def __init__(self, threadnum):
        self.threadnum = threadnum

        self.ShowString = ''
        self.userid = None
        self.devid = None
        self.beginaction_micro = None
        self.Status = ActionFlag

    def ParseFile(self, line, begintime=None, endtime=None, uid=None, cnflag=None):

        if self.Status == ActionFlag:
            if -1 != line.find('Param info: key=[ACTION]'):
                posflag = line.find('[')
                action_time = line[0 : posflag - 1]

                tmpvalue = action_time[:posflag].split('.')
                value = tmpvalue.pop(0) + '.' + tmpvalue.pop(0) + tmpvalue.pop(0)
                self.beginaction_micro = datetime.datetime.strptime(value, "%Y%m%d %H:%M:%S.%f")

                if begintime is not None:
                    posflag = action_time.find('.')
                    beginaction = action_time[:posflag]
                    beginaction = datetime.datetime.strptime(beginaction,"%Y%m%d %H:%M:%S")
                    timevalue = beginaction - begintime
                    if 0 > total_seconds(timevalue): #timevalue.total_seconds():
                        return 'continue'

                posflag = line.find('thread')
                posflag2 = line.find(']', posflag)
                threadnum = line[posflag + 7 : posflag2 - 1]

                posflag = line.find('value=[')
                posflag2 = line.find(']', posflag)
                action = line[posflag + 7 : posflag2]

                self.ShowString = 'Action: %30s, begintime: %s, threadnum: %s ' % (action, action_time, threadnum)

                if endtime is not None:
                    posflag = action_time.find('.')
                    beginaction = action_time[:posflag]
                    beginaction = datetime.datetime.strptime(beginaction,"%Y%m%d %H:%M:%S")
                    timevalue = beginaction - endtime
                    if 0 < total_seconds(timevalue): #timevalue.total_seconds():
                        return 'break'

                self.Status = UserOrDevFlag

        elif self.Status == UserOrDevFlag:

            if -1 != line.find('Param info: key=[devid]'):
                posflag = line.find('value=[')
                posflag2 = line.find(']', posflag)
                self.devid = line[posflag + 7: posflag2]

                self.ShowString += 'device id: %s ' % self.devid

            elif -1 != line.find('Param info: key=[userid]'):
                posflag = line.find('value=[')
                posflag2 = line.find(']', posflag)
                self.userid = line[posflag + 7: posflag2]

                self.ShowString += 'user   id: %s ' % self.userid

            elif -1 != line.find('Param info: key=[username]'):
                posflag = line.find('value=[')
                posflag2 = line.find(']', posflag)
                username = line[posflag + 7: posflag2]

                self.ShowString += 'user name: %s ' % username

            elif -1 != line.find('Param info: key=[REMOTE_ADDR]'):
                posflag = line.find('value=[')
                posflag2 = line.find(']', posflag)
                remoteip = line[posflag + 7: posflag2]
                self.ShowString += 'remote ip: %s ' % remoteip

                if '1' == cnflag:
                    localcountry = GetCountryByIP(remoteip)
                    self.ShowString += 'country: %s ' % localcountry

            elif -1 != line.find('Return msg is writed and result is'):
                posflag = line.find('[')
                end_time = line[0: posflag - 1]

                posflag = line.find('Return msg is writed and result is')
                posflag2 = line.find('-', posflag)
                result = line[posflag + 35 : posflag2 - 1]

                self.ShowString += 'endtime %s result: %s' % (end_time, result)

                tmpvalue = end_time.split('.')
                value = tmpvalue.pop(0) + '.' + tmpvalue.pop(0) + tmpvalue.pop(0)

                endaction_micro = datetime.datetime.strptime(value, "%Y%m%d %H:%M:%S.%f")
                deltatime = endaction_micro - self.beginaction_micro
                delta = total_microseconds(deltatime)
                self.ShowString += 'used microseconds:%.3f' % delta

                if uid is not None:
                    if self.userid is not None:
                        local_uid = self.userid
                        self.userid = None

                        if uid != local_uid:
                            self.Status = ActionFlag  # reset loop
                            return 'continue'
                    else:
                        self.Status = ActionFlag  # reset loop
                        return 'continue'

                print self.ShowString

                self.Status = ActionFlag

            elif -1 != line.find('Connect succeed and remote ip'):

                self.Status = ResultFlag

        elif self.Status == ResultFlag:
            if -1 != line.find('Return msg is writed and result is'):
                posflag = line.find('[')
                end_time = line[0: posflag - 1]

                posflag = line.find('Return msg is writed and result is')
                posflag2 = line.find('-', posflag)
                result = line[posflag + 35 : posflag2 - 1]

                self.ShowString += 'endtime %s result: %s ' % (end_time, result)

                tmpvalue = end_time.split('.')
                value = tmpvalue.pop(0) + '.' + tmpvalue.pop(0) + tmpvalue.pop(0)

                endaction_micro = datetime.datetime.strptime(value, "%Y%m%d %H:%M:%S.%f")
                deltatime = endaction_micro - self.beginaction_micro
                delta = total_microseconds(deltatime)
                self.ShowString += 'used microseconds:%.3f' % delta

                if uid is not None:
                    if self.userid is not None:
                        local_uid = self.userid
                        self.userid = None

                        if uid != local_uid:
                            self.Status = ActionFlag  # reset loop
                            return 'continue'
                    else:
                        self.Status = ActionFlag  # reset loop
                        return 'continue'

                print self.ShowString

                self.Status = ActionFlag
        else:
            print 'Parse status error'
            return False

        return True

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

def Run(filename, begintime, endtime, uid, cnflag):
    ParseMap = {} #key:threadnum, value Parse object
    with open(filename, 'r') as f:
        for line in f:
            posflag = line.find('thread')
            posflag2 = line.find(']', posflag)
            threadnum = line[posflag + 7: posflag2 - 1]

            pobject = ParseMap.get(threadnum)
            if pobject is None:
                pobject = Parse(threadnum)
                ParseMap[threadnum] = pobject

            result = pobject.ParseFile(line, begintime, endtime, uid, cnflag)
            if result == 'continue':
                continue
            elif result == 'break':
                break
            elif result == True:
                pass
            else:
                break

def ReadLineInfinite(filename):
    with open(filename, 'r') as f:
        f.seek(0, 2)  # Go to the end of the file
        while True:
            line = f.readline()
            if not line:
                #print 'sleep...'
                time.sleep(0.5)  # Sleep briefly
                continue
            yield line

def RunInfinite(filename, begintime, endtime, uid, cnflag):

    LineInfinite = ReadLineInfinite(filename)

    ParseMap = {}  # key:threadnum, value Parse object
    for line in LineInfinite:
        posflag = line.find('thread')
        posflag2 = line.find(']', posflag)
        threadnum = line[posflag + 7: posflag2 - 1]

        pobject = ParseMap.get(threadnum)
        if pobject is None:
            pobject = Parse(threadnum)
            ParseMap[threadnum] = pobject

        result = pobject.ParseFile(line, begintime, endtime, uid, cnflag)
        if result == 'continue':
            continue
        elif result == 'break':
            break
        elif result == True:
            pass
        else:
            break

if __name__ == "__main__":
    print 'Runing at ', time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(time.time()))

    if 1 >= len(sys.argv):
        print 'Please give file name first. Parse.py filename [infinite] [cnflag] [begintime] [endtime] [userid]'

    else:
        begintime = None
        endtime = None
        uid = None
        cnflag = None
        infinite = None

        if 3 <= len(sys.argv):
            infinite = sys.argv[2]

        if 4 <= len(sys.argv):
            cnflag = sys.argv[3]

        if 5 <= len(sys.argv):
            begintime = sys.argv[4]
            begintime = datetime.datetime.strptime(begintime, "%Y-%m-%d %H:%M:%S")

        if 6 <= len(sys.argv):
            endtime = sys.argv[5]
            endtime = datetime.datetime.strptime(endtime, "%Y-%m-%d %H:%M:%S")

        if 7 <= len(sys.argv):
            uid = sys.argv[6]

        if ValidInput(begintime=begintime, endtime=endtime):
            if '1' == cnflag:
                IPDataInit()

            if '0' == infinite:
                Run(sys.argv[1], begintime=begintime, endtime=endtime, uid=uid, cnflag=cnflag)
            else:
                RunInfinite(sys.argv[1], begintime=begintime, endtime=endtime, uid=uid, cnflag=cnflag)

            print 'Completed!'






