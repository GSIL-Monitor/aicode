#ifndef _RETURN_CODE_
#define _RETURN_CODE_

#include <boost/thread/tss.hpp>
#include "boost/noncopyable.hpp"
#include <string>
class ReturnInfo : public boost::noncopyable
{
public:
    ReturnInfo();
    ~ReturnInfo();

    static const int SUCCESS_CODE;
    static const std::string SUCCESS_INFO;

    static const int FAILED_CODE;
    static const std::string FAILED_INFO;

    static int RetCode();

    static void RetCode(const int iRetCode);

private:
    static boost::thread_specific_ptr<int> ms_iRetCode;

};


#endif
