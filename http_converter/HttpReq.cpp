#include "HttpReq.hpp"
#include "utils.hpp"
#include <exception>
#include <iostream>

using http_msg::HttpReq;
using http_msg::str_map_t;
using http_msg::str_t;
using http_msg::str_vec_t;

HttpReq::HttpReq(str_t const &req)
	: _src(req), _offset(0), _method(-1), _status(0), _isChunked(false)
{
}

// parsing
bool HttpReq::parse()
{
	try
	{
		parseStartLine();
		parseHeader();
		parseBody();
		return (true);
	}
	catch (std::exception &e)
	{
		std::cerr << e.what() << std::endl;
		_status = 400;
		return (false);
	}
}

void HttpReq::parseStartLine()
{
	/* line 파싱 */
	str_t line = strBeforeSep(_src, CRLF, _offset);
	if (line == "" || line[0] == ' ')
		throw(std::runtime_error("failed to parse request line."));

	/* 공백 기준 split*/
	str_vec_t tokens = split(line, ' ');
	if (tokens.size() != 3)
		throw(std::runtime_error("token num of request line is not correct."));

	/* method index 구하기 */
	for (int i = 0; i < kMethodCount; i++)
	{
		if (tokens.at(0) == kMethodStr[i])
		{
			_method = i;
			break;
		}
	}
	if (_method == -1)
		throw(std::runtime_error("method is not implemented: " + tokens.at(0)));

	/* uri, version 파싱 */
	_uri = components->at(1);
	_version = components->at(2);
	delete components; // split으로 인한 동적 할당 해제
}

void HttpReq::checkHost()
{
	str_map_t::iterator itr = findKey("Host");

	if (itr == _header.end() || itr->second.size() == 0)
		throw(HttpReqException(kErrHeaderParsing));
}

void HttpReq::checkChunk()
{
	str_map_t::iterator itr = findKey("Transfer-Encoding");

	if (itr == _header.end())
		return;
	_isChunked = true;
}

str_map_t::iterator HttpReq::findKey(std::string key)
{
	str_map_t::iterator itr = _header.begin();
	str_map_t::iterator end = _header.end();

	for (; itr != end; ++itr)
		if (itr->first == key)
			break;
	return (itr);
}

// 여러 줄 헤더 처리
void HttpReq::parseHeader()
{
	str_t line;

	str_t key;
	size_t key_end_idx;

	str_t value;
	str_vec_t vec_value;
	str_map_t::iterator itr;

	while (true)
	{
		/* line 파싱 */
		line = strBeforeSep(_src, CRLF, _offset);
		if (line == "")
			break;

		/* key 파싱 */
		key_end_idx = 0;
		key = strBeforeSep(line, ":", key_end_idx);
		if (key == "" || hasSpace(key))
			throw(std::runtime_error("failed to parse header fields."));

		/* value 파싱 */
		while (std::isspace(line[key_end_idx]))
			++key_end_idx;
		while (true)
		{
			value = strBeforeSep(line, ",", key_end_idx);
			if (value == "")
				break;
			vec_value.push_back(strtrim(value, " "));
		}
		value = strtrim(str_t(&line[key_end_idx]), " ");
		if (value != "")
			vec_value.push_back(value);

		/* key가 있다면, 붙여넣기 */
		itr = findKey(key);
		if (itr == _header.end())
			_header[key] = vec_value;
		else
			itr->second.insert(itr->second.end(), vec_value.begin(),
							   vec_value.end());
		vec_value.clear();
	}
	/* Host 헤더 있는지 확인 */
	checkHost();
	/* Transfer-Encoding: chunked인지 확인 */
	checkChunk();
}

void HttpReq::parseBody()
{
	switch (_method)
	{
	case (kGet):
	case (kDelete): {
		if (_offset != _src.length())
			throw(std::runtime_error("given method must not have the body."));
		break;
	}
	case (method::kPost): {
		const size_t end_idx = _src.find_last_of(
			"\r", _src.size()); // 두 번째 인자부터 뒤로 찾는다. 첫 번째 인자의
								// 문자 중 하나라도 맞으면 반환한다.
		if (end_idx == std::string::npos || end_idx != _src.length() - CRLF_LEN)
			throw(std::runtime_error("failed to parse entity body."));
		_body = _src.substr(_offset, end_idx - _offset);
	}
	}
}

// getter
int HttpReq::getMethod() const
{
	return (_method);
}

str_t const &HttpReq::getUri() const
{
	return (_uri);
}

str_t const &HttpReq::getVersion() const
{
	return (_version);
}

str_map_t const &HttpReq::getHeader() const
{
	return (_header);
}

str_map_t::const_iterator HttpReq::getHeaderVal(str_t const &key) const
{
	return (_header.find(key));
}

str_t const &HttpReq::getBody() const
{
	return (_body);
}
