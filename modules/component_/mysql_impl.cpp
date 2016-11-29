
#include "LogRLD.h"
#include "mysql_impl.h"
//#include "st_query_msg.h"

static char NULLSTR[2] = {0};

MysqlImpl::MysqlImpl() : m_myconn(NULL), m_bValidFlag(false), m_myhelper(NULL)
{

}

MysqlImpl::~MysqlImpl()
{
    UnInit();

	if (m_myhelper!=NULL)
	{
		delete m_myhelper;
		m_myhelper = NULL;
	}
}

bool MysqlImpl::Init(const char* dbhost, const char* dbuser, const char* dbpass, const char* dbname, int myhelpervalid)
{
    if (dbhost) m_dbhost = dbhost;
    if (dbuser) m_dbuser = dbuser;
    if (dbpass) m_dbpass = dbpass;
    if (dbname) m_dbname = dbname;

    if(NULL == (m_myconn = mysql_init(NULL)))
    {
        LOG_ERROR_RLD("Init mysql client fail");
        return false;
    }

    if (!ConnectMysql())
    {
        LOG_ERROR_RLD("ConnectMysql faild.");
        return false;
    }

    LOG_INFO_RLD("MysqlImpl Init OK.");

	if (myhelpervalid)
	{
		m_myhelper = new MysqlHelper(this);
		if (!m_myhelper->Init())
		{
			delete m_myhelper;
			m_myhelper = NULL;
			LOG_ERROR_RLD("MysqlHelper Init faild.");
		}
		else
		{
			LOG_INFO_RLD("MysqlHelper Init OK.");
			m_bValidFlag = true;
		}
	}
	
    return true;
}

void MysqlImpl::UnInit()
{
    if (m_myconn!=NULL)
    {
        mysql_close(m_myconn);
        m_myconn = NULL;
    }
}

bool MysqlImpl::SetMyHelperValid(bool myhelpervalid)
{
	boost::unique_lock<boost::mutex> lock(m_mutex);
	if (myhelpervalid)
	{
		if (NULL == m_myhelper)
		{
			m_myhelper = new MysqlHelper(this);
			if (!m_myhelper->Init())
			{
				delete m_myhelper;
				m_myhelper = NULL;
				m_bValidFlag = false;
				LOG_ERROR_RLD("Init MysqlHelper faild.");
				return false;
			}
			else
			{
				m_bValidFlag = true;
			}
		}
		else
		{
			m_bValidFlag = true;
		}
	}
	else
	{

	}
	
	return true;
}

bool MysqlImpl::ConnectMysql(bool reinit)
{
    if (reinit)
    {
        UnInit();

        if(NULL == (m_myconn = mysql_init(NULL)))
        {
            LOG_ERROR_RLD("Init mysql client fail");
            return false;
        }
    }

    LOG_INFO_RLD("try connect to db: " << m_dbhost);
    MYSQL* pConn = mysql_real_connect(m_myconn, m_dbhost.c_str(), m_dbuser.c_str(), m_dbpass.c_str(), m_dbname.c_str(), 0, NULL, 0);
    if (NULL == pConn)
    {   
        LOG_ERROR_RLD("ERROR: Connect mysql[" << m_dbuser.c_str() << "@" << m_dbhost.c_str() << "]. " << mysql_errno(m_myconn) << ":" << mysql_error(m_myconn));
        return false;
    }

    my_bool reconnect = true;
    mysql_options(m_myconn, MYSQL_OPT_RECONNECT, &reconnect);
    LOG_INFO_RLD("connect to db OK.");
    return true;
}

MYSQL* MysqlImpl::GetMysqlConn()
{
    if(mysql_ping(m_myconn) !=0 && !ConnectMysql(true))
    {
        LOG_ERROR_RLD("GetMysqlConn failed.");
        return NULL;
    }

    return m_myconn;
}

bool MysqlImpl::SqlExec(const std::string& sql)
{
    boost::unique_lock<boost::mutex> lock(m_mutex);

    LOG_INFO_RLD("QueryExec SQL: " << sql);

    MYSQL *conn = GetMysqlConn();
    if (!conn)
    {
        LOG_ERROR_RLD("Can not get mysql connection.");
        return false;
    }

    if (mysql_query(conn, sql.c_str()) != 0)
    {
        LOG_ERROR_RLD("ERROR: Query mysql[" << m_dbuser << "@" << m_dbhost << "]. " << mysql_errno(conn) << ":" << mysql_error(conn));
        return false;
    }

    LOG_INFO_RLD("QueryExec Success, SQL: " << sql);
    return true;
}

