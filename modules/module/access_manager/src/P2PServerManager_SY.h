#ifndef _P2P_SERVER_MANAGER_SY_
#define _P2P_SERVER_MANAGER_SY_

/*
尚云P2P服务器管理类
*/

#include "P2PServerManager.h"

class P2PServerManager_SY :
    public P2PServerManager
{
public:
    P2PServerManager_SY(void);
    ~P2PServerManager_SY(void);

    //解析连接参数
    virtual bool ParseConnectParams( string sconnectparams );

};


#endif
