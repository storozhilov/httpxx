#ifndef HTTPXX_URI_H
#define HTTPXX_URI_H

#include <httpxx/common.h>
#include <ostream>

namespace httpxx
{

class Params;

//! Uniform Resource Identifier (<a href="https://www.ietf.org/rfc/rfc3986.txt">RFC-3986</a>)
/*!
 * \note Only <i>"<path>?<query>"</i> format of URI is supported at the moment
 *       (TODO: Full RFC-3986 support).
 */
class Uri
{
public:
	//! Creates URI from path and query
	/*!
	 * TODO: Implementation
	 * \param path Path part of URI
	 * \param query Query part of URI
	 */
	Uri(const std::string& path, const std::string& query);
	//! Creates URI from path and params
	/*!
	 * TODO: Implementation
	 * \param path Path part of URI
	 * \param params Params for query part composition
	 */
	Uri(const std::string& path, const Params& params);
	//! Creates URI by parsing supplied string
	/*!
	  \param str String to parse
	*/
	Uri(const std::string& str);

	//! Returns percent-encoded (raw) path part of the URI
	inline const std::string& encodedPath() const
	{
		return _encodedPath;
	}
	//! Returns path part of the URI
	inline const std::string& path() const
	{
		return _path;
	}
	//! Returns percent-encoded query part of the URI
	/*!
	 * \note This value should be parsed for params extraction!
	 */
	inline const std::string& encodedQuery() const
	{
		return _encodedQuery;
	}
	//! Returns percent-decoded query part of the URI
	/*!
	 * \note Do not parse this value for params extraction - use encodedQuery() instead!
	 *       This method is to be used for custom query part processing only.
	 */
	inline const std::string& query() const
	{
		return _query;
	}
	//! Composes URI to stream
	/*!
	 * \param target A reference to output stream to compose URI into
	 * \return Composed URI size
	 */
	size_t compose(std::ostream& target) const;
	//! Composes URI to buffer
	/*!
	 * TODO: Implementation
	 * \param buf Pointer to buffer to compose URI into
	 * \param len Buffer size
	 * \return Composed URI size
	 */
	size_t compose(void * buf, size_t len) const;

	//! Returns size of composed URI
	inline size_t composedSize() const
	{
		return _encodedPath.size() +
			(_encodedQuery.empty() ? 0U : 1U + _encodedQuery.size());
	}
private:
	std::string _encodedPath;
	std::string _path;
	std::string _encodedQuery;
	std::string _query;
};

} // namespace httpxx

#endif
