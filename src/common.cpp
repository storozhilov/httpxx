#include <httpxx/common.h>
#include <cstring>

namespace httpxx
{

bool CaseInsensitiveComparator::operator()(const std::string &lhs, const std::string &rhs) const
{
	return strcasecmp(lhs.c_str(), rhs.c_str()) < 0;
}

} // namespace httpxx
