
#include "LogRLD.h"
#include "mysql_helper.h"
#include "mysql_impl.h"
#include <sys/mman.h>


MysqlHelper::MysqlHelper(MysqlImpl* mysqlimpl) : m_MysqlImpl(mysqlimpl), m_pFile(NULL), m_iRecCurrentNum(0), m_iRecProcessed(0),
	m_pTimeHandler(boost::bind(&MysqlHelper::ProcessFileRec, this, _1), CHECKTIME), m_SqlProRunner(1)
{

}

MysqlHelper::~MysqlHelper()
{
	if (NULL!=m_pFile)
	{
		fclose(m_pFile);
	}
	m_SqlProRunner.Stop();
	m_pTimeHandler.Stop();
}

bool MysqlHelper::Init()
{
	m_SqlProRunner.Run();

	m_pTimeHandler.Run();

	m_FilePath = boost::filesystem::current_path() / "sqlredo";

	if (!InitFile())
	{
		LOG_ERROR_RLD("init file[" << m_FilePath.c_str() << "] failed.");
		return false;
	}

	return true;

}

bool MysqlHelper::InitFile()
{
	if (NULL==m_pFile)
	{
		m_pFile = fopen(m_FilePath.c_str(), "rb+");
		if (NULL == m_pFile)
		{
			m_pFile = fopen(m_FilePath.c_str(), "wb+");
			if (NULL == m_pFile)
			{
				LOG_ERROR_RLD("open file[" << m_FilePath.c_str() << "] failed.");
				return false;
			}
		}

		m_iRecCurrentNum = 0;
		m_iRecProcessed = 0;
		
		fseek(m_pFile, 0L, SEEK_END);
		unsigned int iFileSize = ftell(m_pFile);
		
		if (0 < iFileSize)
		{
			m_iRecCurrentNum = iFileSize / sizeof(StSqlRecord);
			
			if (0 != iFileSize%sizeof(StSqlRecord))
			{
				LOG_DEBUG_RLD("truncate file currentnum=" << m_iRecCurrentNum);
				int iFd = fileno(m_pFile);
				if (0>ftruncate(iFd, m_iRecCurrentNum*sizeof(StSqlRecord)))  
			    {  
			        LOG_ERROR_RLD("truncate file[" << m_FilePath.c_str() << "] failed.");
					return false; 
			    }
			}

			fseek(m_pFile, 0L, SEEK_SET);
			StSqlRecord	stSqlRecord;
			for (unsigned int i = 0; i < m_iRecCurrentNum ; ++i)
			{
				bzero(&stSqlRecord, sizeof(stSqlRecord));
				
				int iLen = fread(&stSqlRecord, 1, sizeof(stSqlRecord), m_pFile);
				if (iLen!=sizeof(stSqlRecord))
				{
					fclose(m_pFile);
					m_pFile = NULL;
					LOG_ERROR_RLD("read fail.");
					return false;
				}

				if (1 == stSqlRecord.iProFlag)
				{
					m_iRecProcessed++;
				}
			}

			LOG_DEBUG_RLD("RecCurrentNum=" << m_iRecCurrentNum << ", RecProcessed=" << m_iRecProcessed);
			if (m_iRecCurrentNum<m_iRecProcessed)
			{
				LOG_ERROR_RLD("currentrecnum[" << m_iRecCurrentNum << "] is less than processedrecnum[" << m_iRecProcessed << "].");
				return false;
			}

			if (0!=m_iRecCurrentNum && m_iRecCurrentNum<=m_iRecProcessed) //清空文件
			{
				LOG_DEBUG_RLD("RecCurrentNum=" << m_iRecCurrentNum << ", RecProcessed=" << m_iRecProcessed);
				ClearFile();
			}

		}

	}

	LOG_INFO_RLD("RecCurrentNum=" << m_iRecCurrentNum << ", RecProcessed=" << m_iRecProcessed);

	return true;
}

bool MysqlHelper::ModiFileRec(StSqlRecord &stSqlRecord, int pos)
{
	fseek(m_pFile, pos, SEEK_SET);
	int iSuc = fwrite(&stSqlRecord, sizeof(stSqlRecord), 1, m_pFile);
	if (1!=iSuc)
	{
		LOG_ERROR_RLD("DATA: index[" << stSqlRecord.iIndex << "]proflag[" << stSqlRecord.iProFlag << "]sqlrec[" << stSqlRecord.szSqlRec << "], write fail.");
		return false;
	}
	fflush(m_pFile);
	m_iRecProcessed++;
	return true;
}

void MysqlHelper::ClearFile()
{
	LOG_INFO_RLD("clear file......");
	int iFd = fileno(m_pFile);
	if (0>ftruncate(iFd, 0))
    {  
    	fclose(m_pFile);
		m_pFile = NULL;
        LOG_ERROR_RLD("truncate file[" << m_FilePath.c_str() << "] failed.");
		return ;
    }
	fseek(m_pFile, 0L, SEEK_SET);
	m_iRecCurrentNum = 0;
	m_iRecProcessed = 0;
}


