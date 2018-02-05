#include "MessagePush_Getui.h"

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/lexical_cast.hpp>
#include "CommonUtility.h"
#include "HttpClient.h"
#include "LogRLD.h"
#include "json/json.h"
#include "openssl/sha.h"

MessagePush_Getui::MessagePush_Getui(const string &strPushUrl, const string &strAppID, const string &strAppKey, const string &strMasterSecret, const int iAuthExpire) :
    m_strPushUrl(strPushUrl),
    m_strAppID(strAppID),
    m_strAppKey(strAppKey),
    m_strMasterSecret(strMasterSecret),
    m_lAuthExpire(iAuthExpire),
    m_authTimer(NULL, m_lAuthExpire),
    m_reqSequence(0)
{
}

MessagePush_Getui::~MessagePush_Getui()
{
}

void MessagePush_Getui::Init()
{
    LOG_ERROR_RLD("---debug, push url: " << m_strPushUrl
        << ", app id: " << m_strAppID
        << ", app key: " << m_strAppKey
        << ", master secret: " << m_strMasterSecret
        << ", expire time: " << m_lAuthExpire);

    m_authTimer.SetTimeOutCallBack(boost::bind(&MessagePush_Getui::GetAuthToken, this));
    m_authTimer.Run(true);
}

string MessagePush_Getui::CurrentTimestamp()
{
    boost::posix_time::ptime epoch(boost::gregorian::date(1970, 1, 1));
    auto dur = boost::posix_time::microsec_clock::universal_time() - epoch;
    string timestamp = boost::lexical_cast<string>(dur.total_microseconds());

    return timestamp;
}

string MessagePush_Getui::SHA256String(const string &strSrc)
{
    // 调用sha256哈希    
    unsigned char mdStr[33] = { 0 };
    SHA256((const unsigned char *)strSrc.c_str(), strSrc.length(), mdStr);

    // 哈希后的字符串    
    string strEnc = string((const char *)mdStr);
    // 哈希后的十六进制串 32字节    
    char buf[65] = { 0 };
    char tmp[3] = { 0 };
    for (int i = 0; i < 32; i++)
    {
        sprintf(tmp, "%02x", mdStr[i]);
        strcat(buf, tmp);
    }
    //buf[32] = '\0'; // 后面都是0，从32字节截断    
    string strEncHex = string(buf);
    LOG_ERROR_RLD("---debug, sha256: " << strEncHex);

    return strEncHex;
}

void MessagePush_Getui::GetAuthToken()
{
    Json::Value root;
    boost::posix_time::ptime epoch(boost::gregorian::date(1970, 1, 1));
    auto dur = boost::posix_time::microsec_clock::universal_time() - epoch;
    string timestamp = boost::lexical_cast<string>(dur.total_milliseconds());
    root["sign"] = SHA256String(m_strAppKey + timestamp + m_strMasterSecret);
    LOG_ERROR_RLD("---debug, src token: " << m_strAppKey + timestamp + m_strMasterSecret
        << ", milli second: " << timestamp);
    root["timestamp"] = timestamp;
    root["appkey"] = m_strAppKey;

    Json::FastWriter writer;

    HttpClient http;
    string strResponse;
    string strUrl = m_strPushUrl + m_strAppID + "/auth_sign";
    LOG_ERROR_RLD("---debug, url: " << strUrl
        << ", request: " << writer.write(root));
    if (http.HttpsPostJson(strUrl, string(), writer.write(root), strResponse) != CURLE_OK)
    {
        LOG_ERROR_RLD("GetAuthToken failed, http post error, url is " << strUrl);
        return;
    }

    LOG_ERROR_RLD("---debug, response: " << strResponse);

    Json::Value rsp;
    Json::Reader reader;
    if (!reader.parse(strResponse, rsp))
    {
        LOG_ERROR_RLD("GetAuthToken failed, parse response error, raw data is " << strResponse);
        return;
    }

    if (rsp["expire_time"].isNull() || !rsp["expire_time"].isString())
    {
        LOG_ERROR_RLD("GetAuthToken failed, response expire time format error, raw data is " << strResponse);
        return;
    }

    if (rsp["auth_token"].isNull() || !rsp["auth_token"].isString())
    {
        LOG_ERROR_RLD("GetAuthToken failed, response auth token format error, raw data is " << strResponse);
        return;
    }

    boost::unique_lock<boost::mutex> lock(m_authMutex);

    m_lAuthExpire = boost::lexical_cast<long>(rsp["expire_time"].asString());

    m_strAuthToken = rsp["auth_token"].asString();

    LOG_ERROR_RLD("---debug, auth expire: " << m_lAuthExpire
        << ", auth token: " << m_strAuthToken);
}

