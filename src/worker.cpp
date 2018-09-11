#include "header.h"
#include "worker.h"
#include "listener.h"
#include "master.h"
#include "connection.h"
Worker::Worker() {
	w_master = nullptr;
    loop = uv_default_loop();
    w_listener = new Listener();
}

Worker::~Worker() {}

uv_loop_t *Worker::loop = uv_default_loop();
Listener *Worker::w_listener = nullptr;

void Worker::close_cb(uv_handle_t *handle) {
	std::cout << "close successfully!" << std::endl;
    uv_tcp_t *h = (uv_tcp_t *) handle;
    Connection *con = (Connection *) h->data;
    Worker::CloseCon(con);
}


void Worker::alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
    //分配接收缓存内存和设置建议大小，小于等于实际大小
    buf->base = (char *) malloc(suggested_size);
    buf->len = suggested_size;
}

bool Worker::Init(Master *master) {
	w_master = master;

    InitConPool();
    //初始化连接池
    uv_tcp_init(Worker::loop, &this->tcp_server_listen);
    if (!Worker::w_listener->InitListener(this)) {
		std::cerr << "Worker: Listener::InitListener()" << std::endl;
		return false;
	}
	return true;
}

void Worker::Run() {
    uv_run(loop, UV_RUN_DEFAULT);
}


void Worker::InitConPool() {
	con_pool_size = w_master->conf_para.InitConPool;
	con_pool_cur = 0;
	con_pool.resize(con_pool_size);

    for (int i = 0; i < con_pool_size; ++i) con_pool[i] = new Connection();    //初始化连接池
}

Connection *Worker::GetFreeCon() {
	Connection *con = nullptr;

    if (con_pool_cur < con_pool_size) {
        for (auto ptr:con_pool) {
            if (ptr->con_use == CONNECTION_IDLE) {
                ptr->con_use = CONNECTION_BUSY;
                return ptr;
            }
        }
    }
    return con;
}


void Worker::CloseCon(Connection *con) {    //关闭连接，从连接池中释放
	Worker *worker = con->con_worker;
	con->ResetCon();
	//uv_close((uv_handle_t *) con->tcp_conn, NULL);

    if (con->con_use == CONNECTION_BUSY) {//如果连接在连接池则只是重置之后放回池中
        con->con_use = CONNECTION_IDLE;
        worker->con_pool_cur--;
    } else if (con->con_use == CONNECTION_ADD) {//如果连接是多余的，直接删除，析构掉
        delete (con);
	}
	return;
}

Connection *Worker::NewCon() {
	//连接池获取一个空闲连接
	Connection *con = GetFreeCon();
	if (nullptr == con) {
		con = new Connection();    //连接池没有空闲的，分配一个
		con->con_use = CONNECTION_ADD;
	}
	return con;
}
