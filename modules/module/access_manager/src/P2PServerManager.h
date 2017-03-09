#ifndef _P2P_SERVER_NANAGER_
#define  _P2P_SERVER_NANAGER_

#include <boost/thread/mutex.hpp>
#include "TimeZone.h"
#include "DBInfoCacheManager.h"

using std::string;

class MysqlImpl;

//P2P����Ŀ�����
typedef struct _tagP2PConnectParam
{
    string sInitstring; //���ӷ������ĳ�ʼ���ַ���
    string sP2Pid;      //������豸��P2PID
    string sparam1;		//����������˵������licenseֵ�������������͵�p2pid�������������ֵ
	string sparam2;		//����������˵������pushidֵ
    int nTime;              //��Լʱ��
}P2PConnectParam;

//P2P��������Ϣ
typedef struct _tagP2PServerInfo
{
    string sIP;
    int nPort;
    string domain;
    string countrycode;
    string cluster;
    string flag;
    string connectparams;//�� | ���зָ�
    int status;
}P2PServerInfo;

class P2PServerManager
{
public:
    P2PServerManager(void);
    ~P2PServerManager(void);

    //���û�ȡ����ĵ������ӿڵ�ַ
    void SetUrl(string sUrl);

    void SetDBManager(DBInfoCacheManager *pDBCache, MysqlImpl *pMysql);

    //ͨ���豸ID���豸(���û�)IP��ȡ����P2P��Ⱥ��ز���
    virtual bool DeviceRequestP2PConnectParam(P2PConnectParam &p2pparams, string sDeviceID, string sIP, string sUserID = "");

    //ͨ�����������ָ�����豸������Ӧ��Ⱥ��P2PID
    virtual bool AllocP2PID(TimeZone countryTime, string sDevID);
    //����豸�󶨵�P2PID
    virtual bool ReleaseP2PID(string sDevID);
    //��ȡָ���豸��������P2PID
    virtual bool GetP2PID(string sDevID, bool bGetTimeZone, TimeZone &timezone);

    virtual bool GetP2PServerFromTimezone(TimeZone countryTime, P2PServerInfo &p2pServerInfo);

    //����P2P���Ӳ���
    virtual bool ParseConnectParams(string sconnectparams) = 0;

private:
    bool GetP2pIDByDevID(const std::string &strDevID, P2PConnectParam &p2pparams);
    bool GetFreeP2pID(const std::string &strCnCode, const std::string &strP2pIDTableName, P2PConnectParam &p2pparams);
    bool UpdateP2PID(const std::string &p2pid, const std::string &strDeviceID);

public:
    string m_sFlag;
    CTimeZone m_timezone;
    P2PConnectParam m_p2pConnectParams;
    //p2pid���ݱ���
    string m_table_name;
    static boost::mutex m_smutex;


private:
    DBInfoCacheManager *m_pDBCache;
    MysqlImpl *m_pMysql;
};


#endif
