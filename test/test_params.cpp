#include <gtest/gtest.h>
#include <httpxx/params.h>

using namespace httpxx;

static const char * InternationalParams =
	"%D0%B7%3D%D0%BD%2B%D0%B0%26%D1%87=%D0%BF%3D%D0%B0%2B%D1%80%26%D0%B0%D0%BC"
	"&%D0%B7%D0%BD%D0%B0%D1%87=%D0%BF%D0%B0%D1%80%D0%B0%D0%BC"
	"&%D0%BF%D0%B0%D1%80%D0%B0%D0%BC=%D0%B7%D0%BD%D0%B0%D1%87";

TEST(Params, ParseAndCompose)
{
	static const char * LatinParams = "PI=3%2E1415&come=back&foo=bar&name=value";

	Params params(LatinParams);
	EXPECT_EQ(4U, params.size());
	EXPECT_EQ("value", params.value("name"));
	EXPECT_EQ("bar", params.value("foo"));
	EXPECT_EQ("back", params.value("come"));
	EXPECT_EQ("3.1415", params.value("PI"));
	std::ostringstream oss;
	params.compose(oss);
	EXPECT_EQ(std::string(LatinParams), oss.str());
	EXPECT_EQ(oss.str().size(), params.composedSize());

	params = Params(InternationalParams);
	EXPECT_EQ(3U, params.size());
	EXPECT_EQ("знач", params.value("парам"));
	EXPECT_EQ("парам", params.value("знач"));
	EXPECT_EQ("п=а+р&ам", params.value("з=н+а&ч"));
	oss.str("");
	params.compose(oss);
	EXPECT_EQ(std::string(InternationalParams), oss.str());
	EXPECT_EQ(oss.str().size(), params.composedSize());
}

TEST(Params, CreateComposeAndParse)
{
	Params params;
	params.add("парам", "знач");
	params.add("знач", "парам");
	params.add("з=н+а&ч", "п=а+р&ам");
	std::ostringstream oss;
	params.compose(oss);
	EXPECT_EQ(std::string(InternationalParams), oss.str());
	EXPECT_EQ(oss.str().size(), params.composedSize());

	params = Params(oss.str());
	EXPECT_EQ(3U, params.size());
	EXPECT_EQ("знач", params.value("парам"));
	EXPECT_EQ("парам", params.value("знач"));
	EXPECT_EQ("п=а+р&ам", params.value("з=н+а&ч"));
}
