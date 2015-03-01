#include <httpxx/params.h>
#include <httpxx/uri.h>
#include "string_utils.h"
#include <sstream>

namespace httpxx
{

Params::Params(const std::string& str) :
	std::multimap<std::string, std::string>()
{
	parse(str);
}

Params::Params(const Uri& uri) :
	std::multimap<std::string, std::string>()
{
	parse(uri.query());
}

void Params::parse(const std::string& str)
{
	size_t pos = 0;
	while (pos < str.length()) {
		// Parsing param name
		std::string paramName;
		while (pos < str.length()) {
			if (str[pos] == '=') {
				++pos;
				break;
			} else if (str[pos] == '&') {
				break;
			}
			paramName.append(1, str[pos]);
			++pos;
		}
		// Parsing param value
		std::string paramValue;
		while (pos < str.length()) {
			if (str[pos] == '&') {
				++pos;
				break;
			}
			paramValue.append(1, str[pos]);
			++pos;
		}
		// Adding name/value pair to the result
		insert(value_type(decodePercent(paramName), decodePercent(paramValue)));
	}
}

size_t Params::compose(std::ostream& target) const
{
	size_t composedSize = 0U;
	for (const_iterator i = begin(); i != end(); ++i) {
		if (i->first.empty()) {
			continue;
		}
		if (composedSize > 0U) {
			target << '&';
			++composedSize;
		}
		std::string encodedName = encodePercent(i->first);
		std::string encodedValue = encodePercent(i->second);
		target << encodedName << '=' << encodedValue;
		composedSize += (encodedName.size() + 1U + encodedValue.size());
	}
	return composedSize;
}

size_t Params::composedSize() const
{
	std::ostringstream target;
	return compose(target);
}

} // namespace httpxx
