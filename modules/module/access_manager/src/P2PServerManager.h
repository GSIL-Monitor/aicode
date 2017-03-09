#ifndef _P2P_SERVER_NANAGER_
#define  _P2P_SERVER_NANAGER_

#include <boost/thread/mutex.hpp>
#include "TimeZone.h"
#include "DBInfoCacheManager.h"

using std::string;

class MysqlImpl;

//P2P连接目标参数
typedef struct _tagP2PConnectParam
{
    string sInitstring; //连接服务器的初始化字符串
    string sP2Pid;      //分配给设备的P2PID
    string sparam1;		//对于尚云来说，这是license值，对于其他类型的p2pid，则可能是其他值
	string sparam2;		//对于尚云来说，这是pushid值
    int nTime;              //租约时间
}P2PConnectParam;

//P2P服务器信息
typedef struct _tagP2PServerInfo
{
    string sIP;
    int nPort;
    string domain;
    string countrycode;
    string cluster;
    string flag;
    string connectparams;//用 | 进行分隔
    int status;
}P2PServerInfo;

class P2PServerManager
{
public:
    P2PServerManager(void);
    ~P2PServerManager(void);

    //设置获取区域的第三方接口地址
    void SetUrl(string sUrl);

    void SetDBManager(DBInfoCacheManager *pDBCache, MysqlImpl *pMysql);

    //通过设备ID和设备(或用户)IP获取所属P2P集群相关参数
    virtual bool DeviceRequestP2PConnectParam(P2PConnectParam &p2pparams, string sDeviceID, string sIP, string sUserID = "");

    //通过区域参数给指定的设备分配相应集群的P2PID
    virtual bool AllocP2PID(TimeZone countryTime, string sDevID);
    //解除设备绑定的P2PID
    virtual bool ReleaseP2PID(string sDevID);
    //获取指定设备所关联的P2PID
    virtual bool GetP2PID(string sDevID, bool bGetTimeZone, TimeZone &timezone);

    virtual bool GetP2PServerFromTimezone(TimeZone countryTime, P2PServerInfo &p2pServerInfo);

    //解析P2P连接参数
    virtual bool ParseConnectParams(string sconnectparams) = 0;

private:
    bool GetP2pIDByDevID(const std::string &strDevID, P2PConnectParam &p2pparams);
    bool GetFreeP2pID(const std::string &strCnCode, const std::string &strP2pIDTableName, P2PConnectParam &p2pparams);
    bool UpdateP2PID(const std::string &p2pid, const std::string &strDeviceID);

public:
    string m_sFlag;
    CTimeZone m_timezone;
    P2PConnectParam m_p2pConnectParams;
    //p2pid数据表名
    string m_table_name;
    static boost::mutex m_smutex;


private:
    DBInfoCacheManager *m_pDBCache;
    MysqlImpl *m_pMysql;
};


#endif
