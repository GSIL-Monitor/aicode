#include "CSmtp.h"
#include <iostream>
#include <stdlib.h>

//CSmtp *pmail = NULL;
CSmtp g_mail;

int SendMail(char *pMailServer, int uiSSLFlag, char *pLoginName, char *pPwd, char *pSenderName, char *pSenderMail,
  char *pRecipient, char *pMsgTitle, char *pMsgContent, char *pAttach)
{
	bool bError = false;

	try
	{

		CSmtp &mail = g_mail;//*pmail;
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

        mail.SetSMTPServer(pMailServer, 465);//25);//("smtp.163.com",465);//"smtp.qiye.163.com", 465
		if (uiSSLFlag)
		{
			mail.SetSecurityType(USE_SSL);//NO_SECURITY);//USE_SSL);
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
		bError = true;
	}

    if (!bError)
    {
        std::cout << "Mail was send successfully." << std::endl;
    }		
        
    try
    {
        g_mail.DisconnectRemoteServer();
    }
    catch (...)
    {
    }
        

	return 0;
}

int main(int uiCount, char *pInput[])
{
    if (10 > uiCount)
    {
        std::cout << "The argument is not enought, please input needed argument." << std::endl;
        std::cout << "xxx [MailServer] [SSLFlag] [LoginName] [pwd] [SenderName] [SenderMail] [pRecipient] [MsgTitle] [MsgContent] [AttachPath]" << std::endl;
        return 0;
    }
	
	if (!isdigit(*(pInput[2])))
	{
		std::cout << "SSL flag is not correct, it must be a digit." << std::endl;
		return 1;
	}
    
    SendMail(pInput[1], atoi(pInput[2]), pInput[3], pInput[4], pInput[5], 
	pInput[6], pInput[7], pInput[8], pInput[9], uiCount >10 ? pInput[10] : NULL);

    return 1;
}