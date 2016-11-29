#ifndef PROTO_TXT
#define PROTO_TXT

#include <string>
#include <list>
#include "ClientCommInterface.h"

class ProtoTxt
{
public:
    ProtoTxt();
    ~ProtoTxt();

    static char *Pack(const std::string &strSrcID, const std::string &strDstID, const std::string &strType,
        const char *pContentBuffer, const unsigned int uiContentBufferLen, unsigned int &uiTotalLen, const bool IsNeedEncode = false);


    static bool UnPackSingleOne(const char *pOrginalBuffer, const unsigned int uiLenOfOriginal, ClientMsg &Msg, const bool IsNeedValidCrc = false);

    static bool UnPack(std::list<ClientMsg> &TxtMsgList, char *pBuffer, const unsigned int uiSize, std::size_t bytes_transferred, 
        char *&pRemainBufferPos, unsigned int &uiRemainBufferSize);

private:
    static bool ParseProtocol(char *pBuffer, const unsigned int uiSize, std::list<std::string> *strProtoList,
        unsigned int &uiFlag, char *&pBufferNewPos, std::list<std::string> *pDstIDList);


};

#endif
