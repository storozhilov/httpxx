#ifndef HTTPXX_CHAR_H
#define HTTPXX_CHAR_H

#include <ctype.h>

namespace httpxx
{

//! TODO
inline bool isLowAlpha(unsigned char ch)
{
	return ch >= 'a' && ch <= 'z';
}

//! TODO
inline bool isUpAlpha(unsigned char ch)
{
	return ch >= 'A' && ch <= 'Z';
}

//! TODO
inline bool isChar(unsigned char ch)
{
	return (ch <= 0x7F);
}

//! TODO
inline bool isControl(unsigned char ch)
{
	return (ch <= 0x1F) || (ch == 0x7F);
}

//! Inspects if the character is space
/*!
  \param ch Character to inspect
*/
inline bool isSpace(unsigned char ch)
{
	return (ch == 0x20);
}

//! Inspects if the character is tab
/*!
  \param ch Character to inspect
*/
inline bool isTab(unsigned char ch)
{
	return (ch == 0x09);
}

//! Inspects if the character is space or tab
/*!
  \param ch Character to inspect
*/
inline bool isSpaceOrTab(unsigned char ch)
{
	return (isSpace(ch) || isTab(ch));
}

//! Inspects if the character is carriage return
/*!
  \param ch Character to inspect
*/
inline bool isCarriageReturn(unsigned char ch)
{
	return (ch == 0x0D);
}

//! Inspects if the character is line feed
/*!
  \param ch Character to inspect
*/
inline bool isLineFeed(unsigned char ch)
{
	return (ch == 0x0A);
}

//! Inspects if the character is digit
/*!
  \param ch Character to inspect
*/
inline bool isDigit(unsigned char ch)
{
	return isdigit(ch);
}

//! Inspects if the character is hex digit
/*!
  \param ch Character to inspect
*/
inline bool isHexDigit(unsigned char ch)
{
	return isxdigit(ch);
}

//! Inspects if the character is URL-safe
/*!
  \param ch Character to inspect
*/
inline bool isUrlSafe(unsigned char ch)
{
	return ('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z') || isDigit(ch) || (ch == '_');
}

//! TODO
inline bool isSeparator(unsigned char ch)
{
	return	(ch == '(') || (ch == ')') || (ch == '<') || (ch == '>') || (ch == '@') ||
		(ch == ',') || (ch == ';') || (ch == ':') || (ch == '\\') || (ch == '"') ||
		(ch == '/') || (ch == '[') || (ch == ']') || (ch == '?') || (ch == '=') ||
		(ch == '{') || (ch == '}') || isSpaceOrTab(ch);
}

//! TODO
inline bool isAllowedInVersion(unsigned char ch)
{
	return isDigit(ch) || ch == 'H' || ch == 'T' || ch == 'P' || ch == '/' || ch == '.';
}

//! TODO
inline bool isToken(unsigned char ch)
{
	return (isChar(ch) && !isControl(ch) && !isSeparator(ch));
}

//! TODO
inline bool isAlpha(unsigned char ch)
{
	return isLowAlpha(ch) || isUpAlpha(ch);
}

//! TODO
inline bool isAllowedInUri(unsigned char ch)
{
	// See chaper A. of the RFC2936 (http:www.ietf.org/rfc/rfc2396.txt)
	return isAlpha(ch) || isDigit(ch) || ch == '#' || ch == ':' || ch == '?' || ch == ';' || ch == '@' ||
		ch == '&' || ch == '=' || ch == '+' || ch == '$' || ch == ',' || ch == '-' || ch == '.' ||
		ch == '/' || ch == '_' || ch == '!' || ch == '~' || ch == '*' || ch == '\'' || ch == '(' ||
		ch == ')' || ch == '%';
}

//! Returns the value of the hex digit
/*!
  \param ch Hex digit
*/
unsigned char hexValue(unsigned char ch);

} // namespace httpxx

#endif
