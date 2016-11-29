#ifndef _MD5_H
#define _MD5_H

/* MD5.H - header file for MD5C.C */

/* Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
���� rights reserved.

���� License to copy and use this software is granted provided that it
���� is identified as the "RSA Data Security, Inc. MD5 Message-Digest
��? Algorithm" in all material mentioning or referencing this software
     or this function.

     License is also granted to make and use derivative works provided
     that such works are identified as "derived from the RSA Data
     Security, Inc. MD5 Message-Digest Algorithm" in all material
     mentioning or referencing the derived work.

     RSA Data Security, Inc. makes no representations concerning either
     the merchantability of this software or the suitability of this
     software for any particular purpose. It is provided "as is"
     without express or implied warranty of any kind.

     These notices must be retained in any copies of any part of this
     documentation and/or software.

     2007-09-15 Last modified by cheungmine.
  */


    /* MD5 context. */
    typedef struct tagMD5_CTX
    {
        unsigned int state[4];          /* state (ABCD) */
        unsigned int count[2];          /* number of bits, modulo 2^64 (lsb first) */
        unsigned char buffer[64];       /* input buffer */
    }
    MD5_CTX;



        extern void   MD5_Init(MD5_CTX *);
        extern void   MD5_Update(MD5_CTX *, const unsigned char *str, unsigned int len);
        extern void   MD5_Finish(unsigned char[16], MD5_CTX *);

        extern char*  MD5_sign(const unsigned char *str, unsigned int len, int isBCD);

        extern int    MD5_File(const char* filename, unsigned char md[16]);
        extern int    MD5_File_V2(const char* filename, char str[32]);
        extern int    MD5_Buffer(const void* buf, int len, unsigned char md[16]);
        extern int    MD5_Buffer_V2(const void* buf, int len, char str[32]);

        extern int BCD2ASCII(const unsigned char* bcd, int bcdLen, unsigned char* ascii);
        extern int ASCII2BCD(const unsigned char* ascii, unsigned char* bcd, int *bcdLen);

#ifndef _VC_WIN
        void _itoa(unsigned long val, char *buf, unsigned radix);
#endif

#endif       /* _MD5_H__ */



