#ifndef HTTPXX_HEADERS_H
#define HTTPXX_HEADERS_H

#include <httpxx/common.h>
#include <map>

namespace httpxx
{

//! Container for HTTP-headers
class headers : public std::multimap<std::string, std::string, CaseInsensitiveComparator>
{
public:
	//! Inspects headers for header
	/*!
	 * \param header Header to inspect for existence
	 * \return TRUE if header exists in headers
	 */
	inline bool have(const std::string& header) const
	{
		std::pair<const_iterator, const_iterator> range = equal_range(header);
		return range.first != range.second;
	}

	//! Inspects headers for 'header' => 'value' pair
	/*!
	 * \param header Header to inspect for value
	 * \param value Value to inspect against
	 * \return TRUE if header contains 'header' => 'value' pair
	 */
	bool have(const std::string& paramName, const std::string& value) const
	{
		std::pair<const_iterator, const_iterator> range = equal_range(paramName);
		for (const_iterator i = range.first; i != range.second; ++i) {
			if (i->second == value) {
				return true;
			}
		}
		return false;
	}

	//! Returns first header value
	inline std::string value(const std::string& header) const
	{
		std::pair<const_iterator, const_iterator> range = equal_range(header);
		return range.first == range.second ? std::string() : range.first->second;
	}

};

} // namespace httpxx

#endif
