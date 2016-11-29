
#ifndef _SASL_H_
#define _SASL_H_

#include "gsasl/gsasl.h"

/**
*功能：
*	sasl库加载
*/
void InitSasl();

/**
*功能：
*	sasl库卸载
*/
void UninitSasl();

/**
*功能：
*	根据id,密码生成登录票据
*参数：
* 	uid:	用户名
* 	pwd:	明文密码
* 	b64output:	生成的登录票据,用完free掉
*返回值：
* 	会话对象指针
*/
Gsasl_session* LoginByPwd(const char* uid,
							const char* pwd,
							char** b64output);

/**
*功能：
*	根据id/生态id,ks/ks'生成登录票据
*参数：
* 	uid:	用户名
* 	ks:		
* 	b64output:	生成的登录票据
*返回值：
* 	会话对象指针
*/
Gsasl_session* LoginByKs(const char* uid,
						const char* ks,
						char** b64output);

/**
*功能：
*	校验平台返回登录应答，判断是否登录成功，并获取ks
*参数：
* 	sctx:	会话对象指针
* 	input:	登陆应答中msg字段内容
* 	ks:		生成的ks,用完free掉
*返回值：
* 	0:成功
*/
int Verify(Gsasl_session * sctx,
				const char* input,
				char** ks);

/**
*功能：
*	根据生态id,ks计算出ks'
*参数：
* 	uid:	生态id
* 	ks: 	网关设备的ks
* 	ks2:	产生的ks'，用完free掉
*返回值：
* 	0:成功
*/
int Getks2 (const char *uid,
				const char *ks,
				char **ks2);

/**
*功能：
*	校验生态设备的登录请求
*参数：
* 	ks2:	ks'
* 	input:	登陆消息中pwd字段内容
*返回值：
* 	0:成功
*/
int Verify2(const char* ks2,
    const char* input);

/**
*功能：
*	释放会话对象
*/
void DeleteSession(void * sctx);

#endif
