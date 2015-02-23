#ifndef HTTPXX_MESSAGE_PARSER_H
#define HTTPXX_MESSAGE_PARSER_H

#include <string>
#include <map>
#include <functional>
#include <httpxx/headers.h>

#ifndef HTTPXX_DEFAULT_MAX_HEADER_NAME_LENGTH
#define HTTPXX_DEFAULT_MAX_HEADER_NAME_LENGTH 256
#endif
#ifndef HTTPXX_DEFAULT_MAX_HEADER_VALUE_LENGTH
#define HTTPXX_DEFAULT_MAX_HEADER_VALUE_LENGTH 4096	// 4 Kb
#endif
#ifndef HTTPXX_DEFAULT_MAX_HEADERS_AMOUNT
#define HTTPXX_DEFAULT_MAX_HEADERS_AMOUNT 256
#endif

namespace httpxx
{

//! HTTP-message parser
/*!
  Parser does not apply strict rules on the first three tokens: first and second ones could consist of
  CHAR's which are not CTL/SP/HT's, third one is to be of CHAR's, which are not CTL's (see RFC-2616).
*/
class MessageParser
{
public:
	//! Class constants
	enum Constants {
		DefaultMaxHeaderNameLength = HTTPXX_DEFAULT_MAX_HEADER_NAME_LENGTH,
		DefaultMaxHeaderValueLength = HTTPXX_DEFAULT_MAX_HEADER_VALUE_LENGTH,
		DefaultMaxHeadersAmount = HTTPXX_DEFAULT_MAX_HEADERS_AMOUNT
	};
	//! Parser states
	enum State {
		ParsingMessage,					//!< Initial state
		ParsingLeadingSP,				//!< Parsing leading space
		ParsingFirstToken,				//!< Parsing first token
		ParsingFirstTokenSP,				//!< Parsing the delimeter b/w first and second token
		ParsingSecondToken,				//!< Parsing second token
		ParsingSecondTokenSP,				//!< Parsing the delimeter b/w second and third token
		ParsingThirdToken,				//!< Parsing third token
		ParsingFirstLineLF,				//!< First line LF has been found
		ParsingHeader,					//!< Parsing the beginning of the message header
		ParsingHeaderName,				//!< Parsing message header name
		ParsingHeaderValue,				//!< Parsing message header value
		ParsingHeaderValueLF,				//!< Message header line LF has been found
		ParsingHeaderValueLWS,				//!< Parsing message header multiline value LWS
		ParsingEndOfHeader,				//!< Parsing the end of the message header section
		ParsingIdentityBody,				//!< Parsing indentity-encoded message body
		ParsingChunkSize,				//!< Parsing the chunk size of the chunked-encoded message body
		ParsingChunkSizeLF,				//!< Chunk size line LF of of the chunked-encoded message body has been found
		ParsingChunkExtension,				//!< Parsing the chunk extension of the of the chunked-encoded message body
		ParsingChunk,					//!< Parsing the chunk of the chunked-encoded message body
		ParsingChunkCR,					//!< Chunk's CR has been found
		ParsingChunkLF,					//!< Chunk's LF has been found
		ParsingTrailerHeader,				//!< Parsing the beginning of the message trailer header
		ParsingTrailerHeaderName,			//!< Parsing message trailer header name
		ParsingTrailerHeaderValue,			//!< Parsing message trailer header value
		ParsingTrailerHeaderValueLF,			//!< Message trailer header line LF has been found
		ParsingTrailerHeaderValueLWS,			//!< Parsing message trailer header multiline value LWS
		ParsingFinalLF,					//!< Parsing final LF of the message
	};
	//! HTTP-message parser exception class
	class Exception : public std::exception
	{
	public:
		//! HTTP-message parser error codes
		enum Code {
			InvalidFirstToken,
			FirstTokenIsTooLong,
			InvalidSecondToken,
			SecondTokenIsTooLong,
			InvalidThirdToken,
			ThirdTokenIsTooLong,
			InvalidFirstLineLF,
			TooManyHeaders,
			EmptyHeaderName,
			InvalidHeaderName,
			HeaderNameIsTooLong,
			HeaderIsMissingColon,
			InvalidHeaderValue,
			HeaderValueIsTooLong,
			InvalidHeaderLF,
			InvalidContentLength,
			EmptyChunkSize,
			InvalidChunkSize,
			InvalidChunkSizeLF,
			InvalidChunkDataCR,
			InvalidChunkDataLF,
			InvalidFinalLF,
			InvalidState,
		};
		//! Constructs an HTTP-message parser exception
		/*!
		  \param ch Character, which caused an error
		  \param pos Position of the error in the HTTP-message (starts from 0)
		  \param line Line of the error in the HTTP-message (starts from 1)
		  \param col Column of the error in the HTTP-message (starts from 1)
		  \param code Error code
		*/
		Exception(char ch, int pos, int line, int col, Code code);
		virtual ~Exception() throw ()
		{}
		//! Returns a character, which caused an error
		inline char ch() const
		{
			return _ch;
		}
		//! Returns a position of the error in the HTTP-message
		inline int pos() const
		{
			return _pos;
		}
		//! Returns a line of the error in the HTTP-message
		inline int line() const
		{
			return _line;
		}
		//! Returns a column of the error in the HTTP-message
		inline int col() const
		{
			return _col;
		}
		//! Returns error code
		inline const Code code() const
		{
			return _code;
		}
		//! Returns error message
		const char * msg() const throw ();

