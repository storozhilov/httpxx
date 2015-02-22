#include <gtest/gtest.h>
#include <memory>
#include <httpxx/message_parser.h>

using namespace httpxx;

// TODO: Test all exceptions!!!

class MessageParserTest : public ::testing::Test
{
protected:
	virtual void SetUp()
	{
		parser.reset(new MessageParser(10U, 24U, 24U));
	}
	
	std::auto_ptr<MessageParser> parser;
};

TEST_F(MessageParserTest, ParseBodylessMessage)
{
	static const char * BodylessMessage =
		" GET /index.html  HTTP/1.1\r\n"
		"Host: localhost\r\n"
		"X-Foo: bar\r\n"
		"\r\n";

	for (size_t i = 0U; i < strlen(BodylessMessage); ++i) {
		bool isBodyByte = false;
		bool r = parser->parse(BodylessMessage[i], &isBodyByte); 
		if (i < (strlen(BodylessMessage) - 1)) {
			EXPECT_FALSE(r);
		} else {
			EXPECT_TRUE(r);
		}
		EXPECT_FALSE(isBodyByte);
	}
	EXPECT_EQ("GET", parser->firstToken());
	EXPECT_EQ("/index.html", parser->secondToken());
	EXPECT_EQ("HTTP/1.1", parser->thirdToken());
	EXPECT_EQ(2U, parser->headers().size());
	EXPECT_TRUE(parser->headers().have("host", "localhost"));
	EXPECT_TRUE(parser->headers().have("x-foo", "bar"));
	/*EXPECT_FALSE(parser->headers().have("host", "Localhost"));
	EXPECT_FALSE(parser->headers().have("host", "microsoft.com"));
	EXPECT_FALSE(parser->headers().have("x-host"));
	EXPECT_FALSE(parser->headers().have("x-host", "microsoft.com"));
	EXPECT_TRUE(parser->headers().have("x-foo"));
	EXPECT_TRUE(parser->headers().have("x-foo", "bar"));
	EXPECT_FALSE(parser->headers().have("x-foo", "x-bar"));*/
}

TEST_F(MessageParserTest, ParseIdentityEncodedMessage)
{
	static const char * IdentityEncodedMessage =
		"\tHTTP/1.1\t200\t\tOK\r\n"
		"Connection: close\r\n"
		"x-bar: foo\r\n"
		"Content-Length: 10\r\n"
		"\r\n"
		"1234567890";

	std::ostringstream body;
	for (size_t i = 0U; i < strlen(IdentityEncodedMessage); ++i) {
		bool isBodyByte = false;
		bool r = parser->parse(IdentityEncodedMessage[i], &isBodyByte);
		if (i < (strlen(IdentityEncodedMessage) - 1)) {
			EXPECT_FALSE(r);
		} else {
			EXPECT_TRUE(r);
		}
		if (isBodyByte) {
			body << IdentityEncodedMessage[i];
		}
	}
	EXPECT_EQ("HTTP/1.1", parser->firstToken());
	EXPECT_EQ("200", parser->secondToken());
	EXPECT_EQ("OK", parser->thirdToken());
	EXPECT_EQ(3U, parser->headers().size());
	EXPECT_TRUE(parser->headers().have("Connection", "close"));
	EXPECT_TRUE(parser->headers().have("x-bar", "foo"));
	EXPECT_TRUE(parser->headers().have("Content-Length", "10"));
	EXPECT_EQ("1234567890", body.str());
}

TEST_F(MessageParserTest, ParseChunkedEncodedMessage)
{
	static const char * ChunkedEncodedMessage =
		"HTTP/1.1 404 Not found\r\n"
		"Connection: close\r\n"
		"x-header: foobar\r\n"
		"x-multiline: multiline\r\n"
		"\tLWS value\r\n"
		"Transfer-Encoding:\r\n"
		" chunked\r\n"
		"\r\n"
		"a\r\n"
		"1234567890\r\n"
		"b\r\n"
		"12345678901\r\n"
		"0\r\n"
		"X-Trailer: barfoo\r\n"
		"\r\n";

	std::ostringstream body;
	for (size_t i = 0U; i < strlen(ChunkedEncodedMessage); ++i) {
		bool isBodyByte = false;
		bool r = parser->parse(ChunkedEncodedMessage[i], &isBodyByte); 
		if (i < (strlen(ChunkedEncodedMessage) - 1)) {
			EXPECT_FALSE(r);
		} else {
			EXPECT_TRUE(r);
		}
		if (isBodyByte) {
			body << ChunkedEncodedMessage[i];
		}
	}
	EXPECT_EQ("HTTP/1.1", parser->firstToken());
	EXPECT_EQ("404", parser->secondToken());
	EXPECT_EQ("Not found", parser->thirdToken());
	EXPECT_EQ(5U, parser->headers().size());
	EXPECT_TRUE(parser->headers().have("Connection", "close"));
	EXPECT_TRUE(parser->headers().have("X-Header", "foobar"));
	EXPECT_TRUE(parser->headers().have("x-multiline", "multiline LWS value"));
	EXPECT_TRUE(parser->headers().have("x-trailer", "barfoo"));
	EXPECT_TRUE(parser->headers().have("Transfer-Encoding", "chunked"));
	EXPECT_EQ("123456789012345678901", body.str());
}

