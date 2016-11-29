#include "fdfs_impl.h"

#include <string>
#include "LogRLD.h"
#include "ErrorCode.h"

#include <fastdfs/fdfs_client.h>
#include <fastdfs/fdfs_global.h>
#include <fastdfs/trunk_shared.h>
#include <fastcommon/base64.h>
#include <fastcommon/logger.h>


struct Fdfs {
    ConnectionInfo *tracker_server_ptr;
    ConnectionInfo *storage_server_ptr;
    ConnectionInfo storage_server;
    FDFSFileInfo   file_info;
    bool           is_trunkfile;
    char           group_name[FDFS_GROUP_NAME_MAX_LEN + 1];
    char           remote_filename[256];
    char           file_id[256];
};

///////////////////////////////// download //////////////////////////
Fdfs* FdfsImpl::FdfsClientDownloadOpen(const std::string& fileid, int* errcode)
{
    // check this is init
    if(!errcode) return NULL;
    *errcode = E_SERVER_STORAGEDB_FAILED;
    if(!m_iFlagInit) return NULL;


    // check fileid is correct or not
    if(fileid.find('/') == std::string::npos) return NULL;

    Fdfs *fdfs = (Fdfs*)malloc(sizeof(Fdfs));
    if(!fdfs)
    {
        LOG_ERROR_RLD("Malloc fdfs handle fail. memory is not enought.");
        return NULL;
    }
    memset(fdfs, 0, sizeof(Fdfs));
    memcpy(fdfs->group_name, fileid.c_str(), fileid.find('/'));
    strncpy(fdfs->remote_filename, fileid.c_str()+(int)fileid.find('/')+1, sizeof(fdfs->remote_filename));
    strncpy(fdfs->file_id, fileid.c_str(), sizeof(fdfs->file_id));
    
    // get tracker server
    int result = 0;
    
    if(NULL == (fdfs->tracker_server_ptr = tracker_get_connection()))
    {
        free(fdfs);
        LOG_ERROR_RLD("Get fdfs tracker server fail. ");
        return NULL;
    }

    // get storage server information from tracker server
    result = tracker_query_storage_fetch(fdfs->tracker_server_ptr, &fdfs->storage_server, 
            fdfs->group_name, fdfs->remote_filename);
    if (result != 0)
    {
        LOG_ERROR_RLD("tracker_query_storage_fetch fail, group_name="
                  << fdfs->group_name << ", filename=" << fdfs->remote_filename << ". error no: "
                  << result << ", error info: " << STRERROR(result) << "\n");
        free(fdfs);
        return NULL;
    }
    //std::cout << "storage=" << fdfs->storage_server.ip_addr << ":" << fdfs->storage_server.port << std::endl;

    // get storage server connection from storage server information
    if ((fdfs->storage_server_ptr=tracker_connect_server(&fdfs->storage_server, &result)) == NULL)
    {
        free(fdfs);
        return NULL;
    }
    if(0 != (result = fdfs_get_file_info(fdfs->group_name, fdfs->remote_filename, &fdfs->file_info)))
    {
        LOG_ERROR_RLD("fdfs_get_file_info fail, group_name="
                  << fdfs->group_name << ", filename=" << fdfs->remote_filename << ". error no: "
                  << result << ", error info: " << STRERROR(result) << "\n");
        free(fdfs);
        return NULL;
    }

    // check trunk file
    fdfs->is_trunkfile = fdfs_is_trunk_file(fdfs->remote_filename, strlen(fdfs->remote_filename));

    *errcode = 0;
    return fdfs;
}

int FdfsImpl::FdfsClientDownloadBlock(Fdfs* handle, char* buf, int offset, int size, int* errcode)
{
    // check this is init
    if(!errcode) return 0;
    *errcode = E_SERVER_STORAGEDB_FAILED;
    if(!m_iFlagInit) return 0;
    if(!handle || !buf || offset < 0 || size <= 0) return 0;
    if(!handle->tracker_server_ptr || !handle->storage_server_ptr) return 0;
    if(offset >= handle->file_info.file_size) return 0;

    if(offset + size > handle->file_info.file_size)
    {
        size = handle->file_info.file_size - offset;
    }

    char   *file_buf = NULL;
    int64_t download_bytes = 0;
    int result = storage_do_download_file_ex(handle->tracker_server_ptr, 
            handle->storage_server_ptr, FDFS_DOWNLOAD_TO_BUFF, handle->group_name, 
            handle->remote_filename, offset, size, &file_buf, NULL, &download_bytes);

    if(result != 0)
    {
        LOG_ERROR_RLD("download file fail, error no: " << result << ", error info: "
                  << STRERROR(result));
        return 0;
    }

    if(download_bytes == 0)
    {
        free(file_buf);
        return 0;
    }

    int copy_size = download_bytes < size ? download_bytes : size;
    memcpy(buf, file_buf, copy_size);
    free(file_buf);
    *errcode = 0;
    return copy_size;
}

void FdfsImpl::FdfsClientDownloadClose(Fdfs* handle)
{
    // check this is init
    if(!m_iFlagInit) return ;
    if(!handle) return;
    if(handle->storage_server_ptr)
    {
        tracker_disconnect_server_ex(handle->storage_server_ptr, true);
    }
    if(handle->tracker_server_ptr)
    {
        tracker_disconnect_server_ex(handle->tracker_server_ptr, true);
    }
    free(handle);
}

