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
    4: optional string strAliasName;
    5: optional double dlPrice;
    6: optional string strPic;
    7: optional string strExtend;
    8: list<ProductProperty> pptList;
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

