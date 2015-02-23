#include <gtest/gtest.h>
#include <memory>
#include <httpxx/message_composer.h>

#define BUFFER_SIZE 4096U
#define ENVELOPE_SIZE 1024U

using namespace httpxx;

static const char Payload[] = "This is some data to send";
static const size_t PayloadLen = sizeof(Payload) - 1;

class MessageComposerTest : public ::testing::Test
{
protected:
	virtual void SetUp()
	{
		composer.reset(new MessageComposer("GET", "/index.html", "HTTP/1.1"));
		headers.reset(new Headers());
		headers->insert(Headers::value_type("Host", "www.example.com"));
		headers->insert(Headers::value_type("Content-Type", "text/plain"));
		memcpy(buffer + ENVELOPE_SIZE, Payload, PayloadLen);
	}
	
	std::auto_ptr<MessageComposer> composer;
	std::auto_ptr<Headers> headers;
	char buffer[BUFFER_SIZE];
};

TEST_F(MessageComposerTest, ComposeEnvelopeToStream)
{
	std::ostringstream e;
	composer->composeEnvelope(e, *headers);
	EXPECT_EQ(
		"GET /index.html HTTP/1.1\r\n"
		"Content-Type: text/plain\r\n"
		"Host: www.example.com\r\n"
		"\r\n",
		e.str());

	e.str("");
	std::ostringstream re;
	re <<	"GET /index.html HTTP/1.1\r\n" <<
		"Content-Length: " << PayloadLen << "\r\n" <<
		"Content-Type: text/plain\r\n" <<
		"Host: www.example.com\r\n" <<
		"\r\n";
	composer->composeEnvelope(e, *headers, PayloadLen);
	EXPECT_EQ(re.str(), e.str());

	e.str("");
	headers->insert(Headers::value_type("content-length", "256"));
	headers->insert(Headers::value_type("transfer-encoding", "chunked"));
	composer->composeEnvelope(e, *headers, PayloadLen);
	EXPECT_EQ(re.str(), e.str());
}

TEST_F(MessageComposerTest, ComposeEnvelopeToBuffer)
{
	size_t envelopeSize = composer->composeEnvelope(buffer, BUFFER_SIZE, *headers);
	EXPECT_EQ(
		"GET /index.html HTTP/1.1\r\n"
		"Content-Type: text/plain\r\n"
		"Host: www.example.com\r\n"
		"\r\n",
		std::string(buffer, envelopeSize));

	EXPECT_THROW(composer->composeEnvelope(buffer, 1U, *headers), std::runtime_error);

	std::ostringstream re;
	re <<	"GET /index.html HTTP/1.1\r\n" <<
		"Content-Length: " << PayloadLen << "\r\n" <<
		"Content-Type: text/plain\r\n" <<
		"Host: www.example.com\r\n" <<
		"\r\n";
	envelopeSize = composer->composeEnvelope(buffer, BUFFER_SIZE, *headers, PayloadLen);
	EXPECT_EQ(re.str(), std::string(buffer, envelopeSize));

	EXPECT_THROW(composer->composeEnvelope(buffer, 1U, *headers, PayloadLen), std::runtime_error);
}

TEST_F(MessageComposerTest, PrependEnvelope)
{
	MessageComposer::Packet p = composer->prependEnvelope(buffer, ENVELOPE_SIZE, *headers);
	EXPECT_EQ(
		"GET /index.html HTTP/1.1\r\n"
		"Content-Type: text/plain\r\n"
		"Host: www.example.com\r\n"
		"\r\n",
		std::string(p.first, p.second));

	std::ostringstream re;
	re <<	"GET /index.html HTTP/1.1\r\n" <<
		"Content-Length: " << PayloadLen << "\r\n" <<
		"Content-Type: text/plain\r\n" <<
		"Host: www.example.com\r\n" <<
		"\r\n" <<
		Payload;
	p = composer->prependEnvelope(buffer, ENVELOPE_SIZE, *headers, PayloadLen);
	EXPECT_EQ(re.str(), std::string(p.first, p.second));

	headers->insert(Headers::value_type("content-length", "256"));
	headers->insert(Headers::value_type("transfer-encoding", "chunked"));
	p = composer->prependEnvelope(buffer, ENVELOPE_SIZE, *headers, PayloadLen);
	EXPECT_EQ(re.str(), std::string(p.first, p.second));

	EXPECT_THROW(composer->prependEnvelope(buffer, 1U, *headers), std::runtime_error);
	EXPECT_THROW(composer->prependEnvelope(buffer, 1U, *headers, PayloadLen), std::runtime_error);
}

TEST_F(MessageComposerTest, ComposeFirstChunkEnvelopeToStream)
{
	std::ostringstream re;
	re <<	"GET /index.html HTTP/1.1\r\n" <<
		"Content-Type: text/plain\r\n" <<
		"Host: www.example.com\r\n" <<
		"Transfer-Encoding: chunked\r\n" <<
		"\r\n" <<
		std::hex << PayloadLen << "\r\n";
	std::ostringstream e;
	composer->composeFirstChunkEnvelope(e, *headers, PayloadLen);
	EXPECT_EQ(re.str(), e.str());

	e.str("");
	headers->insert(Headers::value_type("content-length", "256"));
	headers->insert(Headers::value_type("transfer-encoding", "chunked"));
	composer->composeFirstChunkEnvelope(e, *headers, PayloadLen);
	EXPECT_EQ(re.str(), e.str());
}
	
