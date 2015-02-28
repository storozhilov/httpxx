#ifndef HTTPXX_MESSAGE_COMPOSER_H
#define HTTPXX_MESSAGE_COMPOSER_H

#include <httpxx/headers.h>
#include <ostream>

namespace httpxx
{

//! HTTP-message composer
/*!
 * Generally speaking, HTTP adds an envelope before payload. This is true either
 * for identity-encoded transmission (<i>"Content-Length: <N>" header is used</i>) or
 * in case of chunked encoding one (<i>"Transfer-Encoding: chunked" header presents</i>).
 * So the main idea is to generate an envelope for given headers and payload,
 * then to send an envelope and payload to peer one by one. If you want to prepend
 * payload with envelope to send it to peer as single piece of bytes use respective
 * family of composition methods: prependEnvelope(), prependFirstChunkEnvelope(),
 * prependNextChunkEnvelope().
 * 
 * Indentity-encoded transmission envelope could be composed using composeEnvelope()
 * methods family. Chunked-encoded transmission should be implemented similar to
 * following scenario (<i>Read lines from standard input and send them to peer as
 * HTTP-chunks</i>):
 *
 * \code{.cpp}
 * ...
 *
 * httpxx::MessageComposer composer("HTTP/1.1", "200", "OK");
 * httpxx::Headers headers;
 * headers.add("Content-Type", "text/plain");
 * bool isFirstChunk = true;
 * while (std::cin) {
 *     std::string s;
 *     std::cin >> s;
 *     std::ostringstream envelope;
 *     if (isFirstChunk) {
 *         composer.composeFirstChunkEnvelope(envelope, headers, s.length());
 *         isFirstChunk = false;
 *     } else {
 *         composer.composeNextChunkEnvelope(envelope, s.length());
 *     }
 *     send(sock, envelope.str().data(), envelope.str().length(), 0);
 *     send(sock, s.data(), s.length(), 0);
 * }
 * headers.clear();
 * // You can send additional headers in last chunk
 * headers.add("X-Good", "Bye! :)");
 * std::ostringstream lastChunk;
 * composer.composeLastChunk(lastChunk, headers);
 * send(sock, lastChunk.str().data(), lastChunk.str().length(), 0);
 *
 * ...
 * \endcode
 * 
 * \note <i>"Content-Length"</i> and <i>"Transfer-Encoding"</i> headers are
 *       automatically generated on envelope composition. Such headers, which
 *       are already exists are removed first and regenerated afterwards according
 *       to requested transmission encoding type.
 */
class MessageComposer
{
public:
	//! HTTP-packet: { buffer ptr => size }
	typedef std::pair<const void *, size_t> Packet;

	MessageComposer(const std::string& firstToken, const std::string& secondToken,
			const std::string& thirdToken);
	virtual ~MessageComposer();

