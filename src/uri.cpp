#include <httpxx/uri.h>
#include "string_utils.h"

namespace httpxx
{

Uri::Uri(const std::string& str) :
	_encodedPath(),
	_path(),
	_encodedQuery(),
	_query()
{
	size_t questionMarkPos = str.find('?');
	if (questionMarkPos == std::string::npos) {
		_encodedPath = str;
	} else {
		_encodedPath = str.substr(0U, questionMarkPos);
		_encodedQuery = str.substr(questionMarkPos + 1);
		_query = decodePercent(_encodedQuery);
	}
	_path = decodePercent(_encodedPath);
}

size_t Uri::compose(std::ostream& target) const
{
	size_t composedSize = _encodedPath.size();
	target << _encodedPath;
	if (!_encodedQuery.empty()) {
		composedSize += (_encodedQuery.size() + 1U);
		target << '?' << _encodedQuery;
	}
	return composedSize;
}

} // namespace httpxx
