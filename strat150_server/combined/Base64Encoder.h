#include <string>

class Base64Encoder{
private:
	std::string chars;
public:
	Base64Encoder()
	{
		chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	}
	bool is_base64(char c);
	std::string base64_encode(const char* to_encode, int len);
	std::string base64_decode(std::string encoded);

};