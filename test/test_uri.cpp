#include <gtest/gtest.h>
#include <httpxx/uri.h>

using namespace httpxx;

TEST(Uri, ParseAndCompose)
{
	static const char * LatinUri = "/index.html";
	static const char * LatinUriWithParams = "/index.html?foo=bar&bar=foo";
	static const char * InternationalUri = "/%D1%80%D0%B5%D1%81%D1%83%D1%80%D1%81.html";
	static const char * InternationalUriWithParams =
		"/%D1%80%D0%B5%D1%81%D1%83%D1%80%D1%81.html?%D0%BF%D0%B0%D1%80%D0%B0%D0%BC=%D0%B7%D0%BD%D0%B0%D1%87"
		"&%D0%B7%D0%BD%D0%B0%D1%87=%D0%BF%D0%B0%D1%80%D0%B0%D0%BC";

	httpxx::Uri uri(LatinUri);
	EXPECT_EQ("/index.html", uri.encodedPath());
	EXPECT_EQ("/index.html", uri.path());
	EXPECT_EQ(std::string(), uri.query());
	std::ostringstream oss;
	uri.compose(oss);
	EXPECT_EQ(std::string(LatinUri), oss.str());
	EXPECT_EQ(uri.composedSize(), oss.str().size());

	uri = Uri(LatinUriWithParams);
	EXPECT_EQ("/index.html", uri.encodedPath());
	EXPECT_EQ("/index.html", uri.path());
	EXPECT_EQ("foo=bar&bar=foo", uri.query());
	oss.str("");
	uri.compose(oss);
	EXPECT_EQ(std::string(LatinUriWithParams), oss.str());
	EXPECT_EQ(uri.composedSize(), oss.str().size());

	uri = Uri(InternationalUri);
	EXPECT_EQ("/%D1%80%D0%B5%D1%81%D1%83%D1%80%D1%81.html", uri.encodedPath());
	EXPECT_EQ("/ресурс.html", uri.path());
	EXPECT_EQ(std::string(), uri.query());
	oss.str("");
	uri.compose(oss);
	EXPECT_EQ(std::string(InternationalUri), oss.str());
	EXPECT_EQ(uri.composedSize(), oss.str().size());

	uri = Uri(InternationalUriWithParams);
	EXPECT_EQ("/%D1%80%D0%B5%D1%81%D1%83%D1%80%D1%81.html", uri.encodedPath());
	EXPECT_EQ("/ресурс.html", uri.path());
	EXPECT_EQ("%D0%BF%D0%B0%D1%80%D0%B0%D0%BC=%D0%B7%D0%BD%D0%B0%D1%87"
			"&%D0%B7%D0%BD%D0%B0%D1%87=%D0%BF%D0%B0%D1%80%D0%B0%D0%BC", uri.query());
	oss.str("");
	uri.compose(oss);
	EXPECT_EQ(std::string(InternationalUriWithParams), oss.str());
	EXPECT_EQ(uri.composedSize(), oss.str().size());
}
