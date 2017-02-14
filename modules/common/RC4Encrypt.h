#ifndef _RC4_ENCRYPT_
#define _RC4_ENCRYPT_

#include <string>
#include <vector>

class RC4Encrypt
{
public:
	std::string RC4Crypt(const std::string &input, const std::string &key);

	//加密后使用Base64编码
	std::string RC4EncryptBase64(const std::string &data, const std::string &key);
	std::string RC4DecryptBase64(const std::string &cipher, const std::string &key);

private:
	std::vector<unsigned char> mS;

	void swap(unsigned char &a, unsigned char &b);
	void RC4Init(const std::string &key);

};

#endif