static const char * MultiMessage =
	" GET /index.html  HTTP/1.1\r\n"
	"Host: localhost\r\n"
	"X-Foo: bar\r\n"
	"\r\n"
	"\t \t\t  HTTP/1.1\t200\t\tOK\r\n"
	"Connection: close\r\n"
	"x-bar: foo\r\n"
	"Content-Length: 10\r\n"
	"\r\n"
	"1234567890"
	"HTTP/1.1 404 Not found\r\n"
	"Connection: close\r\n"
	"x-header: foobar\r\n"
	"Transfer-Encoding: chunked\r\n"
	"\r\n"
	"a\r\n"
	"1234567890\r\n"
	"b\r\n"
	"12345678901\r\n"
	"0\r\n"
	"X-Trailer: barfoo\r\n"
	"\r\n";

TEST_F(MessageParserTest, ParseMultiple)
{
	size_t messagesParsed = 0U;
	std::ostringstream body;
	for (size_t i = 0U; i < strlen(MultiMessage); ++i) {
		bool isBodyByte = false;
		bool r = parser->parse(MultiMessage[i], &isBodyByte); 
		if (isBodyByte) {
			body << MultiMessage[i];
		}
		if (r) {
			++messagesParsed;
			if (messagesParsed == 1) {
				EXPECT_EQ("GET", parser->firstToken());
				EXPECT_EQ("/index.html", parser->secondToken());
				EXPECT_EQ("HTTP/1.1", parser->thirdToken());
				EXPECT_EQ(2U, parser->headers().size());
				EXPECT_TRUE(parser->headers().have("host", "localhost"));
				EXPECT_TRUE(parser->headers().have("x-foo", "bar"));
			} else if (messagesParsed == 2) {
				EXPECT_EQ("HTTP/1.1", parser->firstToken());
				EXPECT_EQ("200", parser->secondToken());
				EXPECT_EQ("OK", parser->thirdToken());
				EXPECT_EQ(3U, parser->headers().size());
				EXPECT_TRUE(parser->headers().have("Connection", "close"));
				EXPECT_TRUE(parser->headers().have("x-bar", "foo"));
				EXPECT_TRUE(parser->headers().have("Content-Length", "10"));
				EXPECT_EQ("1234567890", body.str());
			} else if (messagesParsed == 3) {
				EXPECT_EQ("HTTP/1.1", parser->firstToken());
				EXPECT_EQ("404", parser->secondToken());
				EXPECT_EQ("Not found", parser->thirdToken());
				EXPECT_EQ(4U, parser->headers().size());
				EXPECT_TRUE(parser->headers().have("Connection", "close"));
				EXPECT_TRUE(parser->headers().have("X-Header", "foobar"));
				EXPECT_TRUE(parser->headers().have("x-trailer", "barfoo"));
				EXPECT_TRUE(parser->headers().have("Transfer-Encoding", "chunked"));
				EXPECT_EQ("123456789012345678901", body.str());
			}
			body.str("");
		}
	}
	EXPECT_EQ(3U, messagesParsed);
}

TEST_F(MessageParserTest, ParseMultipleToStream)
{
	size_t messagesParsed = 0U;
	size_t offset = 0U;
	std::ostringstream body;
	while (offset < strlen(MultiMessage)) {
		std::pair<bool, size_t> r = parser->parse(MultiMessage + offset,
				strlen(MultiMessage) - offset, body);
		if (r.first) {
			++messagesParsed;
			if (messagesParsed == 1) {
				EXPECT_EQ("GET", parser->firstToken());
				EXPECT_EQ("/index.html", parser->secondToken());
				EXPECT_EQ("HTTP/1.1", parser->thirdToken());
				EXPECT_EQ(2U, parser->headers().size());
				EXPECT_TRUE(parser->headers().have("host", "localhost"));
				EXPECT_TRUE(parser->headers().have("x-foo", "bar"));
			} else if (messagesParsed == 2) {
				EXPECT_EQ("HTTP/1.1", parser->firstToken());
				EXPECT_EQ("200", parser->secondToken());
				EXPECT_EQ("OK", parser->thirdToken());
				EXPECT_EQ(3U, parser->headers().size());
				EXPECT_TRUE(parser->headers().have("Connection", "close"));
				EXPECT_TRUE(parser->headers().have("x-bar", "foo"));
				EXPECT_TRUE(parser->headers().have("Content-Length", "10"));
				EXPECT_EQ("1234567890", body.str());
			} else if (messagesParsed == 3) {
				EXPECT_EQ("HTTP/1.1", parser->firstToken());
				EXPECT_EQ("404", parser->secondToken());
				EXPECT_EQ("Not found", parser->thirdToken());
				EXPECT_EQ(4U, parser->headers().size());
				EXPECT_TRUE(parser->headers().have("Connection", "close"));
				EXPECT_TRUE(parser->headers().have("X-Header", "foobar"));
				EXPECT_TRUE(parser->headers().have("x-trailer", "barfoo"));
				EXPECT_TRUE(parser->headers().have("Transfer-Encoding", "chunked"));
				EXPECT_EQ("123456789012345678901", body.str());
			}
			body.str("");
		}
		offset += r.second;
	}
	EXPECT_EQ(3U, messagesParsed);
}
