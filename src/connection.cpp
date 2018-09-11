#include "header.h"
#include "http.h"
#include "connection.h"
Connection::Connection() {
	con_worker = nullptr;
	http_req_parsed = nullptr;
	http_req_parser = nullptr;
	con_req_cnt = 0;
	con_use = CONNECTION_IDLE;
	con_state = CON_STATE_REQUEST_START;
	con_finished = false;
}

Connection::~Connection() {}

bool Connection::InitConnection(Worker *worker) {
	this->con_worker = worker;

	try {                 //先给缓冲区分配足够空间
		con_inbuf.reserve(10 * 1024);
		con_outbuf.reserve(10 * 1024);
	} catch (std::bad_alloc) {
		std::cout << "Connection::InitConnection(): std::bad_alloc" << std::endl;
	}

	http_parser.InitParser(this);

	SetState(CON_STATE_REQUEST_START);

	if (!StateMachine()) {  //状态机内部返回false表示出错，返回false
		return false;
	}

	return true;
}

void Connection::ResetCon() {   //重置连接，把connection里面所有动态内存全部释放
	HttpRequest *request;

	while (!req_queue.empty()) {    //request队列里面的动态request全部析构掉
		request = req_queue.front();
		req_queue.pop();
		delete request;
	}
	if(http_req_parsed)
		delete http_req_parsed;
	if(http_req_parser)
		delete http_req_parser;

	con_worker = nullptr;
	http_req_parsed = nullptr;
	http_req_parser = nullptr;
	con_finished = false;
	con_req_cnt = 0;
}

void Connection::ConReadCallback(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf) {

	Connection *con = (Connection *) (((uv_tcp_t *) stream)->data);

	if (nread > 0) {
		con->con_inbuf += std::string(buf->base, nread);//否则不断调用回调函数进行读取
		con->con_state = CON_STATE_READ_END;
		free(buf->base);
		con->StateMachine();
	}else{ //nread < 0 表示出错, nread == 0 表示连接已经断开,此时直接设置CON_STATE_ERROR
		con->con_state = CON_STATE_ERROR;
		con->StateMachine();
		//uv_close((uv_handle_t*) stream, NULL);
	}
}

void Connection::ConWriteCallback(uv_write_t *req, int status) {

	assert(status == 0);

	write_req_t *wri = (write_req_t *) req;

	//Connection *con = (Connection *) (wri->req.data);

	uv_buf_t *buff = &wri->buf;
	free(buff->base);
	free(wri);
	//con->con_state = CON_STATE_ERROR;//CON_STATE_RESPONSE_END;
	//con->StateMachine();
}

void Connection::ConShutdownCallback(uv_shutdown_t *req, int status) {
	assert(status == 0);
	uv_close((uv_handle_t*) req->data, Connection::ConCloseCallback);
	free(req);
}

void Connection::ConCloseCallback(uv_handle_t *handle) {
	Worker::CloseCon((Connection*)(handle->data));
	free(handle);
}

