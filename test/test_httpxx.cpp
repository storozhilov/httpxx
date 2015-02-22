#include <gtest/gtest.h>
#include <httpxx.h>

using namespace httpxx;

TEST(Uri, parseUri)
{
	static const char * LatinUri = "/index.html";
	static const char * LatinUriWithParams = "/index.html?foo=bar&bar=foo";
	static const char * InternationalUri = "/%D1%80%D0%B5%D1%81%D1%83%D1%80%D1%81.html";
	static const char * InternationalUriWithParam =
		"/%D1%80%D0%B5%D1%81%D1%83%D1%80%D1%81.html?%D0%BF%D0%B0%D1%80%D0%B0%D0%BC=%D0%B7%D0%BD%D0%B0%D1%87";
	std::pair<std::string, std::string> parsedUri = parseUri(LatinUri);
	EXPECT_EQ("/index.html", parsedUri.first);
	EXPECT_EQ(std::string(), parsedUri.second);
	parsedUri = parseUri(LatinUriWithParams);
	EXPECT_EQ("/index.html", parsedUri.first);
	EXPECT_EQ("foo=bar&bar=foo", parsedUri.second);
	parsedUri = parseUri(InternationalUri);
	EXPECT_EQ("/ресурс.html", parsedUri.first);
	EXPECT_EQ(std::string(), parsedUri.second);
	parsedUri = parseUri(InternationalUriWithParam);
	EXPECT_EQ("/ресурс.html", parsedUri.first);
	EXPECT_EQ("%D0%BF%D0%B0%D1%80%D0%B0%D0%BC=%D0%B7%D0%BD%D0%B0%D1%87", parsedUri.second);
}
