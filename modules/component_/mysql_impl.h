
#ifndef __MYSQL_IMPL_H__
#define __MYSQL_IMPL_H__

#include <string>
#include <vector>
#include "mysql_helper.h"
#include <boost/lexical_cast.hpp>
#include <boost/thread/mutex.hpp>
#include <my_global.h>
#include <mysql.h>

#define ERR_MYSQL_CONN 2003

struct FileInfoList
{
    std::string id;
    std::string fileid;
    long filesize;
    std::string createdate;
};

class MysqlHelper;
class MysqlImpl
{
public:
    MysqlImpl();
    ~MysqlImpl();

    typedef boost::function<void(const boost::uint32_t uiRowNum, const boost::uint32_t uiColumnNum, const std::string &strColumn)> ColumnCB;

    bool Init(const char* dbhost, const char* dbuser, const char* dbpass, const char* dbname, int myhelpervalid = 0);
    void UnInit();

    bool QueryExec(const std::string& sql);

    bool QueryExec(const std::string &sql, ColumnCB ccb);

    template <typename T>
    bool QueryExec(const std::string& sql, std::vector<T>& result);

	bool SqlExec(const std::string& sql);
	unsigned int GetMysqlErrNo()
	{
		if (m_myconn)
		{
			return mysql_errno(m_myconn);
		}
		else
		{
			return 0;
		}
	}

	bool SetMyHelperValid(bool myhelpervalid);

private:
    bool   ConnectMysql(bool reinit = false);
    MYSQL* GetMysqlConn();

private:
    std::string m_dbhost;
    std::string m_dbuser;
    std::string m_dbpass;
    std::string m_dbname;

    MYSQL* m_myconn;
    boost::mutex m_mutex; //for m_myconn

	bool m_bValidFlag;  //myhelper生效标志,默认不生效
	MysqlHelper* m_myhelper;

};

#endif //__MYSQL_IMPL_H__
