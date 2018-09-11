#include "header.h"
#include "listener.h"
#include "worker.h"
#include "master.h"
Listener::Listener() {
	listen_con_cnt = 0;
}

Listener::~Listener() {}
Worker *Listener::listen_worker = nullptr;

bool Listener::InitListener(Worker *worker) {
	Listener::listen_worker = worker;
	//改用libuv的接口设置监听地址
	const char *ad = worker->w_master->conf_para.ListenIP.c_str();
	int port_ = worker->w_master->conf_para.ListenPort;
	//创建IP地址和端口，服务器使用0.0.0.0代表任意地址，公网服务器使用，客户端可以通过公网IP连接。如果是局域网，则填写局域网IP。
	uv_ip4_addr(ad, port_, &listen_addr);
	//将服务端对象和地址绑定，供后续监听端口使用。
	uv_tcp_bind(&worker->tcp_server_listen, (const struct sockaddr *) &listen_addr, 0);
	//进行端口监听，同时关联监听后进来的连接的回调函数
	int r = uv_listen((uv_stream_t *) &worker->tcp_server_listen, 5/*backlog*/, Listener::ListenEventCallback);
	if(r) return false;
	return true;
}

void Listener::ListenEventCallback(uv_stream_t *server,
								   int status) {//evutil_socket_t sockfd, short event, void *arg)            //监听事件回调函数
	if (status < 0) {
		//新建连接出错
		fprintf(stderr, "New connection error %s\n", uv_strerror(status));
		return;
	}
	//为新的连接分配内存
	uv_tcp_t *client = (uv_tcp_t *) malloc(sizeof(uv_tcp_t));
	//将全局的主循环和处理连接的对象关联起来
	uv_tcp_init(Worker::loop, client);
	//接收服务端对象
	if (uv_accept(server, (uv_stream_t *) client) == 0) {
		//开始读取客户端发送的数据，并设置好接收缓存分配的函数alloc_buffer和读取完毕后的回调函数echo_read


//		uv_close((uv_handle_t*)client,NULL);
//		std::cout<<"close successful!"<<std::endl;
//		return;

		Connection *con = Listener::listen_worker->NewCon();
		if (con == nullptr) {
			std::cerr << "Listener::ListenEventCallback(): NewCon()" << std::endl;
			return;
		}
		client->data = con;                //将Connection指针放到uv_tcp_t的数据中，使得回调函数可以得到连接对象
		con->tcp_conn = client;            //每个Connection对象维护一个uv_tcp_t对象。
		if (!con->InitConnection(Listener::listen_worker)) {    //返回false表示连接建立失败
			std::cerr << "Listener::ListenEventCallback(): Connection::InitConnection()" << std::endl;
			Worker::CloseCon(con);
			return;
		}
		//con->con_worker->w_con_map[client] = con;
		++(Worker::w_listener->listen_con_cnt);
	} else {
		//读取失败，释放处理对象
		uv_close((uv_handle_t *) client, NULL);
	}

}
