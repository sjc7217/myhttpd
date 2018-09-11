#ifndef _LISTENER_H
#define _LISTENER_H

#include "header.h"
#include "connection.h"

class Worker;

class Listener {
public:

	Listener();

	~Listener();

	bool InitListener(Worker *worker);

	void AddListenEvent();

	static void ListenEventCallback(uv_stream_t *server, int status);//uv_os_fd_t fd, short event, void *arg);

public:
	static Worker *listen_worker;
	uv_os_fd_t listen_sockfd;
	//evutil_socket_t		 listen_sockfd;
	struct sockaddr_in listen_addr;
//		struct event		*listen_event;
	int listen_con_cnt;            //listen接收到的请求计数
};

#endif
