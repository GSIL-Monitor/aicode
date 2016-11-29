
#ifndef _SASL_H_
#define _SASL_H_

#include "gsasl/gsasl.h"

/**
*���ܣ�
*	sasl�����
*/
void InitSasl();

/**
*���ܣ�
*	sasl��ж��
*/
void UninitSasl();

/**
*���ܣ�
*	����id,�������ɵ�¼Ʊ��
*������
* 	uid:	�û���
* 	pwd:	��������
* 	b64output:	���ɵĵ�¼Ʊ��,����free��
*����ֵ��
* 	�Ự����ָ��
*/
Gsasl_session* LoginByPwd(const char* uid,
							const char* pwd,
							char** b64output);

/**
*���ܣ�
*	����id/��̬id,ks/ks'���ɵ�¼Ʊ��
*������
* 	uid:	�û���
* 	ks:		
* 	b64output:	���ɵĵ�¼Ʊ��
*����ֵ��
* 	�Ự����ָ��
*/
Gsasl_session* LoginByKs(const char* uid,
						const char* ks,
						char** b64output);

/**
*���ܣ�
*	У��ƽ̨���ص�¼Ӧ���ж��Ƿ��¼�ɹ�������ȡks
*������
* 	sctx:	�Ự����ָ��
* 	input:	��½Ӧ����msg�ֶ�����
* 	ks:		���ɵ�ks,����free��
*����ֵ��
* 	0:�ɹ�
*/
int Verify(Gsasl_session * sctx,
				const char* input,
				char** ks);

/**
*���ܣ�
*	������̬id,ks�����ks'
*������
* 	uid:	��̬id
* 	ks: 	�����豸��ks
* 	ks2:	������ks'������free��
*����ֵ��
* 	0:�ɹ�
*/
int Getks2 (const char *uid,
				const char *ks,
				char **ks2);

/**
*���ܣ�
*	У����̬�豸�ĵ�¼����
*������
* 	ks2:	ks'
* 	input:	��½��Ϣ��pwd�ֶ�����
*����ֵ��
* 	0:�ɹ�
*/
int Verify2(const char* ks2,
    const char* input);

/**
*���ܣ�
*	�ͷŻỰ����
*/
void DeleteSession(void * sctx);

#endif
