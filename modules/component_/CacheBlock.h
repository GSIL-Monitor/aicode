#ifndef CACHE_BLOCK
#define CACHE_BLOCK

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 创建日期：2015-1-8
// 作    者：尹宾
// 修改日期：
// 修 改 者：
// 修改说明：
// 类 摘 要：缓存文件
// 详细说明：
// 附加说明：
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
