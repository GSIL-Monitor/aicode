#ifndef _RETURN_CODE_
#define _RETURN_CODE_

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

};


#endif
