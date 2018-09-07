#include "header.h"


int main(int argc, char *argv[]) {
	Master *master = new Master();
    return master->StartMaster(argc, argv);
}
