#include "CacheBlock.h"
#include "ErrorCode.h"
#include "LogRLD.h"

CacheBlock::CacheBlock(const std::string &strKey, char *pBlkBuffer, const boost::uint32_t uiLen,
    boost::pool<> *pBlkPL) :
    m_strKey(strKey), m_pBlkBuffer(pBlkBuffer), m_uiLen(uiLen), m_pBlkPL(pBlkPL)
{

}


CacheBlock::~CacheBlock()
{
    LOG_INFO_RLD("CacheBlock::~CacheBlock, key is : " << m_strKey << " buffer len: " << m_uiLen);

    if (NULL != m_pBlkPL && NULL != m_pBlkBuffer)
    {
        m_pBlkPL->free(m_pBlkBuffer);

        LOG_INFO_RLD("CacheBlock::~CacheBlock m_pBlkBuffer be freed by pool , key is : " << m_strKey);
    }
    else
    {
        delete[] m_pBlkBuffer;
        m_pBlkBuffer = NULL;

        LOG_INFO_RLD("CacheBlock::~CacheBlock m_pBlkBuffer be freed by system , key is : " << m_strKey);
    }
}


char *CacheBlock::GetContent()
{
    return m_pBlkBuffer;
}


const std::string & CacheBlock::GetKey()
{
    return m_strKey;
}

boost::uint32_t CacheBlock::GetLen()
{
    return m_uiLen;
}


