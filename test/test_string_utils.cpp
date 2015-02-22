#include <gtest/gtest.h>
#include <string_utils.h>

using namespace httpxx;

TEST(StringUtils, encodeDecodePercent)
{
	static const char * SourceString = "To: Василий Пупкин <vasily.pupkin@example.com>";
	static const char * EncodedString =
		"To%3A+%D0%92%D0%B0%D1%81%D0%B8%D0%BB%D0%B8%D0%B9+%D0%9F%D1%83"
		"%D0%BF%D0%BA%D0%B8%D0%BD+%3Cvasily%2Epupkin%40example%2Ecom%3E";
	EXPECT_EQ(EncodedString, encodePercent(SourceString));
	EXPECT_EQ(SourceString, decodePercent(encodePercent(SourceString)));
}
