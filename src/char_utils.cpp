#include "char_utils.h"

namespace httpxx
{

unsigned char hexValue(unsigned char ch)
{
	if (isDigit(ch)) {
		return (ch - '0');
	} else if (ch >= 'a' && ch <= 'f') {
		return (ch - 'a' + 10);
	} else if (ch >= 'A' && ch <= 'F') {
		return (ch - 'A' + 10);
	} else {
		return 0;
	}
}

} // namespace httpxx