bool MysqlImpl::QueryExec(const std::string& sql)
{
    boost::unique_lock<boost::mutex> lock(m_mutex);

    LOG_INFO_RLD("QueryExec SQL: " << sql);

    MYSQL *conn = GetMysqlConn();
    if (!conn)
    {
        LOG_ERROR_RLD("Can not get mysql connection.");
		if (m_myhelper)
		{
			m_myhelper->PostSqlRec(sql);
		}
		
        return false;
    }

    if (mysql_query(conn, sql.c_str()) != 0)
    {
        LOG_ERROR_RLD("ERROR: Query mysql[" << m_dbuser << "@" << m_dbhost << "]. " << mysql_errno(conn) << ":" << mysql_error(conn));
		if (m_myhelper && ERR_MYSQL_CONN==mysql_errno(conn))
		{
			m_myhelper->PostSqlRec(sql);
		}
		
        return false;
    }

    LOG_INFO_RLD("QueryExec Success, SQL: " << sql);
    return true;
}

template <typename T>
void assign(T& left, const MYSQL_ROW& row)
{
	for( int i = 0; i < 1; ++i )
	{
		if( NULL == row[i] )
		{
			row[i] = NULLSTR;
		}
	}
    left = atoi(row[0]);
}

template <>
void assign<FileInfoList>(FileInfoList& left, const MYSQL_ROW& row)
{
	for( int i = 0; i < 4; ++i )
	{
		if( NULL == row[i] )
		{
			row[i] = NULLSTR;
		}
	}
    if(row[0]) left.id         = row[0];       // uuid
    if(row[1]) left.fileid     = row[1];       // fileid
    if(row[2]) left.filesize   = atoi(row[2]); // filesize
    if(row[3]) left.createdate = row[3];       // createdate
}

//template <>
//void assign<StResultItem>(StResultItem& left, const MYSQL_ROW& row)
//{
//	for( int i = 0; i < 6; ++i )
//	{
//		if( NULL == row[i] )
//		{
//			row[i] = NULLSTR;
//		}
//	}
//    if(row[0]) left.SetFileId(row[0]);         // fileid
//    if(row[1]) left.SetFileName(row[1]);       // filename
//    if(row[2]) left.SetFileSize(atoi(row[2])); // filesize
//    if(row[3]) left.SetFileMd5(row[3]);        // filemd5
//    if(row[4]) left.SetFileExtend(row[4]);     // extend
//    if(row[5]) left.SetCreateTime(row[5]);     // extend
//}

template <typename T>
bool MysqlImpl::QueryExec(const std::string& sql, std::vector<T>& result)
{
    boost::unique_lock<boost::mutex> lock(m_mutex);

    LOG_INFO_RLD("QueryExec SQL: " << sql);

    MYSQL *conn = GetMysqlConn();
    if (!conn)
    {
        LOG_ERROR_RLD("Can not get mysql connection.");
        return false;
    }

    if (mysql_query(conn, sql.c_str()) != 0)
    {
        LOG_ERROR_RLD("ERROR: Query mysql[" << m_dbuser << "@" << m_dbhost << "]. " << mysql_errno(conn) << ":" << mysql_error(conn));
        return false;
    }

    MYSQL_RES *res = mysql_store_result(conn);
    if (!res)
    {
        LOG_ERROR_RLD("ERROR: Query mysql[" << m_dbuser << "@" << m_dbhost << "]. " << mysql_errno(conn) << ":" << mysql_error(conn));
        return false;
    }

    MYSQL_ROW row;
    while ((row=mysql_fetch_row(res))!=NULL)
    {
        T tmpObj;
        assign(tmpObj, row);
        result.push_back(tmpObj);
    }

    mysql_free_result(res);
    LOG_INFO_RLD("QueryExec Success, SQL: " << sql);
    return true;
}

template bool MysqlImpl::QueryExec(const std::string& sql, std::vector<int>& result);
template bool MysqlImpl::QueryExec(const std::string& sql, std::vector<unsigned int>& result);
//template bool MysqlImpl::QueryExec(const std::string& sql, std::vector<StResultItem>& result);
template bool MysqlImpl::QueryExec(const std::string& sql, std::vector<FileInfoList>& result);

bool MysqlImpl::QueryExec(const std::string &sql, ColumnCB ccb)
{
    boost::unique_lock<boost::mutex> lock(m_mutex);

    LOG_INFO_RLD("QueryExec SQL: " << sql);

    MYSQL *conn = GetMysqlConn();
    if (!conn)
    {
        LOG_ERROR_RLD("Can not get mysql connection.");
        return false;
    }

    if (mysql_query(conn, sql.c_str()) != 0)
    {
        LOG_ERROR_RLD("ERROR: Query mysql[" << m_dbuser << "@" << m_dbhost << "]. " << mysql_errno(conn) << ":" << mysql_error(conn));
        return false;
    }

    MYSQL_RES *res = mysql_store_result(conn);
    if (!res)
    {
        LOG_ERROR_RLD("ERROR: Query mysql[" << m_dbuser << "@" << m_dbhost << "]. " << mysql_errno(conn) << ":" << mysql_error(conn));
        return false;
    }

    boost::uint32_t k = 0;
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res)) != NULL)
    {
        int iCn = mysql_num_fields(res);
        for (int i = 0; i < iCn; ++i)
        {
            std::string strColumn = row[i];
            ccb(k, i, strColumn);
        }

        ++k;
    }

    mysql_free_result(res);
    LOG_INFO_RLD("QueryExec Success, SQL: " << sql);
    return true;
}