bool MessagePush_Getui::FillMessage(const PushMessage &source, Message &message, const int iClientPlatform)
{
    message.strAppKey = m_strAppKey;
    message.bIsOffline = source.bIsOffline;
    message.iOfflineExpireTime = source.iOfflineExpireTime;
    message.iPushNetworkType = 0;
    message.strMessageType = "notification";
    message.strClientID = source.strClientID;
    message.strAlias = source.strAlias;
    message.strRequestID = boost::lexical_cast<string>(++m_reqSequence) + CurrentTimestamp();

    message.notification.bTransmissionType = false;
    message.notification.strTransmissionContent = source.strPayloadContent;

    boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
    string begin = boost::posix_time::to_iso_extended_string(now);
    string end = boost::posix_time::to_iso_extended_string(now + boost::posix_time::hours(1));
    message.notification.strDurationBegin = begin.replace(begin.find_first_of('T'), 1, string(" "));
    message.notification.strDurationEnd = end.replace(end.find_first_of('T'), 1, string(" "));

    message.notification.style.iType = 0;
    message.notification.style.strTitle = source.strTitle;
    message.notification.style.strContent = source.strNotyContent;

    if (iClientPlatform == PLATFORM_IOS)
    {
        message.pushInfo.aps.strTitle = source.strTitle;
        message.pushInfo.aps.strBody = source.strNotyContent;
        message.pushInfo.aps.strBadge = "0";
        message.pushInfo.aps.iContentAvailable = 1;
    }

    return true;
}

bool MessagePush_Getui::FormatMessage(const Message &source, string &strMessage, const int iClientPlatform)
{
    Json::Value root;
    Json::Value message;
    Json::Value notification;
    Json::Value style;

    style["type"] = 0;
    style["title"] = source.notification.style.strTitle;
    style["text"] = source.notification.style.strContent;

    notification["style"] = style;
    notification["transmission_type"] = source.notification.bTransmissionType;
    notification["transmission_content"] = source.notification.strTransmissionContent;
    notification["duration_begin"] = source.notification.strDurationBegin;
    notification["duration_end"] = source.notification.strDurationEnd;

    message["appkey"] = m_strAppKey;
    message["msgtype"] = source.strMessageType;
    message["is_offline"] = source.bIsOffline;
    if (source.bIsOffline) message["offline_expire_time"] = source.iOfflineExpireTime;

    root["message"] = message;
    root["notification"] = notification;
    if (!source.strClientID.empty()) root["cid"] = source.strClientID;
    if (!source.strAlias.empty()) root["alias"] = source.strAlias;
    root["requestid"] = source.strRequestID;

    if (iClientPlatform == PLATFORM_IOS)
    {
        Json::Value push;
        Json::Value aps;
        Json::Value alert;

        alert["title"] = source.pushInfo.aps.strTitle;
        alert["body"] = source.pushInfo.aps.strBody;
        aps["alert"] = alert;
        aps["autoBadge"] = source.pushInfo.aps.strBadge;
        aps["content-available"] = source.pushInfo.aps.iContentAvailable;
        push["aps"] = aps;

        root["push_info"] = push;
    }

    strMessage = Json::FastWriter().write(root);

    return true;
}

void MessagePush_Getui::PushSingle(const PushMessage &message, const int iClientPlatform)
{
    HttpClient http;

    string strUrl = m_strPushUrl + m_strAppID + "/push_single";
    string strRequest;

    Message push;
    FillMessage(message, push, iClientPlatform);
    FormatMessage(push, strRequest, iClientPlatform);
    string strAuthToken = "authtoken:" + m_strAuthToken;
    string strResponse;

    LOG_ERROR_RLD("---debug, request: " << strRequest
        << ", auth token: " << strAuthToken
        << ", url: " << strUrl);
    if (http.HttpsPostJson(strUrl, strAuthToken, strRequest, strResponse) != CURLE_OK)
    {
        LOG_ERROR_RLD("PushSingle failed, http post error, url is " << strUrl);
    }
    LOG_ERROR_RLD("---debug, response: " << strResponse);
}

