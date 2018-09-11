#include "header.h"
#include "http.h"
#include "connection.h"
std::string HttpResponse::GetResponse()                                     //从HttpResponse中返回所需要的内容字节流
{
	std::ostringstream ostream;
	ostream << "HTTP/1.1" << " " << http_code << " " << http_phrase << "\r\n"
			<< "Connection: keep-alive" << "\r\n";

	header_iter_t iter = http_headers.begin();

	while (iter != http_headers.end()) {
		ostream << iter->first << ": " << iter->second << "\r\n";
		++iter;
	}


	std::string b = "<html>\n"
					"<title>slighttpd</title>\n"
					"<p>Welcome to slighttpd.\n"
					"<h1>CGI demo</h1>\n"
					"<p>\n"
					"<a href=\"../cgi/date.cgi\">Display Date</a>\n"
					"</form>\n"
					"</html>";

	ostream << "Content-Length: " << b.size() << "\r\n\r\n";
	//ostream << "Content-Length: " << http_body.size() << "\r\n\r\n";
	ostream << std::string(b);

	return ostream.str();
}

void HttpResponse::ResetResponse()                                          //重置Response
{
	//http_version = "HTTP/1.1";
	http_code = 200;
	http_phrase = "OK";

	http_body.clear();
	http_headers.clear();
}

void HttpParser::InitParser(Connection *con) {
	bzero(&settings, sizeof(settings));     //初始化http_parser_settings
	settings.on_message_begin = OnMessageBeginCallback;
	settings.on_url = OnUrlCallback;
	settings.on_header_field = OnHeaderFieldCallback;
	settings.on_header_value = OnHeaderValueCallback;
	settings.on_headers_complete = OnHeadersCompleteCallback;
	settings.on_body = OnBodyCallback;
	settings.on_message_complete = OnMessageCompleteCallback;

	http_parser_init(&parser, HTTP_REQUEST);

	parser.data = con;           //parser对象里面指向连接conn的钩子指针
}

int HttpParser::HttpParseRequest(const std::string &inbuf) {//在parse的灰调函数中对connection中的request进行处理
	int nparsed = http_parser_execute(&parser, &settings, inbuf.c_str(), inbuf.size());//该函数不会对原缓冲区产生任何影响

	if (parser.http_errno != HPE_OK)      //http_parse执行出错http_errno就会设置成错误类型
	{
		return -1;
	}

	return nparsed;
}

int HttpParser::OnMessageBeginCallback(http_parser *parser)     //request解析开始时调用，此时new一个HttpRequest对象
{
	Connection *con = static_cast<Connection *>(parser->data);

	con->http_req_parser = new HttpRequest();           //注意析构

	return 0;
}

int HttpParser::OnUrlCallback(http_parser *parser, const char *at, size_t length)   //获取request的URL
{
	Connection *con = static_cast<Connection *>(parser->data);

	con->http_req_parser->http_url.assign(at, length);

	//std::cout<<"URL:"<<at<<std::endl;

	return 0;
}

int HttpParser::OnHeaderFieldCallback(http_parser *parser, const char *at, size_t length) {
	Connection *con = static_cast<Connection *>(parser->data);

	con->http_req_parser->http_header_field.assign(at, length);

	return 0;
}

int HttpParser::OnHeaderValueCallback(http_parser *parser, const char *at,
									  size_t length)       //把http头的键值对放到map<std::string,std::string>里面去
{
	Connection *con = static_cast<Connection *>(parser->data);
	HttpRequest *request = con->http_req_parser;

	request->http_headers[request->http_header_field] = std::string(at, length);

	//std::cout<<request->http_header_field<<":"<<at<<std::endl;

	return 0;
}

int HttpParser::OnHeadersCompleteCallback(http_parser *parser) //获取request类型
{
	Connection *con = static_cast<Connection *>(parser->data);
	HttpRequest *request = con->http_req_parser;
	request->http_method = http_method_str((http_method) parser->method);

	//std::cout<<"http_method:"<<request->http_method<<std::endl;

	return 0;
}

int HttpParser::OnBodyCallback(http_parser *parser, const char *at, size_t length) //请求主体body获取
{
	Connection *con = static_cast<Connection *>(parser->data);

	// NOTICE:OnBody may be called many times per Reuqest
	con->http_req_parser->http_body.append(at, length);

	//std::cout<<"body:"<<at<<std::endl;

	return 0;
}

int HttpParser::OnMessageCompleteCallback(http_parser *parser)  //结束解析的时候调用
{
	Connection *con = static_cast<Connection *>(parser->data);
	HttpRequest *request = con->http_req_parser;

	con->req_queue.push(
			request);       //解析完成队列+1并清空http_req_parser，注意这里并不free数据，因为队列里面存储的是指针
	con->http_req_parser = nullptr;


	//std::cout<<"http_parser end!"<<std::endl;
	//std::cout << __FUNCTION__ << std::endl;

	return 0;
}
