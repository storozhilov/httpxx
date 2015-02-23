#include <httpxx/message_parser.h>
#include <sstream>
#include "char_utils.h"
#include "string_utils.h"

namespace {

const char * ErrorCodeMessages[] =
	{
		"Invalid character in first token", /* Exception::InvalidFirstToken */
		"First token is too long", /* Exception::FirstTokenIsTooLong */
		"Invalid character in second token", /* Exception::InvalidSecondToken */
		"Second token is too long", /* Exception::SecondTokenIsTooLong */
		"Invalid character in third token", /* Exception::InvalidThirdToken */
		"Third token is too long", /* Exception::ThirdTokenIsTooLong */
		"First line CR is followed by invalid character", /* Exception::InvalidFirstLineLF */
		"Too many headers", /* Exception::TooManyHeaders */
		"Empty header name", /* Exception::EmptyHeaderName */
		"Invalid header name", /* Exception::InvalidHeaderName */
		"Header name is too long", /* Exception::HeaderNameIsTooLong */
		"Header is missing ':' separator", /* Exception::HeaderIsMissingColon */
		"Invalid header value", /* Exception::InvalidHeaderValue */
		"Header value is too long", /* Exception::HeaderValueIsTooLong */
		"Header CR is followed by invalid character", /* Exception::InvalidHeaderLF */
		"Invalid content length", /* Exception::InvalidContentLength */
		"Empty chunk size", /* Exception::EmptyChunkSize */
		"Invalid chunk size", /* Exception::InvalidChunkSize */
		"Chunk size CR is followed by invalid character", /* Exception::InvalidChunkSizeLF */
		"Chunk data is followed by invalid character", /* Exception::InvalidChunkDataCR */
		"Chunk data CR is followed by invalid character", /* Exception::InvalidChunkDataLF */
		"Final CR is followed by invalid character", /* Exception::InvalidFinalLF */
		"Invalid parser state", /* Exception::InvalidState - should never happens */
	};

}