FdfsImpl::FdfsImpl() : m_iFlagInit(0)
{

}

FdfsImpl::~FdfsImpl()
{

}

bool FdfsImpl::Init(const char* cfgfilename)
{
    // initialize fdfs client
    log_init();
    //g_log_context.log_level = LOG_DEBUG_RLD;

    int result = 0;
    if (0 != (result = fdfs_client_init(cfgfilename)))
    {
        LOG_ERROR_RLD("Init fdfs fail by client config file: " << cfgfilename
            << " error no: " << result << ", error info: " << STRERROR(result));
        return false;
    }

    m_iFlagInit = 1;
    trunk_shared_init();
    return true;
}

void FdfsImpl::UnInit()
{
    m_iFlagInit = 0;
    fdfs_client_destroy();
}

bool FdfsImpl::Upload(const std::string& filename, std::string& fid, std::string& fileurl, const char* origname)
{
    // check this is init
    if (!m_iFlagInit) return false;

    // get tracker server
    int result = 0;
    ConnectionInfo *tracker_server = tracker_get_connection();
    if (!tracker_server)
    {
        LOG_ERROR_RLD("Get fdfs tracker server fail. ");
        return false;
    }

    // get storage server
    int            store_path_index = 0;
    ConnectionInfo storage_server;
    char group_name[FDFS_GROUP_NAME_MAX_LEN + 1] = { 0 };
    if ((result = tracker_query_storage_store(tracker_server,
        &storage_server, group_name, &store_path_index)) != 0)
    {
        LOG_ERROR_RLD("tracker_query_storage fail, error no: " << result
            << ", error info: " << STRERROR(result));
        tracker_disconnect_server_ex(tracker_server, true);
        return false;
    }

    // connect storage server
    ConnectionInfo *storage_server_ptr = NULL;
    if ((storage_server_ptr = tracker_connect_server(&storage_server, &result)) == NULL)
    {
        LOG_ERROR_RLD("tracker_connect_server fail, error no: " << result
            << ", error info: " << STRERROR(result));

        tracker_disconnect_server_ex(tracker_server, true);
        return false;
    }

    // upload file to this storage server
    *group_name = '\0';
    char remote_filename[256] = { 0 };
    const char *extname = origname ? fdfs_get_file_ext_name(origname) : NULL;
    result = storage_upload_by_filename(tracker_server, storage_server_ptr, store_path_index,
        filename.c_str(), extname, NULL, 0, group_name, remote_filename);
    if (result != 0)
    {
        LOG_ERROR_RLD("upload file fail, error no: " << result << ", error info: " << STRERROR(result) << " and filename is " << filename);
        tracker_disconnect_server_ex(storage_server_ptr, true);
        tracker_disconnect_server_ex(tracker_server, true);
        return false;
    }

    // generate file id
    char fileid[256] = { 0 };
    snprintf(fileid, sizeof(fileid), "%s/%s", group_name, remote_filename);
    fid = fileid;
    snprintf(fileid, sizeof(fileid), "http://%s/%s", storage_server_ptr->ip_addr, fid.c_str());
    //fileurl = fileid;

    tracker_disconnect_server_ex(storage_server_ptr, true);
    tracker_disconnect_server_ex(tracker_server, true);
    return true;
}

HANDLE FdfsImpl::DownloadOpen(const char* fileid, int* errcode)
{
    if (!fileid || !errcode) return NULL;
    return FdfsClientDownloadOpen(fileid, errcode);
}

int FdfsImpl::DownloadBlock(HANDLE handle, char* buf, int offset, int size, int* errcode)
{
    return FdfsClientDownloadBlock(reinterpret_cast<Fdfs*>(handle), buf, offset, size, errcode);
}

void FdfsImpl::DownloadClose(HANDLE handle)
{
    FdfsClientDownloadClose(reinterpret_cast<Fdfs*>(handle));
}

int FdfsImpl::DeleteFdfsFileImpl(const char* file_id)
{
    int result = -1;
    ConnectionInfo *pTrackerServer = NULL;
    ConnectionInfo *pStorageServer = NULL;
    ConnectionInfo storageServer;
    memset(&storageServer, 0, sizeof(ConnectionInfo));

    if ((pTrackerServer = tracker_get_connection()) == NULL)
    {
        LOG_ERROR_RLD("tracker_get_connection failed.");
        return errno != 0 ? errno : ECONNREFUSED;
    }

    if ((result = tracker_query_storage_update1(pTrackerServer, &storageServer, file_id)) != 0)
    {
        tracker_disconnect_server_ex(pTrackerServer, true);
        LOG_ERROR_RLD("tracker_query_storage_update1 failed.");
        return result;
    }

    if ((pStorageServer = tracker_connect_server(&storageServer, &result)) == NULL)
    {
        tracker_disconnect_server(pTrackerServer);
        LOG_ERROR_RLD("tracker_connect_server failed.");
        return result;
    }

    result = storage_delete_file1(pTrackerServer, pStorageServer, file_id);

    tracker_disconnect_server(pTrackerServer);
    tracker_disconnect_server(pStorageServer);

    LOG_WARN_RLD("storage_delete_file1 result: " << result);
    return result;
}

int FdfsImpl::GetFileSize(HANDLE handle)
{
    if (!handle) return -1;
    Fdfs *fdfs = reinterpret_cast<Fdfs*>(handle);
    return fdfs->file_info.file_size;
}
