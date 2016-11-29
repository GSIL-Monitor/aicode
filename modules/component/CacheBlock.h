#ifndef CACHE_BLOCK
#define CACHE_BLOCK

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// �������ڣ�2015-1-8
// ��    �ߣ�����
// �޸����ڣ�
// �� �� �ߣ�
// �޸�˵����
// �� ժ Ҫ�������ļ�
// ��ϸ˵����
// ����˵����
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


#include "CacheManager.h"
#include <boost/pool/pool.hpp>

class CacheBlock : public CacheObj
{
public:
    CacheBlock(const std::string &strKey, char *pBlkBuffer, const boost::uint32_t uiLen,
        boost::pool<> *pBlkPL = NULL);
    virtual ~CacheBlock();

    virtual const std::string &GetKey();

    char *GetContent();

    boost::uint32_t GetLen();

private:
    std::string m_strKey;
    char *m_pBlkBuffer;
    const boost::uint32_t m_uiLen;
    boost::pool<> *m_pBlkPL;

};


#endif
