
#ifndef __FDFS_IMPL_H__
#define __FDFS_IMPL_H__

#include <boost/thread/mutex.hpp>
#include <boost/thread/detail/singleton.hpp>
#include <boost/thread/condition.hpp>
#include <boost/scope_exit.hpp>

typedef void* HANDLE;
struct Fdfs;

class FdfsImpl : public boost::noncopyable
{
public:
    FdfsImpl();
    ~FdfsImpl();    
    
    bool Init(const char* cfgfilename);
    void UnInit();

    bool Upload(const std::string& filename, std::string& fid, std::string& fileurl, const char* origname);

    HANDLE DownloadOpen(const char* fileid, int* errcode);

    int DownloadBlock(HANDLE handle, char* buf, int offset, int size, int* errcode);

    void DownloadClose(HANDLE handle);

    int DeleteFdfsFileImpl(const char* file_id);

    int GetFileSize(HANDLE handle);

private:

    Fdfs* FdfsClientDownloadOpen(const std::string& fileid, int* errcode);
    int FdfsClientDownloadBlock(Fdfs* handle, char* buf, int offset, int size, int* errcode);
    void FdfsClientDownloadClose(Fdfs* handle);

private:
    int m_iFlagInit;
};

#endif //__FDFS_IMPL_H__
