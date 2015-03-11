#ifndef HTTPXX_HEADERS_H
#define HTTPXX_HEADERS_H

#include <httpxx/common.h>
#include <map>
#include <ostream>

namespace httpxx
{

//! Container for HTTP-headers
class Headers : public std::multimap<std::string, std::string, CaseInsensitiveComparator>
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
	 * \param name Header to inspect for value
	 * \param value Value to inspect against
	 * \return TRUE if header contains 'header' => 'value' pair
	 */
	bool have(const std::string& name, const std::string& value) const
	{
		std::pair<const_iterator, const_iterator> range = equal_range(name);
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
	//! Composes headers into output stream
	/*!
	 * \param target Output stream to compose headers into
	 * \note Method does not do any preprocessing to header
	 *       names and values - they should conform RFC.
	 *       If you want header value to be multiline, insert 
	 *       correct LWS(s) manually.
	 */
	inline void compose(std::ostream& target) const
	{
		for (const_iterator i = begin(); i != end(); ++i) {
			target << i->first << ": " << i->second  << "\r\n";
		}
	}
	//! Adds header
	/*!
	 * \param name Header name
	 * \param value Header value
	 */
	inline void add(const std::string& name, const std::string& value)
	{
		insert(value_type(name, value));
	}
	//! Returns size of composed headers
	/*!
	 * \sa compose()
	 */
	inline size_t composedSize() const
	{
		size_t result = 0U;
		for (const_iterator i = begin(); i != end(); ++i) {
			result += (i->first.length() + i->second.length() + 4U);
		}
		return result;
	}
};

} // namespace httpxx

#endif
