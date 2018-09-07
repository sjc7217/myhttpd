
#include "header.h"

#ifndef _WORKER_H
#define _WORKER_H

class Master;
class Connection;
class Plugin;

class Worker {
public:
    static uv_loop_t *loop;
	Worker();
	~Worker();
	bool Init(Master *master);
	void Run();
	Connection *NewCon();
	static void CloseCon(Connection *con);
    static void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf);
    static void close_cb(uv_handle_t *handle);

public:
    typedef std::map<uv_tcp_t *, Connection *> ConnectionMap;
	ConnectionMap w_con_map;
    Master *w_master;                                    //每个worker存在指向其master指针
    //TCP服务端对象
    uv_tcp_t tcp_server_listen;    //worker监听流
    static Listener *w_listener;                //每个worker里面有一个Listener，负责监听事件，在初始化的时候就绑定了
private:

	void InitConPool();
	Connection *GetFreeCon();
    typedef std::vector<Connection *> con_pool_t;    //每个worker有一个连接池，用于管理已经存在的连接（http1.1/keep alive情况下）
    con_pool_t con_pool;        //指针Vector，连接池
    int con_pool_size;            //连接池的大小，根据配置文件设定
    int con_pool_cur;            //当前连接池工作连接数量
};

#endif
