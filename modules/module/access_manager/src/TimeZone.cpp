//#include "StdAfx.h"
#include <list>
#include "TimeZone.h"
#include "json/json.h"
#include "DBInfoCacheManager.h"
#include "mysql_impl.h"
#include "LogRLD.h"
#include "HttpClient.h"

CTimeZone::CTimeZone(void)
{
    m_sUrl = "";
}

CTimeZone::~CTimeZone(void)
{
}

void CTimeZone::SetDBManager(DBInfoCacheManager *pDBCache, MysqlImpl *pMysql)
{
    m_pDBCache = pDBCache;
    m_pMysql = pMysql;
}

//ͨ��IP��ȡʱ��
bool CTimeZone::GetCountryTime( string sIP, TimeZone &timezone)
{
    bool bRet = true;
    //�Ȳ��ұ������ݿ�
    bRet = GetCountryTimeFromDataBase(sIP, timezone);
    if (!bRet)
    {
        //���õ������ӿڻ�ȡʱ����Ϣ��http://ip.taobao.com/service/getIpInfo.php?ip=sIP
        string sCountryInfo;
        bRet = GetCountryTimeFromThirdInterface(m_sUrl, sIP, sCountryInfo);
        if (bRet)
        {
            if (ParseTimezoneString(sCountryInfo, timezone))
            {
                bRet = GetTimezoneFromCountryCode(timezone);
                UpdateCountryTime(sIP, timezone.sCode);
            }
        }
    }
 
    return bRet;
}

//ͨ�����ұ������ݿ��ʱ�����ҳ�ָ������Ӧ��ʱ��
bool CTimeZone::GetCountryTimeFromDataBase( string sIP, TimeZone &timezone)
{
    if (NULL == m_pDBCache)
    {
        LOG_ERROR_RLD("DBCache is null.");
        return false;
    }

    if (NULL == m_pMysql)
    {
        LOG_ERROR_RLD("Mysql is null.");
        return false;
    }

    ////
    //if (!m_pMysql->QueryExec(std::string("SET NAMES utf8")))
    //{
    //    LOG_ERROR_RLD("Insert t_ip_country sql exec failed, sql is " << "SET NAMES utf8");
    //    return false;
    //}

    char sql[1024] = { 0 };
    const char* sqlfmt = "select countrycode, country_en, country_cn, countrySQ from t_timezone_info where countrycode = (select countrycode from t_ip_country where ip='%s' limit 1)";    
    snprintf(sql, sizeof(sql), sqlfmt, sIP.c_str());

    bool bRet = false;
    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn)
    {
        switch (uiColumnNum)
        {
        case 0:
            timezone.sCode = strColumn;
            break;
        case 1:
            timezone.sCountryEn = strColumn;
            break;
        case 2:
            timezone.sCountryCn = strColumn;
            break;
        case 3:
            timezone.sCountrySQ = strColumn;
            /*Result = timezone;*/
            bRet = true;
            break;
        default:
            LOG_ERROR_RLD("Unknown sql cb error, uiRowNum:" << uiRowNum << " uiColumnNum:" << uiColumnNum << " strColumn:" << strColumn);
            break;
        }

    };

    if (!m_pMysql->QueryExec(std::string(sql), SqlFunc))
    {
        LOG_ERROR_RLD("GetCountryInfoByIp sql failed, sql is " << sql);
        return false;
    }

    return bRet;
}

