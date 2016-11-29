#ifndef ERROR_CODE_SERVER
#define ERROR_CODE_SERVER

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 创建日期：2015-1-6
// 作    者：尹宾
// 修改日期：
// 修 改 者：
// 修改说明：
// 类 摘 要：后台错误码汇总，从20000之后开始。
// 详细说明：
// 附加说明：
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#define E_SERVER_NETWORK_FAILED 20001
#define E_SERVER_LOCAL_FILE_FAILED 20100
#define E_SERVER_LOGDB_FAILED 20200
#define E_SERVER_STORAGEDB_FAILED 20300
#define E_SERVER_UNKNOWN_FAILED 20400
#define E_SERVER_EXCEED_CONCURRENT 20500
#define E_SERVER_DOWNLOAD_REPEATED 20600
#define E_SERVER_DELETE_FILE_FAILED 20700
#define E_SERVER_DELETE_FILE_NOT_EXIST 20800
#define E_SERVER_DELETE_FILE_ALREADY_DEL 20900
#define  E_SERVER_BUSINESS_INVALID 20910

#endif