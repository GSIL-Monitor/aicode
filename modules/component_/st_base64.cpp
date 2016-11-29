//Base64.cpp
//vi:set ts=4 sw=4 nowrap:

#include "st_base64.h"

#include <string.h>
#include <stdio.h>
//#ifdef WIN32
//#define snprintf _snprintf;
//#endif
const unsigned char* Base64::Alpha64Map = /* 0 ... 63 => ASCII - 64 */
	(const unsigned char*)"./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

unsigned int Base64::GetEncode64Length(unsigned int len)
{
	return (8*len+5)/6;
}

unsigned int Base64::GetDecode64Length(unsigned int len)
{
    return (len * 6)/8;
}

bool Base64::Encode64(unsigned char *dest, const unsigned char *data, unsigned int max_dest, unsigned int byte_len)
{
	unsigned char sixbits;
	unsigned char fresh_bits_holder=0;

	//calculate size of b64 encoded number, round up
	unsigned int req_length = GetEncode64Length(byte_len);
	//init bits_left to the number of extra bits that will be in a base 64
	//encoded version of "data[0..byte_len]"
	int bits_left= (6*req_length) - (8*byte_len);

	//make sure max_dest is > req_length
	if(max_dest<=req_length)
		return false;
	
//	unsigned char* end = dest + max_dest - 1;
    while(1)
	{
		if(bits_left<6)
		{
			//apply remaining bits
			sixbits = (fresh_bits_holder << (6-bits_left)) & 0x3F;

			//try to get more bits
			if(byte_len>0)
			{
				fresh_bits_holder=*data;
				--byte_len;
				++data;

				//bits gotten so far = bits_left
				//more bits needed now = 6 - gotten so far
				//amount to right shift holder = 8-needed bits
				sixbits |= fresh_bits_holder >> ( 8-(6-bits_left) );

				//8 new bits, 6 taken away
				bits_left= 8 + bits_left - 6;
			}
			else if(bits_left<=0)
			{
				*dest=0;
				break;
			}
		}
		else
		{
			sixbits = (fresh_bits_holder >> (bits_left-6)) & 0x3F;
			bits_left -= 6;
			fresh_bits_holder >>= 6;
		}

		//consume 6 bits from stream
		*dest++ = Alpha64Map[sixbits];
    }

	return true;
}

bool Base64::Decode64(unsigned char *dest, const unsigned char *b64data, unsigned int max_dest, unsigned int byte_len)
{
	unsigned int len = (byte_len? byte_len : strlen((const char*)b64data));
	unsigned int nbytes = GetDecode64Length(len);

	//initialize bits_left to the negative number of bits to skip over (0 to -5)
	//(a value of -6 is possible but we should never encode that). Must be signed
	int bits_left= (8*nbytes) - (6*len);
	unsigned int bit_holder=0;

	//max_dest must have room for the data (no trailing null needed)
	if(max_dest < nbytes)
		return false;
	
	while( nbytes > 0)
	{
		//fill one byte

		while(bits_left<8)
		{
			bit_holder <<= 6;
			
			//gather 6 bits from b64data
			{
				unsigned char sixbits;

				//46 - 57 (12)
				//value = enc-46
				//65 - 90 (26)
				//value = enc-65+12 = enc - 53
				//97 - 122 (26)
				//value = enc-97+12+26 = enc - 59
				if(*b64data<46)
					return false;
				else if(*b64data<=57)
					sixbits = *b64data - 46;
				else if(*b64data<65)
					return false;
				else if(*b64data<=90)
					sixbits = *b64data - 53;
				else if(*b64data<97)
					return false;
				else if(*b64data<=122)
					sixbits = *b64data - 59;
				else
					return false;

				bit_holder |= sixbits;
				bits_left += 6;
				++b64data;
			}
				
		}

		//apply 8 bits from bit_holder
		//(bits_left - 8) == left overs for next time
		bits_left -= 8;
		*dest = (unsigned char)(bit_holder>>bits_left);

		++dest;
		--nbytes;
	}

	return true;
}