		//! Returns full error message
		virtual const char * what() const throw ();
	private:
		const char _ch;
		const int _pos;
		const int _line;
		const int _col;
		const Code _code;
		mutable std::string _what;
	};
	//! Constructs parser
	/*!
	  \param maxFirstTokenLength Maximum first token length
	  \param maxSecondTokenLength Maximum second token length
	  \param maxThirdTokenLength Maximum third token length
	  \param maxHeaderNameLength Maximum header name length
	  \param maxHeaderValueLength Maximum header value length
	  \param maxHeadersAmount Maximum headers amount
	*/
	MessageParser(size_t maxFirstTokenLength, size_t maxSecondTokenLength, size_t maxThirdTokenLength,
			size_t maxHeaderNameLength = DefaultMaxHeaderNameLength,
			size_t maxHeaderValueLength = DefaultMaxHeaderValueLength,
			size_t maxHeadersAmount = DefaultMaxHeadersAmount);
	virtual ~MessageParser();

	//! Returns a current position of the HTTP-message parser (starts from 0)
	inline size_t pos() const
	{
		return _pos;
	}
	//! Returns a current line of the HTTP-message parser (starts from 1)
	inline size_t line() const
	{
		return _line;
	}
	//! Returns a current column of the HTTP-message parser (starts from 1)
	inline size_t col() const
	{
		return _col;
	}
	//! Returns a constant reference to the first token
	inline const std::string& firstToken() const
	{
		return _firstToken;
	}
	//! Returns a constant reference to the second token
	inline const std::string& secondToken() const
	{
		return _secondToken;
	}
	//! Returns a constant reference to the third token
	inline const std::string& thirdToken() const
	{
		return _thirdToken;
	}
	//! Returns a constant reference to the HTTP-message headers
	inline const httpxx::Headers& headers() //const
	{
		return _headers;
	}
	//! Returns maximum header field name length
	inline size_t maxHeaderNameLength() const
	{
		return _maxHeaderNameLength;
	}
	//! Sets maximum header field name length
	/*!
	  \param newValue New maximum header field name length
	*/
	inline void setMaxHeaderNameLength(size_t newValue)
	{
		_maxHeaderNameLength = newValue;
	}
	//! Returns maximum header field value length
	inline size_t maxHeaderValueLength() const
	{
		return _maxHeaderValueLength;
	}
	//! Sets maximum header field value length
	/*!
	  \param newValue New maximum header field value length
	*/
	inline void setMaxHeaderValueLength(size_t newValue)
	{
		_maxHeaderValueLength = newValue;
	}
	//! Returns maximum headers amount
	inline size_t maxHeadersAmount() const
	{
		return _maxHeadersAmount;
	}
	//! Sets maximum headers amount
	/*!
	  \param newValue New maximum headers amount
	*/
	inline void setMaxHeadersAmount(size_t newValue)
	{
		_maxHeadersAmount = newValue;
	}
	//! Return the state of the parser
	inline State state() const
	{
		return _state;
	}
	//! Returns TRUE if the whole HTTP-message has been completely parsed
	inline bool isCompleted() const
	{
		return _state == ParsingMessage;
	}
	//! Parses next character
	/*!
	  \param ch Next character to parse
	  \param isBodyChar Optional pointer to flag where TRUE is to be put if parsed character is a part of HTTP-message body
	  \return TRUE if complete message has been successfully parsed
	*/
	bool parse(char ch, bool * isBodyChar = 0);
	//! Inspects if next character expected to be of HTTP-message body
	inline bool bodyExpected() const
	{
		return _state == ParsingIdentityBody || _state == ParsingChunk;
	}
	//! Parses buffer for an HTTP-message and puts it's body to the supplied buffer
	/*!
	  This method parses a buffer until complete or bad HTTP-message has been parsed or when the
	  supplied body buffer has no space for the body of the HTTP-message.
	  \param parseBuffer Pointer to the buffer to parse
	  \param parseBufferSize Size of the buffer to parse
	  \param bodyBuffer Pointer to buffer where to store HTTP-message body
	  \param bodyBufferSize Size of the buffer where to store HTTP-message body
	  \return Pair with parsed bytes amount and the amount of bytes, which has been stored in body buffer
	*/
	//std::pair<size_t, size_t> parse(const char * parseBuffer, size_t parseBufferSize, char * bodyBuffer, size_t bodyBufferSize);
	//! Parses buffer for an HTTP-message and stores it's body into the supplied stream
	/*!
	  \param parseBuffer Pointer to the buffer to parse
	  \param parseBufferSize Size of the buffer to parse
	  \param os Output stream where to store HTTP-message body
	  \return A pair with complete message flag and parsed bytes amount
	*/
	std::pair<bool, size_t> parse(const void * parseBuffer, size_t parseBufferSize, std::ostream& os);
	//! Resets parser
	virtual void reset();
protected:
	//! Sets parser to the error state
	//void setIsBad(char ch, const std::string& errMsg);
	//! On first token parsed event handler
	/*!
	  \param token Parsed token value
	*/
	virtual void onFirstTokenParsed(const std::string& token)
	{}
	//! On second token parsed event handler
	/*!
	  \param token Parsed token value
	*/
	virtual void onSecondTokenParsed(const std::string& token)
	{}
	//! On third token parsed event handler
	/*!
	  \param token Parsed token value
	*/
	virtual void onThirdTokenParsed(const std::string& token)
	{}
private:
	MessageParser();

	void appendHeader(char ch);
	void parseHeader(char ch, bool isTrailer);
	void parseHeaderName(char ch, bool isTrailer);
	void parseHeaderValue(char ch, bool isTrailer);
	void parseHeaderValueLF(char ch, bool isTrailer);
	void parseHeaderValueLWS(char ch, bool isTrailer);

	State _state;
	//std::auto_ptr<AbstractError> _errorAutoPtr;
	size_t _pos;
	size_t _line;
	size_t _col;
	std::string _firstToken;
	std::string _secondToken;
	std::string _thirdToken;
	std::string _headerFieldName;
	std::string _headerFieldValue;
	httpxx::Headers _headers;
	size_t _contentLength;
	size_t _identityBodyBytesParsed;
	std::string _chunkSizeStr;
	size_t _chunkSize;
	size_t _chunkBytesParsed;
	size_t _maxFirstTokenLength;
	size_t _maxSecondTokenLength;
	size_t _maxThirdTokenLength;
	size_t _maxHeaderNameLength;
	size_t _maxHeaderValueLength;
	size_t _maxHeadersAmount;
};

} // namespace httpxx

#endif