	//! Resets HTTP-message composer
	void reset(const std::string& firstToken, const std::string& secondToken,
			const std::string& thirdToken);
	//! Composes envelope into output stream for identity-encoded transmission
	/*!
	 * \param target Output stream to compose envelope into
	 * \param headers Reference to headers to use
	 * \param payloadLen Length of the payload data in HTTP-message
	 */
	void composeEnvelope(std::ostream& target, const Headers& headers, size_t payloadLen = 0U);
	//! Composes envelope into buffer for identity-encoded transmission
	/*!
	 * \param buffer Pointer to result buffer to compose envelope into
	 * \param bufLen Length of the result buffer
	 * \param headers Reference to headers to use
	 * \param payloadLen Length of the payload data in HTTP-message
	 * \return Length of the envelope
	 */
	size_t composeEnvelope(void * buffer, size_t bufLen, const Headers& headers, size_t payloadLen = 0U);
	//! Returns size of identity-encoded transmission envelope (TODO)
	/*!
	 * TODO
	 */
	size_t envelopeSize(const Headers& headers, size_t payloadLen = 0U);
	//! Prepends data with envelope to compose an HTTP-message for identity-encoded transmission
	/*!
	 * \param buffer Pointer to result buffer to compose envelope into
	 * \param envelopePartLen Length of the envelope part in result buffer
	 * \param headers Reference to headers to use
	 * \param payloadLen Length of the data in buffer to send (should start from 'envelopePartLen' offset)
	 * \return Pointer to HTTP-packet and it's length (envelope + data)
	 */
	Packet prependEnvelope(void * buffer, size_t envelopePartLen, const Headers& headers,
			size_t payloadLen = 0U);
	//! Composes first chunk envelope into output stream for chunked-encoded transmission
	/*!
	 * \param target Output stream to compose envelope into
	 * \param headers Reference to headers to use
	 * \param payloadLen Length of the payload data in HTTP-chunk (should be positive)
	 */
	void composeFirstChunkEnvelope(std::ostream& target, const Headers& headers, size_t payloadLen);
	//! Composes first chunk envelope into buffer for chunked-encoded transmission
	/*!
	 * \param buffer Pointer to result buffer to compose envelope into
	 * \param bufLen Length of the result buffer
	 * \param headers Reference to headers to use
	 * \param payloadLen Length of the payload data in HTTP-chunk (should be positive)
	 * \return Length of the first HTTP-chunk envelope
	 */
	size_t composeFirstChunkEnvelope(void * buffer, size_t bufLen, const Headers& headers, size_t payloadLen);
	//! Returns size of first HTTP-chunk envelope (TODO)
	/*!
	 * TODO
	 */
	size_t firstChunkEnvelopeSize(const Headers& headers, size_t payloadLen = 0U);
	//! Prepends data with envelope to compose a first HTTP-chunk to start chunked-encoded transmission
	/*!
	 * \param buffer Pointer to result buffer to compose envelope into
	 * \param envelopePartLen Length of the envelope part in result buffer
	 * \param headers Reference to headers to use
	 * \param payloadLen Length of the data in buffer to send (should start from 'envelopePartLen' offset)
	 * \return Pointer to the first HTTP-chunk and it's length (envelope + data)
	 */
	Packet prependFirstChunkEnvelope(void * buffer, size_t envelopePartLen, const Headers& headers,
			size_t payloadLen);
	//! Composes next chunk envelope into output stream for chunked-encoded transmission
	/*!
	 * \param target Output stream to compose envelope into
	 * \param payloadLen Length of the payload data in next HTTP-chunk (should be positive)
	 */
	void composeNextChunkEnvelope(std::ostream& target, size_t payloadLen);
	//! Composes next chunk envelope into buffer for chunked-encoded transmission
	/*!
	 * \param buffer Pointer to result buffer to compose envelope into
	 * \param bufLen Length of the result buffer
	 * \param payloadLen Length of the payload data in next HTTP-chunk (should be positive)
	 * \return Length of the next HTTP-chunk envelope
	 */
	size_t composeNextChunkEnvelope(void * buffer, size_t bufLen, size_t payloadLen);
	//! Returns size of next HTTP-chunk envelope (TODO)
	/*!
	 * TODO
	 */
	size_t nextChunkEnvelopeSize(size_t payloadLen);
	//! Prepends data with envelope to compose next HTTP-chunk to continue chunked-encoded transmission
	/*!
	 * \param buffer Pointer to result buffer to compose envelope into
	 * \param envelopePartLen Length of the envelope part in result buffer
	 * \param payloadLen Length of the data in buffer to send (should start from 'envelopePartLen' offset)
	 * \return Pointer to the next HTTP-chunk and it's length (envelope + data)
	 */
	Packet prependNextChunkEnvelope(void * buffer, size_t envelopePartLen, size_t payloadLen);
	//! Composes last HTTP-chunk into output stream for chunked-encoded transmission
	/*!
	 * \param target Output stream to compose envelope into
	 * \param headers Reference to headers to use
	 */
	void composeLastChunk(std::ostream& target, const Headers& headers = Headers());
	//! Composes last HTTP-chunk into buffer to complete chunked-encoded transmission
	/*!
	 * \param buffer Pointer to result buffer to compose envelope into
	 * \param bufLen Length of the result buffer
	 * \param headers Reference to headers to use
	 * \return Length of the last HTTP-chunk
	 */
	size_t composeLastChunk(char * buffer, size_t bufLen, const Headers& headers = Headers());
	//! Returns size of last HTTP-chunk (TODO)
	/*!
	 * TODO
	 */
	size_t lastChunkSize(const Headers& headers = Headers());
private:
	std::string _firstToken;
	std::string _secondToken;
	std::string _thirdToken;
};

} // namespace httpxx

#endif

