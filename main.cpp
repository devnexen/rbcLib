#include "rbcserial.hpp"
#include <dlfcn.h>

#define EXIT_CMD		127
#define FAIL_PLUG(eventname)	std::cerr << "Cannot load " << eventname << " plugin symbols" << std::endl; \
				dlclose(plugin); \
				return -1;

typedef int (* menu_t)(void);
typedef void (* start_t)(RbcSerial &, std::ostream &);
typedef void (* stop_t)(RbcSerial &, std::ostream &);
typedef void (* task_t)(RbcSerial &, int, int, char **, std::ostream &);

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
	menu_t menuhandle = (menu_t) dlsym(plugin, "menu");
	
	if(!menuhandle) {
		FAIL_PLUG("menu")
	}

	task_t taskhandle = (task_t) dlsym(plugin, "task");

	if(!taskhandle) {
		FAIL_PLUG("task")
	}

	start_t starthandle = (start_t) dlsym(plugin, "start");

	if(!starthandle) {
		FAIL_PLUG("start")
	}

	stop_t stophandle = (stop_t) dlsym(plugin, "stop");

	if(!stophandle) {
		FAIL_PLUG("stop")
	}

	argc --;
	argv ++;

	starthandle(r, std::cout);

	while(1) {
		int c;
		if((c = menuhandle()) == EXIT_CMD) {
			std::cout << "Exit..." << std::endl;
			break;
		}

		taskhandle(r, c, argc, argv, std::cout);
	}

	stophandle(r, std::cout);

	dlclose(plugin);

	return 0;
}
