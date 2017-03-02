#pragma once
//#include "Common.h"
#include <string>

using std::string;

//时区
typedef struct _tagCountryTime
{
    _tagCountryTime()
    {
        sCode = "CN";
        sCountryEn = "China";
        sCountryCn = "中国";
        sCountrySQ = "8";
    }
    string sCode;               //国家代码
    string sCountryEn;      //国家英文名
    string sCountryCn;  //国家中文名
    string sCountrySQ;  //国家时区
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

    //设置实时获取时区第三方接口地址
    void setpostUrl(string sUrl);
    //通过IP获取时区
    bool GetCountryTime(string sIP, TimeZone &timezone);
    //通过查找本地数据库的时区表找出指定所对应的时区
    bool GetCountryTimeFromDataBase(string sIP, TimeZone &timezone);
    //通过第三方接口获取指定IP的对应时区
    bool GetCountryTimeFromThirdInterface(string sUrl, string sIP, string &timezone);
    //解析第三方接口返回的时区信息字符串
    bool ParseTimezoneString(string sTimezoneString, TimeZone &timezone);
    //通过指定的国家代码从数据库中获取时区值
    bool GetTimezoneFromCountryCode( TimeZone &timezone );
    //插入新的IP与国家代码对应关系记录
    bool UpdateCountryTime(string sIP, string sCountryCode);

    bool GetCountryInfoByDevID(const std::string &strDevID, TimeZone &timezone);

private:

public:
    string m_sUrl;      //第三方接口获取时区信息

    DBInfoCacheManager *m_pDBCache;
    MysqlImpl *m_pMysql;
};
