#pragma once
//#include "Common.h"
#include <string>

using std::string;

//ʱ��
typedef struct _tagCountryTime
{
    _tagCountryTime()
    {
        sCode = "CN";
        sCountryEn = "China";
        sCountryCn = "�й�";
        sCountrySQ = "8";
    }
    string sCode;               //���Ҵ���
    string sCountryEn;      //����Ӣ����
    string sCountryCn;  //����������
    string sCountrySQ;  //����ʱ��
}TimeZone;
typedef struct _tagCountryInfo
{
    string country;
    string country_id;
    string area;
    string area_id;
    string region;
    string region_id;
    string city;
    string city_id;
    string county;
    string county_id;
    string isp;
    string isp_id;
    string ip;
}CountryInfo;

class DBInfoCacheManager;
class MysqlImpl;

class CTimeZone
{
public:
    CTimeZone(void);
    ~CTimeZone(void);

    void SetDBManager(DBInfoCacheManager *pDBCache, MysqlImpl *pMysql);

    //����ʵʱ��ȡʱ���������ӿڵ�ַ
    void setpostUrl(string sUrl);
    //ͨ��IP��ȡʱ��
    bool GetCountryTime(string sIP, TimeZone &timezone);
    //ͨ�����ұ������ݿ��ʱ�����ҳ�ָ������Ӧ��ʱ��
    bool GetCountryTimeFromDataBase(string sIP, TimeZone &timezone);
    //ͨ���������ӿڻ�ȡָ��IP�Ķ�Ӧʱ��
    bool GetCountryTimeFromThirdInterface(string sUrl, string sIP, string &timezone);
    //�����������ӿڷ��ص�ʱ����Ϣ�ַ���
    bool ParseTimezoneString(string sTimezoneString, TimeZone &timezone);
    //ͨ��ָ���Ĺ��Ҵ�������ݿ��л�ȡʱ��ֵ
    bool GetTimezoneFromCountryCode( TimeZone &timezone );
    //�����µ�IP����Ҵ����Ӧ��ϵ��¼
    bool UpdateCountryTime(string sIP, string sCountryCode);

    bool GetCountryInfoByDevID(const std::string &strDevID, TimeZone &timezone);

private:

public:
    string m_sUrl;      //�������ӿڻ�ȡʱ����Ϣ

    DBInfoCacheManager *m_pDBCache;
    MysqlImpl *m_pMysql;
};
