#include "P2PServerManager_SY.h"

P2PServerManager_SY::P2PServerManager_SY(void)
{
    m_sFlag = "SY";
    m_table_name = "t_p2pid_sy";    
}

P2PServerManager_SY::~P2PServerManager_SY(void)
{
}

bool P2PServerManager_SY::ParseConnectParams( string sconnectparams )
{
    m_p2pConnectParams.sInitstring = sconnectparams;
    return true;
}

