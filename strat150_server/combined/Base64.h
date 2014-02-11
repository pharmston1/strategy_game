#include <string>

#ifndef BASE64_H
#define BASE64_H

class Base64{
private:
	std::string chars;
	bool is_base64(char c)
	{
	  return (isalnum(c) || (c == '+') || (c == '/'));
	}

public:
	Base64()
	{
		chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	}
	std::string base64_encode(const char* to_encode, int len);
	std::string base64_decode(std::string encoded);

};

#endif
