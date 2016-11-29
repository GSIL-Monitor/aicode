
#include "sasl.h"
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "gsasl/gsasl-mech.h"

Gsasl*	g_GsaslCtx = NULL;

const char* CRAMMD5 = "CRAM-MD5";

/**
*功能：
*	sasl库加载
*/
void InitSasl()
{
	if (g_GsaslCtx==NULL)
	{
		gsasl_init(&g_GsaslCtx);
	}
}

/**
*功能：
*	sasl库卸载
*/
void UninitSasl()
{
	if (g_GsaslCtx!=NULL)
	{
		gsasl_done(g_GsaslCtx);
		g_GsaslCtx = NULL;
	}
}

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
							char** b64output)
{
	int rc = -1;
	char* input = NULL;
	Gsasl_session* sctx = NULL;
	/* Create new authentication session. */
	if ((rc = gsasl_client_start(g_GsaslCtx,CRAMMD5, &sctx)) != GSASL_OK)
	{
		return NULL;
	}
	/* 设置用户名及密码  */
	gsasl_property_set(sctx, GSASL_AUTHID, uid);
	gsasl_property_set(sctx, GSASL_PASSWORD, pwd);

	rc = gsasl_step64(sctx, input, b64output);

	if(rc != GSASL_OK && rc != GSASL_NEEDS_MORE)
	{
		if(*b64output) gsasl_free(*b64output);
		(*b64output) = NULL;
		return NULL;
	}
	return sctx;
}

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
						char** b64output)
{
	int rc = -1;
	char* input = NULL;
	Gsasl_session* sctx = NULL;

	/* Create new authentication session. */
	if ((rc = gsasl_client_start(g_GsaslCtx,CRAMMD5, &sctx)) != GSASL_OK)
	{
		return NULL;
	}

	/* 设置密码*/
	gsasl_property_set(sctx, GSASL_PASSCODE, ks);
	gsasl_property_set(sctx, GSASL_PASSWORD, ks);
	
	/* 设置用户名及密码  */
	gsasl_property_set(sctx, GSASL_AUTHID, uid);

	rc = gsasl_step64(sctx, input, b64output);
	if(rc != GSASL_OK && rc != GSASL_NEEDS_MORE)
	{
		if(*b64output) gsasl_free(*b64output);
		(*b64output) = NULL;
		return NULL;
	}

	return sctx;
}

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
				char** ks)
{
	int rc = -1;
	char * b64output = NULL;
	rc = gsasl_step64(sctx, input, &b64output);

	if(b64output) gsasl_free(b64output);

	if(rc == GSASL_OK)
	{
		(*ks) = (char*)gsasl_property_fast(sctx, GSASL_PASSCODE);
		return 0;
	}
	return -1;
}

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
				char **ks2)
{
	int res=0;
	char *saltedpassword;
	size_t  outlen = 0;
	
	res=gsasl_hmac_md5 (ks, strlen(ks),
		   uid, strlen (uid),
		   &saltedpassword);

	if(res!=GSASL_OK) {
  	return res;
  }

  res = gsasl_base64_to (saltedpassword, 16, ks2, &outlen);
  free(saltedpassword);
  return res;
}

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
    const char* input)
{
    int rc = -1;
    char * b64output = NULL;
    Gsasl_session* sctx = NULL;

    if ((rc = gsasl_server_start(g_GsaslCtx, CRAMMD5, &sctx)) != GSASL_OK)
    {
        return -1;
    }

    gsasl_property_set(sctx, GSASL_PASSWORD, ks2);
    gsasl_property_set(sctx, GSASL_PASSCODE, ks2);
    gsasl_property_set(sctx, GSASL_QOPS, "3600");

    rc = gsasl_step64(sctx, input, &b64output);

    if (b64output) gsasl_free(b64output);

    gsasl_finish(sctx);
    if (rc == GSASL_OK)
    {
        return 0;
    }
    return -1;
}

/**
*功能：
*	释放会话对象
*/
void DeleteSession(void * sctx)
{
	gsasl_finish((Gsasl_session *)sctx);
	sctx = NULL;
}