namespace httpxx
{

//------------------------------------------------------------------------------
// MessageParser
//------------------------------------------------------------------------------

MessageParser::MessageParser(size_t maxFirstTokenLength, size_t maxSecondTokenLength, size_t maxThirdTokenLength,
		size_t maxHeaderNameLength, size_t maxHeaderValueLength, size_t maxHeadersAmount) :
	_state(ParsingMessage),
	_pos(0),
	_line(1),
	_col(1),
	_firstToken(),
	_secondToken(),
	_thirdToken(),
	_headerFieldName(),
	_headerFieldValue(),
	_headers(),
	_contentLength(0),
	_identityBodyBytesParsed(0),
	_chunkSizeStr(),
	_chunkSize(0),
	_chunkBytesParsed(0),
	_maxFirstTokenLength(maxFirstTokenLength),
	_maxSecondTokenLength(maxSecondTokenLength),
	_maxThirdTokenLength(maxThirdTokenLength),
	_maxHeaderNameLength(maxHeaderNameLength),
	_maxHeaderValueLength(maxHeaderValueLength),
	_maxHeadersAmount(maxHeadersAmount)
{}

MessageParser::~MessageParser()
{}

bool MessageParser::parse(char ch, bool * isBodyChar)
{
	bool bodyByteExtracted = bodyExpected();
	switch (_state) {
	case ParsingMessage:
		if (isSpaceOrTab(ch)) {
			reset();
			_state = ParsingLeadingSP;
		} else if (isChar(ch) && !isControl(ch)) {
			reset();
			_firstToken += ch;
			_state = ParsingFirstToken;
		} else {
			throw Exception(ch, _pos, _line, _col, Exception::InvalidFirstToken);
		}
		break;
	case ParsingLeadingSP:
		if (isSpaceOrTab(ch)) {
			// Just ignore leading space
		} else if (isChar(ch) && !isControl(ch)) {
			_firstToken += ch;
			_state = ParsingFirstToken;
		} else {
			throw Exception(ch, _pos, _line, _col, Exception::InvalidFirstToken);
		}
		break;
	case ParsingFirstToken:
		if (isSpaceOrTab(ch)) {
			_state = ParsingFirstTokenSP;
		} else if (isChar(ch) && !isControl(ch)) {
			if (_firstToken.length() >= _maxFirstTokenLength) {
				throw Exception(ch, _pos, _line, _col, Exception::FirstTokenIsTooLong);
			} else {
				_firstToken += ch;
			}
		} else {
			throw Exception(ch, _pos, _line, _col, Exception::InvalidFirstToken);
		}
		break;
	case ParsingFirstTokenSP:
		if (isSpaceOrTab(ch)) {
			// Just ignore it
		} else if (isChar(ch) && !isControl(ch)) {
			// Second token is empty -> no length check
			_secondToken += ch;
			_state = ParsingSecondToken;
		} else {
			throw Exception(ch, _pos, _line, _col, Exception::InvalidSecondToken);
		}
		break;
	case ParsingSecondToken:
		if (isSpaceOrTab(ch)) {
			_state = ParsingSecondTokenSP;
		} else if (isChar(ch) && !isControl(ch)) {
			if (_secondToken.length() >= _maxSecondTokenLength) {
				throw Exception(ch, _pos, _line, _col, Exception::SecondTokenIsTooLong);
			} else {
				_secondToken += ch;
			}
		} else {
			throw Exception(ch, _pos, _line, _col, Exception::InvalidSecondToken);
		}
		break;
	case ParsingSecondTokenSP:
		if (isSpaceOrTab(ch)) {
			// Just ignore it
		} else if (isChar(ch) && !isControl(ch)) {
			// Third token is empty -> no length check
			_thirdToken += ch;
			_state = ParsingThirdToken;
		} else {
			throw Exception(ch, _pos, _line, _col, Exception::InvalidThirdToken);
		}
		break;
	case ParsingThirdToken:
		if (isCarriageReturn(ch)) {
			_state = ParsingFirstLineLF;
		} else if (isChar(ch) && !isControl(ch)) {
			if (_thirdToken.length() >= _maxThirdTokenLength) {
				throw Exception(ch, _pos, _line, _col, Exception::ThirdTokenIsTooLong);
			} else {
				_thirdToken += ch;
			} 
		} else {
			throw Exception(ch, _pos, _line, _col, Exception::InvalidThirdToken);
		}
		break;
	case ParsingFirstLineLF:
		if (isLineFeed(ch)) {
			_state = ParsingHeader;
		} else {
			throw Exception(ch, _pos, _line, _col, Exception::InvalidFirstLineLF);
		}
		break;
	case ParsingHeader:
		parseHeader(ch, false);
		break;
	case ParsingHeaderName:
		parseHeaderName(ch, false);
		break;
	case ParsingHeaderValue:
		parseHeaderValue(ch, false);
		break;
	case ParsingHeaderValueLF:
		parseHeaderValueLF(ch, false);
		break;
	case ParsingHeaderValueLWS:
		parseHeaderValueLWS(ch, false);
		break;
	case ParsingEndOfHeader:
		if (isLineFeed(ch)) {
			if (_headers.have("Transfer-Encoding", "chunked")) {
				_state = ParsingChunkSize;
			} else if (_headers.have("Content-Length")) {
				// Extracting the content length
				try {
					_contentLength = toUnsignedInt(_headers.value("Content-Length"));
				} catch (std::exception& /* e */) {
					throw Exception(ch, _pos, _line, _col, Exception::InvalidContentLength);
				}
				if (_contentLength <= 0) {
					_state = ParsingMessage;
				} else {
					_state = ParsingIdentityBody;
				}
			} else {
				_state = ParsingMessage;
			}
		} else {
			throw Exception(ch, _pos, _line, _col, Exception::InvalidHeaderLF);
		}
		break;
	case ParsingIdentityBody:
		++_identityBodyBytesParsed;
		if (_identityBodyBytesParsed >= _contentLength) {
			_state = ParsingMessage;
		}
		break;
	case ParsingChunkSize:
		if (isHexDigit(ch)) {
			_chunkSizeStr += ch;
		} else {
			if (_chunkSizeStr.empty()) {
				throw Exception(ch, _pos, _line, _col, Exception::EmptyChunkSize);
			} else {
				try {
					_chunkSize = toUnsignedInt(_chunkSizeStr, true);
				} catch (std::exception& /* e */) {
					throw Exception(ch, _pos, _line, _col, Exception::InvalidChunkSize);
				}
				_chunkBytesParsed = 0;
				_chunkSizeStr.clear();
				if (isCarriageReturn(ch)) {
					_state = ParsingChunkSizeLF;
				} else {
					_state = ParsingChunkExtension;
				}
			}
		}
		break;
	case ParsingChunkExtension:
		// Just ignore a chunk extension.
		if (isCarriageReturn(ch)) {
			_state = ParsingChunkSizeLF;
		}
		break;
	case ParsingChunkSizeLF:
		if (isLineFeed(ch)) {
			_state = (_chunkSize > 0) ? ParsingChunk : ParsingTrailerHeader;
		} else {
			throw Exception(ch, _pos, _line, _col, Exception::InvalidChunkSizeLF);
		}
		break;
	case ParsingChunk:
		++_chunkBytesParsed;
		if (_chunkBytesParsed >= _chunkSize) {
			_state = ParsingChunkCR;
		}
		break;
	case ParsingChunkCR:
		if (isCarriageReturn(ch)) {
			_state = ParsingChunkLF;
		} else {
			throw Exception(ch, _pos, _line, _col, Exception::InvalidChunkDataCR);
		}
		break;
	case ParsingChunkLF:
		if (isLineFeed(ch)) {
			_state = ParsingChunkSize;
		} else {
			throw Exception(ch, _pos, _line, _col, Exception::InvalidChunkDataLF);
		}
		break;
	case ParsingTrailerHeader:
		parseHeader(ch, true);
		break;
	case ParsingTrailerHeaderName:
		parseHeaderName(ch, true);
		break;
	case ParsingTrailerHeaderValue:
		parseHeaderValue(ch, true);
		break;
	case ParsingTrailerHeaderValueLF:
		parseHeaderValueLF(ch, true);
		break;
	case ParsingTrailerHeaderValueLWS:
		parseHeaderValueLWS(ch, true);
		break;
	case ParsingFinalLF:
		if (isLineFeed(ch)) {
			_state = ParsingMessage;
		} else {
			throw Exception(ch, _pos, _line, _col, Exception::InvalidFinalLF);
		}
		break;
	default:
		throw Exception(ch, _pos, _line, _col, Exception::InvalidState);
	}
	// Updating current position data
	++_pos;
	if (isLineFeed(ch)) {
		++_line;
		_col = 1;
	} else {
		++_col;
	}
	if (isBodyChar != 0) {
		*isBodyChar = bodyByteExtracted;
	}
	return _state == ParsingMessage;
}

/*std::pair<size_t, size_t> MessageParser::parse(const char * parseBuffer, size_t parseBufferSize, char * bodyBuffer, size_t bodyBufferSize)
{
	size_t bytesParsed = 0;
	size_t bodyBytes = 0;
	while (bytesParsed < parseBufferSize) {
		if (bodyExpected() && (bodyBytes >= bodyBufferSize)) {
			// Body buffer has no space for body byte
			break;
		}
		char ch = *(parseBuffer + bytesParsed++);
		if (parse(ch)) {
			*(bodyBuffer + bodyBytes++) = ch;
		}
		if (isCompleted() || isBad()) {
			break;
		}
	}
	return std::pair<size_t, size_t>(bytesParsed, bodyBytes);
}

size_t MessageParser::parse(const char * parseBuffer, size_t parseBufferSize, std::ostream& os)
{
	size_t bytesParsed = 0;
	while (bytesParsed < parseBufferSize) {
		char ch = *(parseBuffer + bytesParsed++);
		if (parse(ch)) {
			os.put(ch);
		}
		if (isCompleted() || isBad()) {
			break;
		}
	}
	return bytesParsed;
}*/

std::pair<bool, size_t> MessageParser::parse(const void * parseBuffer, size_t parseBufferSize, std::ostream& os)
{
	const char * pb = static_cast<const char *>(parseBuffer);
	size_t bytesParsed = 0;
	bool completeMessageDetected = false;
	while (bytesParsed < parseBufferSize && !completeMessageDetected) {
		char ch = *(pb + bytesParsed++);
		bool isBodyChar;
		completeMessageDetected = parse(ch, &isBodyChar);
		if (isBodyChar) {
			os.put(ch);
		}
	}
	return std::pair<bool, size_t>(completeMessageDetected, bytesParsed);
}

void MessageParser::reset()
{
	_state = ParsingMessage;
	_pos = 0;
	_line = 1;
	_col = 1;
	_firstToken.clear(),
	_secondToken.clear(),
	_thirdToken.clear(),
	_headerFieldName.clear();
	_headerFieldValue.clear();
	_headers.clear();
	_contentLength = 0;
	_identityBodyBytesParsed = 0;
	_chunkSizeStr.clear();
	_chunkSize = 0;
	_chunkBytesParsed = 0;
}

void MessageParser::appendHeader(char ch)
{
	if (_headers.size() >= _maxHeadersAmount) {
		throw Exception(ch, _pos, _line, _col, Exception::TooManyHeaders);
	}
	trim(_headerFieldName);
	trim(_headerFieldValue);
	_headers.insert(Headers::value_type(_headerFieldName, _headerFieldValue));
	_headerFieldName.clear();
	_headerFieldValue.clear();
}

void MessageParser::parseHeader(char ch, bool isTrailer)
{
	_headerFieldName.clear();
	_headerFieldValue.clear();
	if (isCarriageReturn(ch)) {
		_state = isTrailer ? ParsingFinalLF : ParsingEndOfHeader;
	} else if (ch == ':') {
		throw Exception(ch, _pos, _line, _col, Exception::EmptyHeaderName);
	} else if (isToken(ch)) {
		// Header field name is empty -> no length check
		_headerFieldName += ch;
		_state = isTrailer ? ParsingTrailerHeaderName : ParsingHeaderName;
	} else {
		throw Exception(ch, _pos, _line, _col, Exception::InvalidHeaderName);
	}
}

void MessageParser::parseHeaderName(char ch, bool isTrailer)
{
	if (isCarriageReturn(ch)) {
		throw Exception(ch, _pos, _line, _col, Exception::HeaderIsMissingColon);
	} else if (ch == ':') {
		_state = isTrailer ? ParsingTrailerHeaderValue : ParsingHeaderValue;
	} else if (isToken(ch)) {
		if (_headerFieldName.length() < maxHeaderNameLength()) {
			_headerFieldName += ch;
		} else {
			throw Exception(ch, _pos, _line, _col, Exception::HeaderNameIsTooLong);
		}
	} else {
		throw Exception(ch, _pos, _line, _col, Exception::InvalidHeaderName);
	}
}

void MessageParser::parseHeaderValue(char ch, bool isTrailer)
{
	if (isCarriageReturn(ch)) {
		_state = isTrailer ? ParsingTrailerHeaderValueLF : ParsingHeaderValueLF;
	} else if (!isControl(ch)) {
		if (_headerFieldValue.length() < maxHeaderValueLength()) {
			_headerFieldValue += ch;
		} else {
			throw Exception(ch, _pos, _line, _col, Exception::HeaderValueIsTooLong);
		}
	} else {
		throw Exception(ch, _pos, _line, _col, Exception::InvalidHeaderValue);
	}
}

void MessageParser::parseHeaderValueLF(char ch, bool isTrailer)
{
	if (isLineFeed(ch)) {
		_state = isTrailer ? ParsingTrailerHeaderValueLWS : ParsingHeaderValueLWS;
	} else {
		throw Exception(ch, _pos, _line, _col, Exception::InvalidHeaderLF);
	}
}

void MessageParser::parseHeaderValueLWS(char ch, bool isTrailer)
{
	if (isCarriageReturn(ch)) {
		appendHeader(ch);
		_state = isTrailer ? ParsingFinalLF : ParsingEndOfHeader;
	} else if (ch == ':') {
		throw Exception(ch, _pos, _line, _col, Exception::EmptyHeaderName);
	} else if (isSpaceOrTab(ch)) {
		if (_headerFieldValue.length() < maxHeaderValueLength()) {
			_headerFieldValue += ' ';
			_state = isTrailer ? ParsingTrailerHeaderValue : ParsingHeaderValue;
		} else {
			throw Exception(ch, _pos, _line, _col, Exception::HeaderValueIsTooLong);
		}
	} else if (isToken(ch)) {
		appendHeader(ch);
		// Header field name is empty -> no length check
		_headerFieldName += ch;
		_state = isTrailer ? ParsingTrailerHeaderName : ParsingHeaderName;
	} else {
		throw Exception(ch, _pos, _line, _col, Exception::InvalidHeaderName);
	}
}

//------------------------------------------------------------------------------
// MessageParser::Exception
//------------------------------------------------------------------------------

MessageParser::Exception::Exception(char ch, int pos, int line, int col, Code code) :
	std::exception(),
	_ch(ch),
	_pos(pos),
	_line(line),
	_col(col),
	_code(code),
	_what()
{}

const char * MessageParser::Exception::msg() const throw ()
{
	return ErrorCodeMessages[_code];
}

const char * MessageParser::Exception::what() const throw ()
{
	if (_what.empty()) {
		std::ostringstream oss;
		oss << "HTTP-message parsing error (pos: " << _pos << ", line: " << _line << ", col: " << _col <<
			", character: " << std::showbase << std::hex << static_cast<int>(_ch) << "): " <<
			ErrorCodeMessages[_code];
		_what = oss.str();
	}
	return _what.c_str();
}

} // namespace httpxx
