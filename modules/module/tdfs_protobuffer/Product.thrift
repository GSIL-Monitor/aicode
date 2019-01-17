//产品服务相关定义内容
//
//

namespace cpp Product.Service

const i32 PDT_SUCCESS_CODE = 0;
const i32 PDT_FAILED_CODE = -1;


struct ProductRTInfo
{
    1: optional i32 iRtCode = PDT_SUCCESS_CODE;
    2: optional string strRtMsg = "Ok";
}

struct SaleCompanyInfo
{
    1: optional string strID;
    2: optional string strName;
    3: optional string strEmail;
    4: optional string strPhone;
    5: optional string strBackReceiver;
    6: optional string strBackPhone;
    7: optional string strExtend;
}

struct ProductProperty
{
    1: optional string strID;
    2: optional string strPdtID;
    3: optional i32    iType;
    4: optional string strName;
    5: optional string strValue;
    6: optional string strExtend;
}

struct ProductInfo
{
    1: optional string strID;
    2: optional string strName;
    3: optional i32    iType;
    4: optional string strTypeName;
    5: optional string strAliasName;
    6: optional double dlPrice;
    7: optional string strPic;
    8: optional string strExtend;
    9: list<ProductProperty> pptList;
}

struct OrderDetail
{
    1: optional string strID;
    2: optional string strOrdID;
    3: optional string strPdtID;
    4: optional i32    iNumber;
    5: optional double dlPrice;
    6: optional double dlTotalPrice;
    7: optional string strExtend;
}

struct OrderInfo
{
    1: optional string strID;
    2: optional string strName;
    3: optional i32    iType;
    4: optional string strUserID;
    5: optional double dlTotalPrice;
    6: optional i32    iOrdStatus;
    7: optional string strExpressInfo;
    8: optional string strHisOrdStatus;
    9: optional string strAddress;
   10: optional string strReceiver;
   11: optional string strPhone;
   12: optional string strBackReceiver;
   13: optional string strBackPhone;
   14: optional string strBackExpressInfo;
   15: optional string strCreateDate;
   16: optional string strExtend;
   17: list<OrderDetail> orddtList;
}

//开通申请信息
struct OpenRequest
{
    1: optional string strOpReqID;
    2: optional string strReqUserID;
    3: optional string strReqUserName;
    4: optional i32    iReqStatus;
    5: optional string strReqInfo;
    6: optional string strReqDate;
    7: optional string strExtend;
}

//产品类型信息
struct ProductType
{
    1: optional string strPdtTpID;
    2: optional i32    iType;
    3: optional string strTypeName;
    4: optional string strPic;
    5: optional i32    iIndex;
    6: optional string strExtend;
}


//增加产品类型返回信息
struct AddProductTypeRT
{
    1: optional ProductRTInfo rtcode;
    2: optional string strTypeID;
}

//查询产品类型返回信息
struct QueryProductTypeRT
{
    1: optional ProductRTInfo rtcode;
    2: optional ProductType pdttype;
}

//查询所有产品类型返回信息
struct QueryAllProductTypeRT
{
    1: optional ProductRTInfo rtcode;
    2: optional list<ProductType> pdttypelist;
}

//增加开通申请返回信息
struct AddOpenRequestRT
{
    1: optional ProductRTInfo rtcode;
    2: optional string strOpReqID;
}

//查询开通申请返回信息
struct QueryOpenRequestRT
{
    1: optional ProductRTInfo rtcode;
    2: optional string strReqUserID;
    3: optional string strReqUserName;
    4: list<OpenRequest> opreqlist;
}

//查询所有开通申请返回信息
struct QueryAllOpenRequestRT
{
    1: optional ProductRTInfo rtcode;
    2: list<OpenRequest> opreqlist;
}

//查询所有开通申请参数
struct QueryAllOpenRequestParam
{
    1: optional i32 iReqStatus;
    2: optional string strBeginDate;
    3: optional string strEndDate;
    4: optional string strBeginIndex;
}

struct AddProductRT
{
    1: optional ProductRTInfo rtcode;
    2: optional string strPdtID;
}

struct QueryProductRT
{
    1: optional ProductRTInfo rtcode;
    2: optional ProductInfo pdt;
}

