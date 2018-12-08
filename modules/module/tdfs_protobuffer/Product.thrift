//产品服务相关定义内容
//
//

namespace cpp Product.Service

enum TReturnCode
{
    Success = 0,
    Failed = 1
}

struct TReqInfo
{
    1: optional binary req;
}

struct TRspInfo
{
    1: optional binary rsp;
}

struct TReturnInfo
{
    1: optional TReturnCode rtcode; //标准返回码
    2: optional TRspInfo rspinfo;
}


service TransFunc
{
    TReturnInfo ProcessMsg(1: TReqInfo reqinfo);
    oneway void SendMsg(1: TReqInfo reqinfo);  //单向发送消息，不需要返回响应
}

