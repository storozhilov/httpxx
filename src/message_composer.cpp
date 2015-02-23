#include <httpxx/message_composer.h>
#include <sstream>
#include <stdexcept>

namespace httpxx
{

namespace {

static const char * ContentLengthHeader = "Content-Length";
static const char * TransferEncodingHeader = "Transfer-Encoding";

inline void composeFirstLine(std::ostream& target, const std::string& firstToken,
		const std::string& secondToken, const std::string& thirdToken)
{
	target << firstToken << ' ' << secondToken << ' ' << thirdToken << "\r\n";
}

inline void composeHeader(std::ostream& target, const Headers& headers)
{
	for (Headers::const_iterator i = headers.begin(); i != headers.end(); ++i) {
		target << i->first << ": " << i->second  << "\r\n";
	}
}

} // anonymous namespace

MessageComposer::MessageComposer(const std::string& firstToken, const std::string& secondToken,
		const std::string& thirdToken) :
	_firstToken(firstToken),
	_secondToken(secondToken),
	_thirdToken(thirdToken)
{}

MessageComposer::~MessageComposer()
{}

void MessageComposer::reset(const std::string& firstToken, const std::string& secondToken,
		const std::string& thirdToken)
{
	_firstToken = firstToken;
	_secondToken = secondToken;
	_thirdToken = thirdToken;
}

void MessageComposer::composeEnvelope(std::ostream& target, const Headers& headers, size_t payloadLen)
{
	Headers actualHeaders(headers);
	actualHeaders.erase(ContentLengthHeader);
	actualHeaders.erase(TransferEncodingHeader);
	if (payloadLen > 0U) {
		std::ostringstream oss;
		oss << payloadLen;
		actualHeaders.insert(Headers::value_type(ContentLengthHeader, oss.str()));
	}
	composeFirstLine(target, _firstToken, _secondToken, _thirdToken);
	composeHeader(target, actualHeaders);
	target << "\r\n";
}

size_t MessageComposer::composeEnvelope(void * buffer, size_t bufLen, const Headers& headers, size_t payloadLen)
{
	std::ostringstream envelope;
	composeEnvelope(envelope, headers, payloadLen);
	if (envelope.str().size() > bufLen) {
		std::ostringstream msg;
		msg << "Not enough buffer for envelope: " << bufLen <<
			" bytes available, " << envelope.str().size() << " bytes needed";
		throw std::runtime_error(msg.str());
	}
	envelope.str().copy(static_cast<char *>(buffer), envelope.str().size());
	return envelope.str().size();
}

MessageComposer::Packet MessageComposer::prependEnvelope(void * buffer, size_t envelopePartLen,
		const Headers& headers, size_t payloadLen)
{
	std::ostringstream envelope;
	composeEnvelope(envelope, headers, payloadLen);
	if (envelope.str().size() > envelopePartLen) {
		std::ostringstream msg;
		msg << "Not enough buffer for envelope: " << envelopePartLen <<
			" bytes available, " << envelope.str().size() << " bytes needed";
		throw std::runtime_error(msg.str());
	}
	char * packetPtr = static_cast<char *>(buffer) + envelopePartLen - envelope.str().size(); 
	envelope.str().copy(packetPtr, envelope.str().size());
	return Packet(packetPtr, envelope.str().size() + payloadLen);
}

void MessageComposer::composeFirstChunkEnvelope(std::ostream& target, 
		const Headers& headers, size_t payloadLen)
{
	if (payloadLen <= 0U) {
		throw std::runtime_error("Could not compose envelope for empty chunk");
	}
	Headers actualHeaders(headers);
	actualHeaders.erase(ContentLengthHeader);
	actualHeaders.erase(TransferEncodingHeader);
	actualHeaders.insert(Headers::value_type(TransferEncodingHeader, "chunked"));
	composeFirstLine(target, _firstToken, _secondToken, _thirdToken);
	composeHeader(target, actualHeaders);
	target << "\r\n" << std::hex << payloadLen << "\r\n";
}

size_t MessageComposer::composeFirstChunkEnvelope(void * buffer, size_t bufLen, const Headers& headers,
		size_t payloadLen)
{
	std::ostringstream envelope;
	composeFirstChunkEnvelope(envelope, headers, payloadLen);
	if (envelope.str().size() > bufLen) {
		std::ostringstream msg;
		msg << "Not enough buffer for envelope: " << bufLen <<
			" bytes available, " << envelope.str().size() << " bytes needed";
		throw std::runtime_error(msg.str());
	}
	envelope.str().copy(static_cast<char *>(buffer), envelope.str().size());
	return envelope.str().size();
}

MessageComposer::Packet MessageComposer::prependFirstChunkEnvelope(void * buffer,
		size_t envelopePartLen, const Headers& headers, size_t payloadLen)
{
	std::ostringstream envelope;
	composeFirstChunkEnvelope(envelope, headers, payloadLen);
	if (envelope.str().size() > envelopePartLen) {
		std::ostringstream msg;
		msg << "Not enough buffer for envelope: " << envelopePartLen <<
			" bytes available, " << envelope.str().size() << " bytes needed";
		throw std::runtime_error(msg.str());
	}
	char * packetPtr = static_cast<char *>(buffer) + envelopePartLen - envelope.str().size(); 
	envelope.str().copy(packetPtr, envelope.str().size());
	return Packet(packetPtr, envelope.str().size() + payloadLen);
}

void MessageComposer::composeNextChunkEnvelope(std::ostream& target, size_t payloadLen)
{
	if (payloadLen <= 0U) {
		throw std::runtime_error("Could not compose envelope for empty chunk");
	}
	target << "\r\n" << std::hex << payloadLen << "\r\n";
}

size_t MessageComposer::composeNextChunkEnvelope(void * buffer, size_t bufLen, size_t payloadLen)
{
	std::ostringstream envelope;
	composeNextChunkEnvelope(envelope, payloadLen);
	if (envelope.str().size() > bufLen) {
		std::ostringstream msg;
		msg << "Not enough buffer for envelope: " << bufLen <<
			" bytes available, " << envelope.str().size() << " bytes needed";
		throw std::runtime_error(msg.str());
	}
	envelope.str().copy(static_cast<char *>(buffer), envelope.str().size());
	return envelope.str().size();
}

MessageComposer::Packet MessageComposer::prependNextChunkEnvelope(void * buffer, size_t envelopePartLen, size_t payloadLen)
{
	std::ostringstream envelope;
	composeNextChunkEnvelope(envelope, payloadLen);
	if (envelope.str().size() > envelopePartLen) {
		std::ostringstream msg;
		msg << "Not enough buffer for envelope: " << envelopePartLen <<
			" bytes available, " << envelope.str().size() << " bytes needed";
		throw std::runtime_error(msg.str());
	}
	char * packetPtr = static_cast<char *>(buffer) + envelopePartLen - envelope.str().size(); 
	envelope.str().copy(packetPtr, envelope.str().size());
	return Packet(packetPtr, envelope.str().size() + payloadLen);
}

void MessageComposer::composeLastChunk(std::ostream& target, const Headers& headers)
{
	target << "\r\n0\r\n";
	composeHeader(target, headers);
	target << "\r\n";
}

size_t MessageComposer::composeLastChunk(char * buffer, size_t bufLen, const Headers& headers)
{
	std::ostringstream envelope;
	composeLastChunk(envelope, headers);
	if (envelope.str().size() > bufLen) {
		std::ostringstream msg;
		msg << "Not enough buffer for envelope: " << bufLen <<
			" bytes available, " << envelope.str().size() << " bytes needed";
		throw std::runtime_error(msg.str());
	}
	envelope.str().copy(buffer, envelope.str().size());
	return envelope.str().size();
}

} // namespace httpxx