struct QueryAllProductRT
{
    1: optional ProductRTInfo rtcode;
    2: optional list<ProductInfo> pdtlist;
}

struct AddProductPropertyRT
{
    1: optional ProductRTInfo rtcode;
    2: optional string strPdtpptID;
}

service ProductService
{
    AddProductTypeRT AddProductType(1: string strSid, 2: string strUserID, 3: ProductType pdttype);
    ProductRTInfo RemoveProductType(1: string strSid, 2: string strUserID, 3: string strTypeID);
    ProductRTInfo ModifyProductType(1: string strSid, 2: string strUserID, 3: ProductType pdttype);
    
    QueryProductTypeRT QueryProductType(1: string strSid, 2: string strUserID, 3: string strTypeID);
    QueryAllProductTypeRT QueryAllProductType(1: string strSid, 2: string strUserID);
    
    AddOpenRequestRT AddOpenRequest(1: string strSid, 2: string strUserID, 3: OpenRequest opreq);
    ProductRTInfo RemoveOpenRequest(1: string strSid, 2: string strUserID, 3: string strOpReqID);
    ProductRTInfo ModifyOpenRequest(1: string strSid, 2: string strUserID, 3: OpenRequest opreq);
    
    QueryOpenRequestRT QueryOpenRequest(1: string strSid, 2: string strUserID, 3: string strReqID, 4: string strReqUserID, 5: string strReqUserName);
    QueryAllOpenRequestRT QueryAllOpenRequest(1: string strSid, 2: string strUserID, 3: QueryAllOpenRequestParam qryparam);
    
    
    AddProductRT AddProduct(1: string strSid, 2: string strUserID, 3: ProductInfo pdt);
    ProductRTInfo RemoveProduct(1: string strSid, 2: string strUserID, 3: string strPdtID);
    ProductRTInfo ModifyProduct(1: string strSid, 2: string strUserID, 3: string strPdtID, 4: ProductInfo pdt);
    
    QueryProductRT QueryProduct(1: string strSid, 2: string strUserID, 3: string strPdtID);
    QueryAllProductRT QueryAllProduct(1: string strSid, 2: string strUserID);
    
    AddProductPropertyRT AddProductProperty(1: string strSid, 2: string strUserID, 3: string strPdtID, 4: ProductProperty pdtppt);
    ProductRTInfo RemoveProductProperty(1: string strSid, 2: string strUserID, 3: string strPdtID, 4: string strPdtpptID);
    
}

struct AddOrdRT
{
    1: optional ProductRTInfo rtcode;
    2: optional string strOrdID;
}

struct AddOrdDetailRT
{
    1: optional ProductRTInfo rtcode;
    2: optional string strOrddtID;
}

struct QueryOrdRT
{
    1: optional ProductRTInfo rtcode;
    2: optional OrderInfo ord;
}

struct QueryAllOrdRT
{
    1: optional ProductRTInfo rtcode;
    2: optional list<OrderInfo> ordlist;
}

struct QueryAllOrdParam
{
    1: optional i32 iType;
    2: optional i32 iOrdStatus;
    3: optional string strReceiver;
    4: optional string strPhone;
    5: optional string strPdtID;
    6: optional string strBeginDate;
    7: optional string strEndDate;
    8: optional string strBeginIndex;
}

service OrderService
{
    AddOrdRT AddOrd(1: string strSid, 2: string strUserID, 3: OrderInfo ord);
    ProductRTInfo RemoveOrd(1: string strSid, 2: string strUserID, 3: string strOrdID);
    ProductRTInfo ModifyOrd(1: string strSid, 2: string strUserID, 3: string strOrdID, 4: OrderInfo ord);
    
    AddOrdDetailRT AddOrdDetail(1: string strSid, 2: string strUserID, 3: OrderDetail orddt);
    ProductRTInfo RemoveOrdDetail(1: string strSid, 2: string strUserID, 3: string strOrdID, 4: string strOrddtID);
    
    QueryOrdRT QueryOrd(1: string strSid, 2: string strUserID, 3: string strOrdID);
    QueryAllOrdRT QueryAllOrd(1: string strSid, 2: string strUserID, 3: QueryAllOrdParam qryparam);

}

