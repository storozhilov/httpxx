#include "string_utils.h"
#include "char_utils.h"
#include <sstream>
#include <iomanip>
#include <stdexcept>

namespace httpxx
{

std::string encodePercent(const std::string &str)
{
	std::ostringstream encodedString;
	encodedString.setf(std::ios::uppercase);
	encodedString.setf(std::ios::hex, std::ios::basefield);
	encodedString.unsetf(std::ios::showbase);
	encodedString.fill('0');
	for (size_t i = 0; i < str.length(); ++i) {
		unsigned char code = str[i];
		if (isSpace(code)) {
			encodedString << '+';
		} else if (!isUrlSafe(code)) {
			encodedString << '%' << std::setw(2) << static_cast<unsigned int>(code);
		} else {
			encodedString << code;
		}
	}
	return encodedString.str();
}

std::string decodePercent(const std::string &str)
{
	std::string decodedString;
	size_t i = 0;
	while (i < str.length()) {
		if (str[i] == '%') {
			if ((i + 2) >= str.length() || !isHexDigit(str[i + 1]) || !isHexDigit(str[i + 2])) {
				decodedString += str[i];
				++i;
			} else {
				unsigned char charCode = hexValue(str[i + 1]) * 16 + hexValue(str[i + 2]);
				decodedString += charCode;
				i += 3;
			}
		} else if (str[i] == '+') {
			decodedString += ' ';
			++i;
		} else {
			decodedString += str[i];
			++i;
		}
	}
	return decodedString;
}

unsigned int toUnsignedInt(const std::string& str, bool isHex)
{
	std::string strToParse = str;
	trim(strToParse);
	if (strToParse.empty()) {
		return 0U;
	}
	unsigned int result = 0U;
	if (isHex) {
		for (size_t curPos = 0; curPos < strToParse.size(); ++curPos) {
			unsigned int newResult;
			if (strToParse[curPos] >= '0' && strToParse[curPos] <= '9') {
				newResult = result * 16U + strToParse[curPos] - '0';
			} else if (strToParse[curPos] >= 'a' && strToParse[curPos] <= 'f') {
				newResult = result * 16U + strToParse[curPos] - 'a' + 10U;
			} else if (strToParse[curPos] >= 'A' && strToParse[curPos] <= 'F') {
				newResult = result * 16U + strToParse[curPos] - 'A' + 10U;
			} else {
				std::ostringstream msg;
				msg << "Invalid hex digit character '" << strToParse[curPos] <<
					"' on " << curPos << " position";
				throw std::runtime_error(msg.str());
			}
			if (newResult < result) {
				throw std::runtime_error("Integer overflow has been detected");
			}
			result = newResult;
		}
	} else {
		for (size_t curPos = (strToParse[0] == '+' ? 1 : 0); curPos < strToParse.size(); ++curPos) {
			if (!isDigit(strToParse[curPos])) {
				std::ostringstream msg;
				msg << "Invalid decimal digit character '" << strToParse[curPos] <<
					"' on " << curPos << " position";
				throw std::runtime_error(msg.str());
			}
			unsigned int newResult = result * 10 + strToParse[curPos] - '0';
			if (newResult < result) {
				throw std::runtime_error("Integer overflow has been detected");
			}
			result = newResult;
		}
	}
	return result;
}

void trim(std::string &str)
{
	std::string charsToTrim(" \t\r\n");
	std::string::size_type pos = str.find_last_not_of(charsToTrim);
	if (pos == std::string::npos) {
		str.clear();
		return;
	}
	str.erase(pos + 1);
	pos = str.find_first_not_of(charsToTrim);
	str.erase(0, pos);
}

std::string trim(const std::string &str)
{
	std::string s(str);
	trim(s);
	return s;
}

} // namespace httpxx
