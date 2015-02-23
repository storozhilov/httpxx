#include <httpxx/message_parser.h>
#include <sstream>
#include "char_utils.h"
#include "string_utils.h"

namespace {

const char * ErrorCodeMessages[] =
	{
		"Invalid character in first token", /* exception::InvalidFirstToken */
		"First token is too long", /* exception::FirstTokenIsTooLong */
		"Invalid character in second token", /* exception::InvalidSecondToken */
		"Second token is too long", /* exception::SecondTokenIsTooLong */
		"Invalid character in third token", /* exception::InvalidThirdToken */
		"Third token is too long", /* exception::ThirdTokenIsTooLong */
		"First line CR is followed by invalid character", /* exception::InvalidFirstLineLF */
		"Too many headers", /* exception::TooManyHeaders */
		"Empty header name", /* exception::EmptyHeaderName */
		"Invalid header name", /* exception::InvalidHeaderName */
		"Header name is too long", /* exception::HeaderNameIsTooLong */
		"Header is missing ':' separator", /* exception::HeaderIsMissingColon */
		"Invalid header value", /* exception::InvalidHeaderValue */
		"Header value is too long", /* exception::HeaderValueIsTooLong */
		"Header CR is followed by invalid character", /* exception::InvalidHeaderLF */
		"Invalid content length", /* exception::InvalidContentLength */
		"Empty chunk size", /* exception::EmptyChunkSize */
		"Invalid chunk size", /* exception::InvalidChunkSize */
		"Chunk size CR is followed by invalid character", /* exception::InvalidChunkSizeLF */
		"Chunk data is followed by invalid character", /* exception::InvalidChunkDataCR */
		"Chunk data CR is followed by invalid character", /* exception::InvalidChunkDataLF */
		"Final CR is followed by invalid character", /* exception::InvalidFinalLF */
		"Invalid parser state", /* exception::InvalidState - should never happens */
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
			throw exception(ch, _pos, _line, _col, exception::InvalidFirstToken);
		}
		break;
	case ParsingLeadingSP:
		if (isSpaceOrTab(ch)) {
			// Just ignore leading space
		} else if (isChar(ch) && !isControl(ch)) {
			_firstToken += ch;
			_state = ParsingFirstToken;
		} else {
			throw exception(ch, _pos, _line, _col, exception::InvalidFirstToken);
		}
		break;
	case ParsingFirstToken:
		if (isSpaceOrTab(ch)) {
			_state = ParsingFirstTokenSP;
		} else if (isChar(ch) && !isControl(ch)) {
			if (_firstToken.length() >= _maxFirstTokenLength) {
				throw exception(ch, _pos, _line, _col, exception::FirstTokenIsTooLong);
			} else {
				_firstToken += ch;
			}
		} else {
			throw exception(ch, _pos, _line, _col, exception::InvalidFirstToken);
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
			throw exception(ch, _pos, _line, _col, exception::InvalidSecondToken);
		}
		break;
	case ParsingSecondToken:
		if (isSpaceOrTab(ch)) {
			_state = ParsingSecondTokenSP;
		} else if (isChar(ch) && !isControl(ch)) {
			if (_secondToken.length() >= _maxSecondTokenLength) {
				throw exception(ch, _pos, _line, _col, exception::SecondTokenIsTooLong);
			} else {
				_secondToken += ch;
			}
		} else {
			throw exception(ch, _pos, _line, _col, exception::InvalidSecondToken);
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
			throw exception(ch, _pos, _line, _col, exception::InvalidThirdToken);
		}
		break;
	case ParsingThirdToken:
		if (isCarriageReturn(ch)) {
			_state = ParsingFirstLineLF;
		} else if (isChar(ch) && !isControl(ch)) {
			if (_thirdToken.length() >= _maxThirdTokenLength) {
				throw exception(ch, _pos, _line, _col, exception::ThirdTokenIsTooLong);
			} else {
				_thirdToken += ch;
			} 
		} else {
			throw exception(ch, _pos, _line, _col, exception::InvalidThirdToken);
		}
		break;
	case ParsingFirstLineLF:
		if (isLineFeed(ch)) {
			_state = ParsingHeader;
		} else {
			throw exception(ch, _pos, _line, _col, exception::InvalidFirstLineLF);
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
					throw exception(ch, _pos, _line, _col, exception::InvalidContentLength);
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
			throw exception(ch, _pos, _line, _col, exception::InvalidHeaderLF);
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
				throw exception(ch, _pos, _line, _col, exception::EmptyChunkSize);
			} else {
				try {
					_chunkSize = toUnsignedInt(_chunkSizeStr, true);
				} catch (std::exception& e) {
					std::ostringstream msg;
					msg << "Error casting chunk size '" << _chunkSizeStr <<
						"' as unsigned integer hex value: " << e.what();
					throw exception(ch, _pos, _line, _col, exception::InvalidChunkSize);
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
			throw exception(ch, _pos, _line, _col, exception::InvalidChunkSizeLF);
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
			throw exception(ch, _pos, _line, _col, exception::InvalidChunkDataCR);
		}
		break;
	case ParsingChunkLF:
		if (isLineFeed(ch)) {
			_state = ParsingChunkSize;
		} else {
			throw exception(ch, _pos, _line, _col, exception::InvalidChunkDataLF);
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
			throw exception(ch, _pos, _line, _col, exception::InvalidFinalLF);
		}
		break;
	default:
		throw exception(ch, _pos, _line, _col, exception::InvalidState);
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
		throw exception(ch, _pos, _line, _col, exception::TooManyHeaders);
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
		throw exception(ch, _pos, _line, _col, exception::EmptyHeaderName);
	} else if (isToken(ch)) {
		// Header field name is empty -> no length check
		_headerFieldName += ch;
		_state = isTrailer ? ParsingTrailerHeaderName : ParsingHeaderName;
	} else {
		throw exception(ch, _pos, _line, _col, exception::InvalidHeaderName);
	}
}

