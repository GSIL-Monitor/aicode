#ifndef _P2P_SERVER_MANAGER_LT_
#define _P2P_SERVER_MANAGER_LT_

using std::string;

/*
尚云P2P服务器管理类
*/

#include "P2PServerManager.h"

class P2PServerManager_LT :
    public P2PServerManager
{
public:
    P2PServerManager_LT(const string &strSupplier);
    virtual ~P2PServerManager_LT(void);

    //解析连接参数
    virtual bool ParseConnectParams(string sconnectparams);

    //通过设备ID和设备(或用户)IP获取所属P2P集群相关参数
    virtual bool DeviceRequestP2PConnectParam(P2PConnectParam &p2pparams, string sDeviceID, string sIP, string sUserID = "");


private:
    bool GetP2pID(const string &strDeviceID, P2PConnectParam &p2pparams);
    bool AllocateP2pID(const string &strDeviceID, P2PConnectParam &p2pparams);
    bool UpdateP2PID(const string &strDeviceID, const string &strP2pID);

    static boost::mutex m_p2pMutex;
};


#endif
