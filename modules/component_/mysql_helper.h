
#ifndef __RW_MYSQL_HELPER_H__
#define __RW_MYSQL_HELPER_H__

#include <string>
#include <vector>
#include <boost/thread/mutex.hpp>
#include <boost/filesystem.hpp>
#include <boost/date_time/posix_time/posix_time.hpp> 


#include "NetComm.h"

typedef struct SqlRecord
{
	int			iIndex;
	int			iProFlag;
	char		szSqlRec[1024];
} StSqlRecord;

class MysqlImpl;
class MysqlHelper 
{
public:
    MysqlHelper(MysqlImpl* mysqlimpl);
    ~MysqlHelper();

	void PostSqlRec(const std::string & sSqlRec);
	void WriteRecToFile(boost::shared_ptr<std::string> ptrSqlRec);
	void ProcessFileRec(const boost::system::error_code& e);
	bool Init();

private:
	bool InitFile();
	bool ModiFileRec(StSqlRecord &stSqlRecord, int pos);
	void ClearFile();

private:
	MysqlImpl	*m_MysqlImpl;
	FILE		*m_pFile;

	boost::atomic_uint32_t			m_iRecCurrentNum;
	boost::atomic_uint32_t			m_iRecProcessed;
	static const int				CHECKTIME     = 1800; //定时器1800s检查一次CHECKTIME
	boost::filesystem::path			m_FilePath;
	
    TimeOutHandler m_pTimeHandler;

	boost::mutex m_mutexFileOpr;
	
	Runner m_SqlProRunner;
	
};

#endif //__RW_MYSQL_HELPER_H__