TEST_F(MessageComposerTest, ComposeFirstChunkEnvelopeToBuffer)
{
	std::ostringstream re;
	re <<	"GET /index.html HTTP/1.1\r\n" <<
		"Content-Type: text/plain\r\n" <<
		"Host: www.example.com\r\n" <<
		"Transfer-Encoding: chunked\r\n" <<
		"\r\n" <<
		std::hex << PayloadLen << "\r\n";
	size_t envelopeSize = composer->composeFirstChunkEnvelope(buffer, BUFFER_SIZE, *headers, PayloadLen);
	EXPECT_EQ(re.str(), std::string(buffer, envelopeSize));

	headers->insert(Headers::value_type("content-length", "256"));
	headers->insert(Headers::value_type("transfer-encoding", "chunked"));
	envelopeSize = composer->composeFirstChunkEnvelope(buffer, BUFFER_SIZE, *headers, PayloadLen);
	EXPECT_EQ(re.str(), std::string(buffer, envelopeSize));

	EXPECT_THROW(composer->composeFirstChunkEnvelope(buffer, 1U, *headers, PayloadLen), std::runtime_error);
	EXPECT_THROW(composer->composeFirstChunkEnvelope(buffer, BUFFER_SIZE, *headers, 0U), std::runtime_error);
}
	
TEST_F(MessageComposerTest, PrependFirstChunkEnvelope)
{
	std::ostringstream re;
	re <<	"GET /index.html HTTP/1.1\r\n" <<
		"Content-Type: text/plain\r\n" <<
		"Host: www.example.com\r\n" <<
		"Transfer-Encoding: chunked\r\n" <<
		"\r\n" <<
		std::hex << PayloadLen << "\r\n" <<
		Payload;
	MessageComposer::Packet p = composer->prependFirstChunkEnvelope(buffer, ENVELOPE_SIZE, *headers, PayloadLen);
	EXPECT_EQ(re.str(), std::string(p.first, p.second));

	headers->insert(Headers::value_type("content-length", "256"));
	headers->insert(Headers::value_type("transfer-encoding", "chunked"));
	p = composer->prependFirstChunkEnvelope(buffer, ENVELOPE_SIZE, *headers, PayloadLen);
	EXPECT_EQ(re.str(), std::string(p.first, p.second));

	EXPECT_THROW(composer->prependFirstChunkEnvelope(buffer, ENVELOPE_SIZE, *headers, 0U), std::runtime_error);
	EXPECT_THROW(composer->prependFirstChunkEnvelope(buffer, 1U, *headers, PayloadLen), std::runtime_error);
}

TEST_F(MessageComposerTest, ComposeNextChunkEnvelopeToStream)
{
	std::ostringstream re;
	re <<	"\r\n" <<
		std::hex << PayloadLen << "\r\n";
	std::ostringstream e;
	composer->composeNextChunkEnvelope(e, PayloadLen);
	EXPECT_EQ(re.str(), e.str());
}

TEST_F(MessageComposerTest, ComposeNextChunkEnvelopeToBuffer)
{
	std::ostringstream re;
	re <<	"\r\n" <<
		std::hex << PayloadLen << "\r\n";
	size_t envelopeSize = composer->composeNextChunkEnvelope(buffer, BUFFER_SIZE, PayloadLen);
	EXPECT_EQ(re.str(), std::string(buffer, envelopeSize));

	EXPECT_THROW(composer->composeNextChunkEnvelope(buffer, 1U, PayloadLen), std::runtime_error);
	EXPECT_THROW(composer->composeNextChunkEnvelope(buffer, BUFFER_SIZE, 0U), std::runtime_error);
}

TEST_F(MessageComposerTest, PrependNextChunkEnvelope)
{
	std::ostringstream re;
	re <<	"\r\n" <<
		std::hex << PayloadLen << "\r\n" <<
		Payload;
	MessageComposer::Packet p = composer->prependNextChunkEnvelope(buffer, ENVELOPE_SIZE, PayloadLen);
	EXPECT_EQ(re.str(), std::string(p.first, p.second));

	headers->insert(Headers::value_type("content-length", "256"));
	headers->insert(Headers::value_type("transfer-encoding", "chunked"));
	p = composer->prependNextChunkEnvelope(buffer, ENVELOPE_SIZE, PayloadLen);
	EXPECT_EQ(re.str(), std::string(p.first, p.second));

	EXPECT_THROW(composer->prependNextChunkEnvelope(buffer, 1U, PayloadLen), std::runtime_error);
	EXPECT_THROW(composer->prependNextChunkEnvelope(buffer, BUFFER_SIZE, 0U), std::runtime_error);
}
	
TEST_F(MessageComposerTest, ComposeLastChunkToStream)
{
	std::ostringstream re;
	re <<	"\r\n" <<
		"0\r\n" <<
		"Content-Type: text/plain\r\n" <<
		"Host: www.example.com\r\n" <<
		"\r\n";
	std::ostringstream e;
	composer->composeLastChunk(e, *headers);
	EXPECT_EQ(re.str(), e.str());
}

TEST_F(MessageComposerTest, ComposeLastChunkToBuffer)
{
	std::ostringstream re;
	re <<	"\r\n" <<
		"0\r\n" <<
		"Content-Type: text/plain\r\n" <<
		"Host: www.example.com\r\n" <<
		"\r\n";
	size_t lastChunkSize = composer->composeLastChunk(buffer, BUFFER_SIZE, *headers);
	EXPECT_EQ(re.str(), std::string(buffer, lastChunkSize));

	EXPECT_THROW(composer->composeLastChunk(buffer, 1U, *headers), std::runtime_error);
}
