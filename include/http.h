#ifndef _HTTP_H
#define _HTTP_H

#include "header.h"

class Connection;

typedef std::map<std::string, std::string> header_t;
typedef header_t::iterator header_iter_t;

struct HttpRequestContent {
	std::string http_method;
	std::string http_url;

	header_t http_headers;
	std::string http_header_field; //field is waiting for value while parsing

	std::string http_body;
};

struct HttpResponseContent {
	int http_code;
	std::string http_phrase;

	header_t http_headers;

	std::string http_body;

	std::string GetResponse();

	void ResetResponse();
};

class HttpParser {
public:

	void InitParser(Connection *con);

	int HttpParseRequest(const std::string &inbuf);

private:                                                                        //当前类的私有静态函数

	static int OnMessageBeginCallback(
			http_parser *parser);  //http_parser在解析过程中并不保留状态，相关数据或者要求可以在相应settings中通过设置回调函数实现
	static int OnUrlCallback(http_parser *parser, const char *at, size_t length);

	static int OnHeaderFieldCallback(http_parser *parser, const char *at, size_t length);

	static int OnHeaderValueCallback(http_parser *parser, const char *at, size_t length);

	static int OnHeadersCompleteCallback(http_parser *parser);

	static int OnBodyCallback(http_parser *parser, const char *at, size_t length);

	static int OnMessageCompleteCallback(http_parser *parser);

private:

	http_parser parser;
	http_parser_settings settings;
};

#endif
