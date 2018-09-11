#ifndef _MASTER_H
#define _MASTER_H

#include "header.h"
#include "config.h"
#include "worker.h"

class Master {
public:

    Master();

    ~Master();

    int StartMaster(int argc, char *argv[]);

    Config conf_para;

private:

    Worker m_worker;                //每个Master类里面有一个Worker类，fork之后Worker类负责本进程正常工作？

    int nums_of_child;
};

#endif
