#include "header.h"
#include "master.h"
Master::Master() {
	nums_of_child = 0;
}

Master::~Master() {}

int Master::StartMaster(int argc, char *argv[]) {
	if (0 != conf_para.InitPara(argc, argv))                    //master做的第一步就是获取程序运行所需配置值，将其存储在Master类的conf_para变量中
		return -2;

	std::cout << "Start Master" << std::endl;

	if (conf_para.Daemonize) daemon(1, 0);

	if (!m_worker.Init(this)) {       //在fork之前完成了各个子进程的listener的初始化工作，后续只需要对同一个socket进行监听
		std::cerr << "Master: Worker::Init()" << std::endl;
		return false;
	}

	nums_of_child = conf_para.MaxWorker;
	//创建一定数量的worker
	m_worker.Run();
//	while (1) {
//
//		while (nums_of_child > 0) {
//			switch (fork()) {
//				case -1:
//					std::cerr << "Master: StartMaster(): fork()" << std::endl;
//					return false;
//				case 0:
//					m_worker.Run();
//					return true;
//				default:
//					//childs.insert(pid);
//					--nums_of_child;
//					break;
//			}
//		}
//
//		//atexit(Master::exitfunc);	//atexit函数是在程序正常调用exit函数时才会调用，收到SIGKILL信号不会正常调用。
//		int status;
//		if (wait(&status) != -1) {//接收到子进程退出信号在此fork一个工作进程
//			++nums_of_child;
//		}
//	}
}


