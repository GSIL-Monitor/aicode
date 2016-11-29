//Base64.h
//vi:set ts=4 sw=4 nowrap:

#ifndef _ST_BASE64_H_
#define _ST_BASE64_H_

class Base64
{
public:
	static const unsigned char* Alpha64Map;

	static unsigned int GetEncode64Length(unsigned int len);
	static unsigned int GetDecode64Length(unsigned int len);
	
	static bool Encode64(unsigned char *dest, const unsigned char *data, unsigned int max_dest, unsigned int byte_len);
	static bool Decode64(unsigned char *dest, const unsigned char *b64data, unsigned int max_dest, unsigned int byte_len = 0);
};

#endif