//ͨ���������ӿڻ�ȡָ��IP�Ķ�Ӧʱ��
bool CTimeZone::GetCountryTimeFromThirdInterface( string sUrl, string sIP, string &timezone)
{
    //��װHttp�����ķ��͵�sUrl

            //C#����
            //private string GetCountryPost(string sendUrl1, string DevIpAddress)
            //{
            //  //post
            //  string strResult = string.Empty;            
            //  bool ret = true;
            //  //if (!UrlIsValid(sendUrl1))
            //  //{
            //  //    ret = false;
            //  //}
            //  if (ret)
            //  {
            //      //string postdata ="url=" + HttpContext.Current.Request.Url.Host.ToString() +"";
            //      string postdata = "ip=" + DevIpAddress;
            //      UTF8Encoding code = new UTF8Encoding();����//�������UTF8���뷽ʽ
            //      //string postData = "aa=iceapple.net&bb=yibin.net"; //����Ҫpost������ 
            //      byte[] data = code.GetBytes(postdata);
            //      HttpWebRequest request = (HttpWebRequest)WebRequest.Create(sendUrl1);
            //      request.Method = "POST";
            //      request.ContentType = "application/x-www-form-urlencoded"; //�����ContentType����Ҫ! 
            //      request.ContentLength = data.Length;
            //      request.UserAgent = "Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1; SV1; .NET CLR 2.0.50727)";//�������CLR�汾��NT�汾�ʵ��޸ġ�
            //      request.KeepAlive = false;
            //      request.ServicePoint.ConnectionLimit = 1000;
            //      System.Net.ServicePointManager.DefaultConnectionLimit = 1000;
            //      using (Stream stream = request.GetRequestStream()) //��ȡ������,�����ǿ�д��� 
            //      {
            //          stream.Write(data, 0, data.Length); //���������� 
            //          stream.Close();
            //          HttpWebResponse response = (HttpWebResponse)request.GetResponse();
            //          StreamReader reader = new StreamReader(response.GetResponseStream());
            //          strResult = reader.ReadToEnd();
            //          reader.Close();
            //          response.Close();
            //      }
            //      request.Abort();
            //  }
            //  return strResult;
            //}

    std::string::size_type pos = sUrl.find("|");
    std::string strGetUrl = sUrl.substr(0, pos) + "?ip=" + sIP;

    std::list<std::string> headerList;
    headerList.push_back(sUrl.substr(pos + 1));

    HttpClient httpClient;
    if (CURLE_OK != httpClient.HttpsGet(strGetUrl, headerList, timezone))
    {
        LOG_ERROR_RLD("GetCountryTimeFromThirdInterface get failed, url is " << strGetUrl);
        return false;
    }

    LOG_INFO_RLD("GetCountryTimeFromThirdInterface url is " << strGetUrl << " and response data: " << timezone);
            
    return true;
}

void CTimeZone::setpostUrl(string sUrl)
{
    m_sUrl = sUrl;
}

bool CTimeZone::GetCountryInfoByDevID( const std::string &strDevID, TimeZone &timezone)
{
    if (NULL == m_pDBCache)
    {
        LOG_ERROR_RLD("DBCache is null.");
        return false;
    }

    char sql[1024] = { 0 };
    const char* sqlfmt = "select countrycode, country_en, country_cn, countrySQ from t_timezone_info where countrycode = (select countrycode from t_p2pserver_info where cluster = (select cluster from t_p2pid_sy where deviceid='%s' limit 1))";
    snprintf(sql, sizeof(sql), sqlfmt, strDevID.c_str());

    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &Result)
    {
        switch (uiColumnNum)
        {
        case 0:
            timezone.sCode = strColumn;
            break;
        case 1:
            timezone.sCountryEn = strColumn;
            break;
        case 2:
            timezone.sCountryCn = strColumn;
            break;
        case 3:
            timezone.sCountrySQ = strColumn;
            Result = timezone;
            break;
        default:
            LOG_ERROR_RLD("Unknown sql cb error, uiRowNum:" << uiRowNum << " uiColumnNum:" << uiColumnNum << " strColumn:" << strColumn);
            break;
        }

    };

    std::list<boost::any> ResultList;
    if (!m_pDBCache->QuerySql(std::string(sql), ResultList, SqlFunc))
    {
        LOG_ERROR_RLD("GetCountryInfoByDevID sql failed, sql is " << sql);
        return false;
    }

    if (ResultList.empty())
    {
        LOG_INFO_RLD("GetCountryInfoByDevID info not found, sql is " << sql);
        return false;
    }

    return true;
}

