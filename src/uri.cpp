#include <httpxx/uri.h>
#include "string_utils.h"

namespace httpxx
{

Uri::Uri(const std::string& str) :
	_encodedPath(),
	_path(),
	_query()
{
	size_t questionMarkPos = str.find('?');
	if (questionMarkPos == std::string::npos) {
		_encodedPath = str;
	} else {
		_encodedPath = str.substr(0U, questionMarkPos);
		_query = str.substr(questionMarkPos + 1);
	}
	_path = decodePercent(_encodedPath);
}

size_t Uri::compose(std::ostream& target) const
{
	size_t composedSize = _encodedPath.size();
	target << _encodedPath;
	if (!_query.empty()) {
		composedSize += (_query.size() + 1U);
		target << '?' << _query;
	}
	return composedSize;
}

} // namespace httpxx
