#ifndef HTTPXX_PARAMS_H
#define HTTPXX_PARAMS_H

#include <httpxx/common.h>
#include <map>

namespace httpxx
{

class Uri;

//! GET/POST params
class Params : public std::multimap<std::string, std::string>
{
public:
	Params() :
		std::multimap<std::string, std::string>()
	{}
	//! Constructs params by parsing a supplied string
	/*!
	 * \param str String to parse
	 */
	Params(const std::string& str);
	//! Constructs params from URI
	/*!
	 * \param uri URI to construct params from
	 */
	Params(const Uri& uri);

	//! Inspects params for item
	/*!
	 * \param name Item name to inspect for existence
	 * \return TRUE if item exists in params
	 */
	inline bool have(const std::string& name) const
	{
		std::pair<const_iterator, const_iterator> range = equal_range(name);
		return range.first != range.second;
	}

	//! Inspects params for 'name' => 'value' pair
	/*!
	 * \param name Item name to inspect for existence
	 * \param value Value to inspect against
	 * \return TRUE if name contains 'name' => 'value' pair
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

	//! Returns first item value
	/*!
	 * \param name Item name to return value of
	 * \return Item value
	 */
	inline std::string value(const std::string& name) const
	{
		std::pair<const_iterator, const_iterator> range = equal_range(name);
		return range.first == range.second ? std::string() : range.first->second;
	}

	//! Adds item to param
	/*!
	 * \param name Param name
	 * \param value Param value
	 */
	inline void add(const std::string& name, const std::string& value)
	{
		insert(value_type(name, value));
	}

	//! Composes params into stream
	/*!
	 * \param target A reference to output stream to compose params into
	 * \return Composed URI size
	 */
	size_t compose(std::ostream& target) const;

	//! Returns size of composed params
	size_t composedSize() const;
private:
	void parse(const std::string& str);
};

} // namespace httpxx

#endif
