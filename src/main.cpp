#include "header.h"
#include "master.h"

int main(int argc, char *argv[]) {
	Master *master = new Master();
    return master->StartMaster(argc, argv);
}
