/*
����P2P������������
*/

#pragma once
#include "P2PServerManager.h"

class P2PServerManager_SY :
    public P2PServerManager
{
public:
    P2PServerManager_SY(void);
    ~P2PServerManager_SY(void);

    //�������Ӳ���
    virtual bool ParseConnectParams( string sconnectparams );

};
