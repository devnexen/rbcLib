#include "rbcserial.hpp"
#include <dlfcn.h>

typedef void (* task_t)(RbcSerial &, char **, std::ostream &);

void usage(void) {
	std::cout << "./main <port> <pluginname> (argv1...argvn)" << std::endl;
}

int main(int argc, char ** argv) {
	if(argc < 3) {
		usage();
		return -1;
	}

	const std::string tty = argv[1];
	const std::string pluginpath = argv[2];

	int speed = 115200;

	RbcSerial r(tty, speed);
	
	std::cout << "Serial Number : " << r.sn() << std::endl;
	std::cout << "Firmware version : " << r.fw() << std::endl;

	void * plugin = dlopen(pluginpath.c_str(), RTLD_LAZY);

	if(!plugin) {
		std::cerr << "Cannot load plugin " << dlerror() << std::endl;
		return -1;
	}

	dlerror();
	task_t funchandle = (task_t) dlsym(plugin, "task");

	if(!funchandle) {
		std::cerr << "Cannot load plugin symbols " << std::endl;
		dlclose(plugin);
		return -1;
	}

	funchandle(r, argv, std::cout);

	dlclose(plugin);

	return 0;
}
