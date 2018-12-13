#ifndef _MESSAGE_PUSH_GETUI_
#define _MESSAGE_PUSH_GETUI_

#include <string>
#include <boost/thread/mutex.hpp>
#include "NetComm.h"

using std::string;

class MessagePush_Getui
{
public:
    MessagePush_Getui(const string &strPushUrl, const string &strAppID, const string &strAppKey,
        const string &strMasterSecret, const int iAuthExpire);
    ~MessagePush_Getui();

    static const int PLATFORM_ANDROID = 0;
    static const int PLATFORM_ANDROID_HD = 1;
    static const int PLATFORM_IOS = 2;
    static const int PLATFORM_IOS_HD = 3;
    static const int PLATFORM_WEB = 9;

    struct Style
    {
        int iType;
        string strTitle;
        string strContent;
    };

    struct Notification
    {
        bool bTransmissionType;
        string strTransmissionContent;
        string strDurationBegin;
        string strDurationEnd;
        Style style;
    };

    struct Aps
    {
        string strTitle;
        string strBody;
        string strBadge;
        int iContentAvailable;
    };

    struct MultiMedia
    {
        string strUrl;
        int iType;
        bool bOnlyWifi;
    };

    struct PushInfo
    {
        Aps aps;
        MultiMedia multiMedia;
    };

    struct Message
    {
        string strAppKey;
        bool bIsOffline;
        int iOfflineExpireTime;
        int iPushNetworkType;
        string strMessageType;
        string strClientID;
        string strAlias;
        string strRequestID;
        Notification notification;
        PushInfo pushInfo;
    };

    struct PushMessage
    {
        string strTitle;
        string strNotyContent;
        string strPayloadContent;
        bool bIsOffline;
        int iOfflineExpireTime;
        string strClientID;
        string strAlias;
    };

    void Init();

    bool FillMessage(const PushMessage &source, Message &message, const int iClientPlatform);
    bool FormatMessage(const Message &source, string &strMessage, const int iClientPlatform);
    void PushSingle(const PushMessage &message, const int iClientPlatform);

private:
    void GetAuthToken();
    string CurrentTimestamp();
    string SHA256String(const string &strSrc);

    string m_strPushUrl;
    string m_strAppID;
    string m_strAppSecret;
    string m_strAppKey;
    string m_strMasterSecret;
    string m_strAuthToken;
    long int m_lAuthExpire;

    TimeOutHandler m_authTimer;

    boost::atomic_uint32_t m_reqSequence;
};

#endif
