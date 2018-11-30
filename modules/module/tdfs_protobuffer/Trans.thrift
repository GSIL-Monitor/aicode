//目的是定义通讯传输服务，实现点对点直接通讯，不在经过中间节点中转。
//通过定义一个服务来实现该目标。
//

namespace cpp Trans.Service

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

