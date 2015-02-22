#ifndef HTTPXX_STRING_H
#define HTTPXX_STRING_H

#include <string>

namespace httpxx
{

//! Encodes string using Percent-encoding (see http://en.wikipedia.org/wiki/Percent-encoding)
std::string encodePercent(const std::string &str);

//! Decodes string using Percent-encoding (see http://en.wikipedia.org/wiki/Percent-encoding)
std::string decodePercent(const std::string &str);

// Converts string to unsigned int
/*!
 * TODO
 */
unsigned int toUnsignedInt(const std::string& str, bool isHex = false);

//! Trims space characters on the both ends of the string
void trim(std::string &str);
//! Trims space characters on the both ends of the string and returns the result
std::string trim(const std::string &str);

} // namespace httpxx

#endif
