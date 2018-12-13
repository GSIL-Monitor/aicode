#include "CommonUtility.h"

#include "RC4Encrypt.h"

using std::string;

inline void RC4Encrypt::swap(unsigned char &a, unsigned char &b)
{
	unsigned char temp = a;
	a = b;
	b = temp;
}

/*KSA算法初始化s-box*/
void RC4Encrypt::RC4Init(const string &key)
{
	if (key.empty())
	{
		return;
	}

	mS.clear();

	for (int i = 0; i < 256; i++)
	{
		mS.push_back(i);
	}

	unsigned int k = 0;
	unsigned int len = key.length();
	for (unsigned int j = 0; j < 256; j++)
	{
		/*和255进行逻辑与运算相当于对256求模*/
		k = (k + mS[j] + key[j % len]) & 255;
		swap(mS[j], mS[k]);
	}
}

/*PRGA算法生成伪随机序列*/
string RC4Encrypt::RC4Crypt(const string &input, const string &key)
{
	if (key.empty())
	{
		return input;
	}

	RC4Init(key);

	string output;

	int x = 0;
	int y = 0;
	unsigned int len = input.length();
	for (unsigned int i = 0; i < len; i++)
	{
		x = (x + 1) & 255;
		y = (y + mS[x]) & 255;
		swap(mS[x], mS[y]);
		unsigned int t = (mS[x] + mS[y]) & 255;
		output.push_back(input[i] ^ mS[t]);
	}

	return output;
}

string RC4Encrypt::RC4EncryptBase64(const std::string &data, const std::string &key)
{
	string cipher = RC4Crypt(data, key);
	return Encode64((const unsigned char*)cipher.c_str(), cipher.length());
}

string RC4Encrypt::RC4DecryptBase64(const std::string &cipher, const std::string &key)
{
	string data = Decode64((const unsigned char*)cipher.c_str(), cipher.length());
	return RC4Crypt(data, key);
}