void MysqlHelper::PostSqlRec(const std::string & sSqlRec)
{
	boost::shared_ptr<std::string> ptrSqlRec(new std::string(sSqlRec));
	m_SqlProRunner.Post(boost::bind(&MysqlHelper::WriteRecToFile, this, ptrSqlRec));
}

void MysqlHelper::WriteRecToFile(boost::shared_ptr<std::string> ptrSqlRec)
{
	if (ptrSqlRec->empty())
	{
		LOG_ERROR_RLD("data is null.");
		return;
	}

	boost::unique_lock<boost::mutex> lock(m_mutexFileOpr);

	if (NULL==m_pFile)
	{
		if (!InitFile())
		{
			LOG_ERROR_RLD("init file[" << m_FilePath.c_str() << "] failed.");
			return;
		}
	}

	fseek(m_pFile, 0L, SEEK_END);
	
	StSqlRecord stSqlRecord;
	bzero(&stSqlRecord, sizeof(stSqlRecord));
	stSqlRecord.iIndex = m_iRecCurrentNum + 1;
	stSqlRecord.iProFlag = 0;
	strcpy(stSqlRecord.szSqlRec, ptrSqlRec->c_str());
	LOG_INFO_RLD("index[" << stSqlRecord.iIndex << "]proflag[" << stSqlRecord.iProFlag << "]sqlrec[" << stSqlRecord.szSqlRec << "]");
	
	int iSuc = fwrite(&stSqlRecord, sizeof(stSqlRecord), 1, m_pFile);
	if (1!=iSuc)
	{
		fclose(m_pFile);
		m_pFile = NULL;
		LOG_ERROR_RLD("DATA: index[" << stSqlRecord.iIndex << "]proflag[" << stSqlRecord.iProFlag << "]sqlrec[" << stSqlRecord.szSqlRec << "], write fail.");
		return;
	}
	fflush(m_pFile);
	m_iRecCurrentNum++;
	
}

void MysqlHelper::ProcessFileRec(const boost::system::error_code& e)
{
	boost::unique_lock<boost::mutex> lock(m_mutexFileOpr);

	if (NULL==m_pFile)
	{
		if (!InitFile())
		{
			LOG_ERROR_RLD("init file[" << m_FilePath.c_str() << "] failed.");
			return;
		}
	}

	fseek(m_pFile, 0L, SEEK_SET);

	StSqlRecord	stSqlRecord;
	std::string sSqlRec;
	int iPos = 0;
	LOG_DEBUG_RLD("currentnum[" << m_iRecCurrentNum << "]processednum[" << m_iRecProcessed << "]");
	for (unsigned int i = 0; i < m_iRecCurrentNum ; ++i)
	{
		bzero(&stSqlRecord, sizeof(stSqlRecord));
		
		int iLen = fread(&stSqlRecord, 1, sizeof(stSqlRecord), m_pFile);
		if (iLen!=sizeof(stSqlRecord))
		{
			fclose(m_pFile);
			m_pFile = NULL;
			LOG_ERROR_RLD("read fail.");
			return;
		}
		
		if (0 == stSqlRecord.iProFlag)
		{
			bool	bResult = false;
			sSqlRec = stSqlRecord.szSqlRec;
			LOG_INFO_RLD("REDO sql[" << sSqlRec << "]");
			bResult = m_MysqlImpl->SqlExec(sSqlRec);
			if( bResult )
			{
				stSqlRecord.iProFlag = 1;
				iPos = i*sizeof(stSqlRecord);
				if (!ModiFileRec(stSqlRecord, iPos))
				{
					fclose(m_pFile);
					m_pFile = NULL;
					LOG_ERROR_RLD("DATA: index[" << stSqlRecord.iIndex << "]proflag[" << stSqlRecord.iProFlag << "]sqlrec[" << stSqlRecord.szSqlRec << "], modi fail.");
					return;
				}
			}
			else
			{
				unsigned int iErrNo = m_MysqlImpl->GetMysqlErrNo();
				LOG_ERROR_RLD("exec sql[" << sSqlRec << "] failed, sqlerrno=" << iErrNo);
				if (0!=iErrNo && ERR_MYSQL_CONN!=iErrNo)
				{
					stSqlRecord.iProFlag = 1;
					iPos = i*sizeof(stSqlRecord);
					if (!ModiFileRec(stSqlRecord, iPos))
					{
						fclose(m_pFile);
						m_pFile = NULL;
						LOG_ERROR_RLD("DATA: index[" << stSqlRecord.iIndex << "]proflag[" << stSqlRecord.iProFlag << "]sqlrec[" << stSqlRecord.szSqlRec << "], modi fail.");
						return;
					}
				}
				else
				{
					return;
				}
			}

		}
	}

	LOG_DEBUG_RLD("process file suc.currentnum[" << m_iRecCurrentNum << "]processednum[" << m_iRecProcessed << "]");

	if (0!=m_iRecCurrentNum && m_iRecCurrentNum<=m_iRecProcessed) //清空文件
	{
		ClearFile();
	}

	
}

