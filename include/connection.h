#ifndef _CONNECTION_H
#define _CONNECTION_H

#include "header.h"
#include "http.h"
#include "worker.h"


typedef enum {
	CONNECTION_BUSY,
	CONNECTION_IDLE,
	CONNECTION_ADD
} connection_on_use;

typedef struct {
	uv_write_t req;
	uv_buf_t buf;
} write_req_t;


typedef enum {
	CON_STATE_CONNECT,
	CON_STATE_REQUEST_START,
	CON_STATE_READ,
	CON_STATE_READ_END,
	CON_STATE_HANDLE_REQUEST,
	CON_STATE_RESPONSE_START,
	CON_STATE_WRITE,
	CON_STATE_RESPONSE_END,
	CON_STATE_ERROR
} connection_state_t;

typedef enum {
	REQ_ERROR,
	REQ_IS_COMPLETE,
	REQ_NOT_COMPLETE
} request_state_t;

class Worker;

class Connection {
public:

	Connection();
	~Connection();
	bool InitConnection(Worker *worker);
	void ResetCon();
	bool StateMachine();

public:

	uv_tcp_t *tcp_conn;
	typedef std::queue<struct HttpRequestContent *> req_queue_t; //connection内部对于request的缓冲，采用了堆空间，记得析构

	Worker *con_worker;
	req_queue_t req_queue;
	struct HttpRequestContent *http_req_parser;   //解析时用
	struct HttpRequestContent *http_req_parsed;   //处理请求时用
	struct HttpResponseContent http_response;
	connection_on_use con_use;	//用于表示当前连接状态(正在使用,空闲,或者是新建连接,不在连接池)
	//static void ConEventCallback(uv_os_fd_t fd, short event, void *arg);

	static void ConReadCallback(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf);
	static void ConWriteCallback(uv_write_t *req, int status);
	static void ConShutdownCallback(uv_shutdown_t* req, int status);
	static void ConCloseCallback(uv_handle_t* handle);
	//void (*uv_read_cb)(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf)

private:

	void ResetConnection();
	void PrepareResponse();
	void SetErrorResponse();
	void SetState(connection_state_t state);
	request_state_t GetParsedRequest();

private:

	int con_req_cnt;
	HttpParser http_parser;
	std::string con_inbuf;             //三个缓冲区
	std::string con_outbuf;
	connection_state_t con_state;
	request_state_t req_state;
	bool con_finished;

};

#endif
