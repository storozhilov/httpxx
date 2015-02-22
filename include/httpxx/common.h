#ifndef HTTPXX_COMMON_H
#define HTTPXX_COMMON_H

#include <string>
#include <functional>

namespace httpxx
{

//! Case-insensitive string comparator
struct CaseInsensitiveComparator : public std::binary_function<std::string, std::string, bool>
{
	bool operator()(const std::string &lhs, const std::string &rhs) const;
};

} // namespace httpxx

#endif
