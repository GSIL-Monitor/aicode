#include "CSmtp.h"
#include <iostream>
#include <stdlib.h>
#include "LogRLD.h"

//CSmtp *pmail = NULL;
//CSmtp g_mail;

static void InitLog()
{
    std::string strHost = "MailSender(127.0.0.1)";
    std::string strLogPath = "./mail_logs/";
    std::string strLogFile = "./mail_logs/MailSender.log";
    std::string strLogInnerShowName = "MailSender";
    int iLoglevel = LogRLD::INFO_LOG_LEVEL;
    int iSchedule = LogRLD::DAILY_LOG_SCHEDULE;
    int iMaxLogFileBackupNum = 2000;

    LogRLD::GetInstance().Init(iLoglevel, strHost, strLogInnerShowName, strLogFile, iSchedule, iMaxLogFileBackupNum);

}

static std::string GetStr(char *pValue)
{
    return NULL == pValue ? "NULL" : std::string(pValue);
}

int SendMail(char *pMailServer, int uiSSLFlag, int iPort, char *pLoginName, char *pPwd, char *pSenderName, char *pSenderMail,
  char *pRecipient, char *pMsgTitle, char *pMsgContent, char *pAttach)
{
    LOG_INFO_RLD("Send mail info has received and mailserver is " << GetStr(pMailServer) << " and ssl flag is " << uiSSLFlag <<
        " and port is " << iPort << " and login name is " << GetStr(pLoginName) << " and pwd is " << GetStr(pPwd) <<
        " and sender name is " << GetStr(pSenderName) << " and sender mail is " << GetStr(pSenderMail) <<
        " and recipient is " << GetStr(pRecipient) << " and msg title is " << GetStr(pMsgTitle) << " and msg content is " << GetStr(pMsgContent) <<
        " and attach is " << GetStr(pAttach));

	bool bError = false;

	try
	{

        CSmtp mail;// = //g_mail;//*pmail;
        mail.ClearMessage();
		mail.SetCharSet("UTF-8");

//#define test_gmail_tls

#if defined(test_gmail_tls)
        /*mail.SetSMTPServer("smtp.gmail.com",587);
        mail.SetSecurityType(USE_TLS);*/
#elif defined(test_gmail_ssl)
		mail.SetSMTPServer("smtp.gmail.com",465);
		mail.SetSecurityType(USE_SSL);
#elif defined(test_hotmail_TLS)
		mail.SetSMTPServer("smtp.live.com",25);
		mail.SetSecurityType(USE_TLS);
#elif defined(test_aol_tls)
		mail.SetSMTPServer("smtp.aol.com",587);
		mail.SetSecurityType(USE_TLS);
#elif defined(test_yahoo_ssl)
		mail.SetSMTPServer("plus.smtp.mail.yahoo.com",465);
		mail.SetSecurityType(USE_SSL);
#endif

        mail.SetSMTPServer(pMailServer, (const unsigned short)iPort); //465);//25);//("smtp.163.com",465);//"smtp.qiye.163.com", 465
		if (0 == uiSSLFlag)
		{
			mail.SetSecurityType(USE_SSL);//NO_SECURITY);//USE_SSL);
		}
        else if (1 == uiSSLFlag)
        {
            mail.SetSecurityType(USE_TLS);
        }

		mail.SetLogin(pLoginName);//"fkdulijun2013");
        mail.SetPassword(pPwd);//
        mail.SetSenderName(pSenderName);//"User");
        mail.SetSenderMail(pSenderMail);//cTmp);//"fkdulijun2013@163.com");
        mail.SetReplyTo(pSenderMail);// cTmp);//"fkdulijun2013@163.com");
        mail.SetSubject(pMsgTitle);//"newdata");

		if (NULL != pRecipient)
		{
			std::string strRecipientAll(pRecipient);
			while (true)
			{

				std::string::size_type iPos = std::string::npos;
				if (std::string::npos == (iPos = strRecipientAll.find('|')) && !strRecipientAll.empty())
				{
					mail.AddRecipient(strRecipientAll.c_str());
					break;
				}
				else
				{
					std::string strRecipient = strRecipientAll.substr(0, iPos);
					mail.AddRecipient(strRecipient.c_str());

					strRecipientAll = strRecipientAll.substr(iPos + 1);
				}
			}
			
		}
        
        //mail.AddRecipient(pRecipient);//pInput);//"fkdulijun2013@163.com"); //pInput); ///
  		mail.SetXPriority(XPRIORITY_NORMAL);
  		mail.SetXMailer("The Bat! (v3.02) Professional");
  //		mail.AddMsgLine("");//"Hello,");
		mail.AddMsgLine(pMsgContent);
		//mail.AddMsgLine("...");
		//mail.AddMsgLine("...");//"How are you today?");
		//mail.AddMsgLine("");
		//mail.AddMsgLine("");//"Regards");
		//mail.ModMsgLine(5, ".......");//"regards");
		//mail.DelMsgLine(2);
		//mail.AddMsgLine("User");

		if (NULL != pAttach)
		{
			std::string strPathAll(pAttach);
			while (true)
			{

				std::string::size_type iPos = std::string::npos;
				if (std::string::npos == (iPos = strPathAll.find('|')) && !strPathAll.empty())
				{
					mail.AddAttachment(strPathAll.c_str());
					break;
				}
				else
				{
					std::string strPath = strPathAll.substr(0, iPos);
					mail.AddAttachment(strPath.c_str());

					strPathAll = strPathAll.substr(iPos + 1);
				}
			}
			
		}        


        //mail.AddAttachment(pAttachContext);//".\\Compress.data");
  		//mail.AddAttachment("../test1.jpg");
  		//mail.AddAttachment("c:\\test2.exe");
		//mail.AddAttachment("c:\\test3.txt");
		mail.Send();

	}
	catch(ECSmtp e)
	{
        std::cout << "Error: " << e.GetErrorText().c_str() << ".\n";
        LOG_ERROR_RLD("Send mail failed and error msg is " << e.GetErrorText());

		bError = true;
	}

    if (!bError)
    {
        std::cout << "Mail was send successfully." << std::endl;
        LOG_INFO_RLD("Send mail success");
        exit(0);
    }		
    
    /**
    try
    {
        g_mail.DisconnectRemoteServer();
    }
    catch (...)
    {
    }
     */   

	return 0;
}

int main(int uiCount, char *pInput[])
{
    if (11 > uiCount)
    {
        std::cout << "The argument is not enought, please input needed argument." << std::endl;
        std::cout << "xxx [MailServer] [SSLFlag] [Port] [LoginName] [pwd] [SenderName] [SenderMail] [pRecipient] [MsgTitle] [MsgContent] [AttachPath]" << std::endl;
        return 0;
    }
	
	if (!isdigit(*(pInput[2])))
	{
		std::cout << "SSL flag is not correct, it must be a digit." << std::endl;
		return 1;
	}

    InitLog();
    
    SendMail(pInput[1], atoi(pInput[2]), atoi(pInput[3]), pInput[4], pInput[5], pInput[6], 
	pInput[7], pInput[8], pInput[9], pInput[10], uiCount >11 ? pInput[11] : NULL);

    exit(1);
}