bool CTimeZone::UpdateCountryTime( string sIP, string sCountryCode )
{

    //��������insert into t_ip_country (id,ip,countrycode) values (uuid(),'XX', 'YY')

    char sql[1024] = { 0 };
    const char* sqlfmt = "insert t_ip_country (id,ip,countrycode) values (uuid(), '%s', '%s') on duplicate key update updatetime=current_timestamp, visitecount=visitecount+1 ";
    //const char* sqlfmt = "insert into t_ip_country (id,ip,countrycode) values (uuid(),'%s', '%s')";
    snprintf(sql, sizeof(sql), sqlfmt, sIP.c_str(), sCountryCode.c_str());

    if (!m_pMysql->QueryExec(std::string(sql)))
    {
        LOG_ERROR_RLD("Insert t_ip_country sql exec failed, sql is " << sql);
        return false;
    }

    return true;
}

bool CTimeZone::ParseTimezoneString( string sTimezoneString, TimeZone &timezone)
{
    //{
    //    "code":0,
    //    "data":
    //    {
    //        "country":"\u4e2d\u56fd",
    //        "country_id":"CN",
    //        "area":"\u534e\u5357",
    //        "area_id":"800000",
    //        "region":"\u5e7f\u4e1c\u7701",
    //        "region_id":"440000",
    //        "city":"\u6df1\u5733\u5e02",
    //        "city_id":"440300",
    //        "county":"",
    //        "county_id":"-1",
    //        "isp":"\u7535\u4fe1",
    //        "isp_id":"100017",
    //        "ip":"183.14.119.173"
    //    }
    //}

    Json::Reader reader;
    Json::Value root;
    if (!reader.parse(sTimezoneString, root, false))
    {
        LOG_ERROR_RLD("Parse Timezone failed beacuse value parsed failed and SrcString is" << sTimezoneString);
        return false;
    }

    if (!root.isObject())
    {
        LOG_ERROR_RLD("Parse Timezone failed beacuse json root parsed failed and SrcString is" << sTimezoneString);
        return false;
    }

    Json::Value retValue = root["code"];
    if (retValue.isNull() || (!retValue.isInt() && !retValue.isUInt()) || retValue.asInt() != 0)
    {
        LOG_ERROR_RLD("Http post return error, SrcString is" << sTimezoneString);
        return false;
    }

    Json::Value data = root["data"];

    Json::Value countrycode = data["country_id"];
    if (countrycode.isNull())
    {
        LOG_ERROR_RLD("Parse Timezone failed beacuse json root[\"country_id\"] is null");
        return false;
    }

    if (countrycode.asString() != "IANA" || data["isp"].asString() != "")
    {
        timezone.sCode = countrycode.asString();
    }
    

    return true;
}

bool CTimeZone::GetTimezoneFromCountryCode( TimeZone &timezone )
{
    if (NULL == m_pDBCache)
    {
        LOG_ERROR_RLD("DBCache is null.");
        return false;
    }

    char sql[1024] = { 0 };
    const char* sqlfmt = "select countrycode, country_en, country_cn, countrySQ from t_timezone_info where countrycode = '%s'";
    snprintf(sql, sizeof(sql), sqlfmt, timezone.sCode.empty() ? "CN" : timezone.sCode.c_str());

    auto SqlFunc = [&](const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn, boost::any &Result)
    {
        switch (uiColumnNum)
        {
        case 0:
            timezone.sCode = strColumn;
            break;
        case 1:
            timezone.sCountryEn = strColumn;
            break;
        case 2:
            timezone.sCountryCn = strColumn;
            break;
        case 3:
            timezone.sCountrySQ = strColumn;
            Result = timezone;
            break;
        default:
            LOG_ERROR_RLD("Unknown sql cb error, uiRowNum:" << uiRowNum << " uiColumnNum:" << uiColumnNum << " strColumn:" << strColumn);
            break;
        }

    };

    std::list<boost::any> ResultList;
    if (!m_pDBCache->QuerySql(std::string(sql), ResultList, SqlFunc))
    {
        LOG_ERROR_RLD("GetCountryInfoByDevID sql failed, sql is " << sql);
        return false;
    }

    if (ResultList.empty())
    {
        LOG_INFO_RLD("GetCountryInfoByDevID info not found, sql is " << sql);
        return false;
    }

    return true;
}