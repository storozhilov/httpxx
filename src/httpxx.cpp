#include <httpxx.h>
#include <cstring>
#include "string_utils.h"

namespace httpxx
{

/*void parseUri(const std::string& uriStr, std::string& path, std::string& query)
{
	size_t questionMarkPos = uriStr.find('?');
	if (questionMarkPos == std::string::npos) {
		path = decodePercent(uriStr);
		query.clear();
	} else {
		path = decodePercent(uriStr.substr(0, questionMarkPos));
		query = uriStr.substr(questionMarkPos + 1);
	}
}*/

std::pair<std::string, std::string> parseUri(const std::string& uriStr)
{
	size_t questionMarkPos = uriStr.find('?');
	if (questionMarkPos == std::string::npos) {
		return std::pair<std::string, std::string>(decodePercent(uriStr), std::string());
	} else {
		return std::pair<std::string, std::string>(
				decodePercent(uriStr.substr(0, questionMarkPos)),
				uriStr.substr(questionMarkPos + 1));
	}
}

} // namespace httpxx