bool Connection::StateMachine() {                                    //return false的时候回调直接关闭该连接；return true的时候正常结束本次回调
	request_state_t req_state; //请求是否完整的标志

	while (true) {
		switch (con_state) {
			case CON_STATE_CONNECT:
				ResetConnection();
				Worker::CloseCon(this);
				return true;

			case CON_STATE_REQUEST_START:    //监听读端口可读事件发生时进入该状态，进行读入之前的准备工作。正常结束进入READ状态进行读操作，否则出错。
				http_response.ResetResponse();
				//++con_req_cnt;
				SetState(CON_STATE_READ);
				break;                            //这次回调继续在状态机中进行，运行到下一个状态，其实这里直接返回true不就好了？

			case CON_STATE_READ:        //从上一状态进入，负责读事件的监听绑定。
				uv_read_start((uv_stream_t *) (this->tcp_conn), Worker::alloc_buffer, Connection::ConReadCallback);
				return true;

			case CON_STATE_READ_END: { //本次读端口完成，测试缓冲区中的请求是否完整，若不完整继续回到CON_STATE_READ，否则进入CON_STATE_RESPONSE_START状态。

				req_state = GetParsedRequest(); //从request队列中取到已经解析完成的request，若不存在则直接调用http_parser进行解析，看是否成功
				if (req_state == REQ_ERROR) {
					SetState(CON_STATE_ERROR);
					break;
				} else if (req_state == REQ_IS_COMPLETE) {
					SetState(CON_STATE_RESPONSE_START);
					break;                    //已经读到数据，直接转换状态进行下一状态的变化
				} else {                      //req_state == REQ_NOT_COMPLETE这种情况说明这一次http request还没完成，继续处于CON_STATE_READ状态，等待回调函数，而当前回调流程结束
					//SetState(CON_STATE_READ);
					//break;
					return true;
				}                              //整个连接的绝大部分等待时间都停留在等待request数据读取阶段，并可能在这里暂停逻辑，等待事件回调

			}
				//完成上一步仅仅把request放入了req_queue队列里面，后面需要做的就是通过第三方web程序反馈响应！！！
			case CON_STATE_RESPONSE_START: {//开始response之前的准备工作！
				con_outbuf += http_response.GetResponse();  //把这次request的response写入缓冲区，之后就可以清除这次request的相关数据然后等待下一次request了
				SetState(CON_STATE_WRITE);
				break;
			}

			case CON_STATE_WRITE: {   //向端口写入数据
				write_req_t *req = (write_req_t *) malloc(sizeof(write_req_t));//执行回调函数的时候这个数据需要还存在
				req->req.data = this->tcp_conn;
				//用缓存中的起始地址和大小初始化写数据对象write_req_t
				char *buff_ = (char *) malloc(con_outbuf.size());
				snprintf(buff_, con_outbuf.size(),con_outbuf.data());
				req->buf = uv_buf_init(buff_, con_outbuf.size());
				//写数据，并将写数据对象write_req_t和客户端、缓存、回调函数关联，第四个参数表示创建一个uv_buf_t缓存，不是1个字节
				uv_write((uv_write_t *) req, (uv_stream_t *) this->tcp_conn, &req->buf, 1, Connection::ConWriteCallback);
				SetState(CON_STATE_READ_END);
				break;                    //已经读到数据，直接转换状态进行下一状态的变化
				//return true;
			}

			case CON_STATE_RESPONSE_END: {  //写入完毕，检查缓冲区是否有完整请求，有则继续处理，否则进入读状态。
				SetState(CON_STATE_READ_END);
				if (con_finished) SetState(CON_STATE_ERROR);
				break;
			}

			case CON_STATE_ERROR: {   //Connection出错或者连接关闭进入该状态，清除所有连接资源，释放该连接。
				//Worker::CloseCon(this);
//				uv_shutdown_t * shutd = (uv_shutdown_t *)malloc(sizeof(uv_shutdown_t));
//				shutd->data = this->tcp_conn;
//				assert(0 == uv_shutdown(shutd, (uv_stream_t*)tcp_conn, Connection::ConShutdownCallback));
				uv_close((uv_handle_t*) tcp_conn,Connection::ConCloseCallback);
				return true;
			}
			default:
				return false;
		}
	}

	return true;
}


void Connection::SetState(connection_state_t state) {
	con_state = state;
}

void Connection::ResetConnection()                //重置连接，清空request队列，request计数重置为0
{
	http_response.ResetResponse();
	while (!req_queue.empty())
		req_queue.pop();
	con_req_cnt = 0;
}

void Connection::PrepareResponse() {
	http_response.http_code = 200;
	http_response.http_phrase = "ok";
	http_response.http_body = "<html><body>hello</body></html>";
}

void Connection::SetErrorResponse() {
	http_response.http_code = 500;
	http_response.http_phrase = "Server Error";
}

request_state_t Connection::GetParsedRequest()          //得到已经解析好的请求
{
	if (!req_queue.empty()) {
		http_req_parsed = req_queue.front();            //从请求队列req_queue里面取出一个请求赋值给http_req_parsed开始处理
		req_queue.pop();
		return REQ_IS_COMPLETE;
	}

	int ret = http_parser.HttpParseRequest(con_inbuf);  //如果request队列为空，则调用http_parser开始处理，返回的是处理的字节数

	if (ret == -1) {
		return REQ_ERROR;
	}

	if (ret == 0)      //读取当空串或者不完整的field-value对时，http-parser解析会返回0     指的是request不完整吧？
	{
		return REQ_NOT_COMPLETE;
	}

	con_inbuf.erase(0, ret);                   //若解析成功，删除已经经过解析的缓冲区数据

	if (!req_queue.empty())                    //http_parser处理完成的时候req_queue应该已经有数据了
	{
		http_req_parsed = req_queue.front();
		req_queue.pop();
		return REQ_IS_COMPLETE;
	}

	return REQ_NOT_COMPLETE;
}