void MessageParser::parseHeaderName(char ch, bool isTrailer)
{
	if (isCarriageReturn(ch)) {
		throw exception(ch, _pos, _line, _col, exception::HeaderIsMissingColon);
	} else if (ch == ':') {
		_state = isTrailer ? ParsingTrailerHeaderValue : ParsingHeaderValue;
	} else if (isToken(ch)) {
		if (_headerFieldName.length() < maxHeaderNameLength()) {
			_headerFieldName += ch;
		} else {
			throw exception(ch, _pos, _line, _col, exception::HeaderNameIsTooLong);
		}
	} else {
		throw exception(ch, _pos, _line, _col, exception::InvalidHeaderName);
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
			throw exception(ch, _pos, _line, _col, exception::HeaderValueIsTooLong);
		}
	} else {
		throw exception(ch, _pos, _line, _col, exception::InvalidHeaderValue);
	}
}

void MessageParser::parseHeaderValueLF(char ch, bool isTrailer)
{
	if (isLineFeed(ch)) {
		_state = isTrailer ? ParsingTrailerHeaderValueLWS : ParsingHeaderValueLWS;
	} else {
		throw exception(ch, _pos, _line, _col, exception::InvalidHeaderLF);
	}
}

void MessageParser::parseHeaderValueLWS(char ch, bool isTrailer)
{
	if (isCarriageReturn(ch)) {
		appendHeader(ch);
		_state = isTrailer ? ParsingFinalLF : ParsingEndOfHeader;
	} else if (ch == ':') {
		throw exception(ch, _pos, _line, _col, exception::EmptyHeaderName);
	} else if (isSpaceOrTab(ch)) {
		if (_headerFieldValue.length() < maxHeaderValueLength()) {
			_headerFieldValue += ' ';
			_state = isTrailer ? ParsingTrailerHeaderValue : ParsingHeaderValue;
		} else {
			throw exception(ch, _pos, _line, _col, exception::HeaderValueIsTooLong);
		}
	} else if (isToken(ch)) {
		appendHeader(ch);
		// Header field name is empty -> no length check
		_headerFieldName += ch;
		_state = isTrailer ? ParsingTrailerHeaderName : ParsingHeaderName;
	} else {
		throw exception(ch, _pos, _line, _col, exception::InvalidHeaderName);
	}
}

//------------------------------------------------------------------------------
// MessageParser::exception
//------------------------------------------------------------------------------

MessageParser::exception::exception(char ch, int pos, int line, int col, Code code) :
	_ch(ch),
	_pos(pos),
	_line(line),
	_col(col),
	_code(code),
	_what()
{}

const char * MessageParser::exception::msg() const throw ()
{
	return ErrorCodeMessages[_code];
}

const char * MessageParser::exception::what() const throw ()
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
