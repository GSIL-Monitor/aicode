#include "ReturnCode.h"

const int ReturnInfo::SUCCESS_CODE = 0;

const std::string ReturnInfo::SUCCESS_INFO = "ok";

const int ReturnInfo::FAILED_CODE = 1;

const std::string ReturnInfo::FAILED_INFO = "failed";

boost::thread_specific_ptr<int> ReturnInfo::ms_iRetCode;

int ReturnInfo::RetCode()
{
    if (NULL == ms_iRetCode.get())
    {
        ms_iRetCode.reset(new int(SUCCESS_CODE));
    }

    return *ms_iRetCode;
}

void ReturnInfo::RetCode(const int iRetCode)
{
    if (NULL == ms_iRetCode.get())
    {
        ms_iRetCode.reset(new int(SUCCESS_CODE));
    }

    (*ms_iRetCode) = iRetCode;
}




