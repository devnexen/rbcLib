#include "rbcserial.hpp"
#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <dlfcn.h>

#define EXIT_CMD		127
#define FAIL_PLUG(eventname)	std::cerr << "Cannot load " << eventname << " plugin symbols" << std::endl; \
				dlclose(plugin); \
				return -1;

#if defined(_WIN32) || defined(_WIN64)
#define LIB_EXTENSION	".dll"
#elif defined(__unix__)
#if defined(__APPLE__) || defined(__MACH__)
#define LIB_EXTENSION	".dylib"
#else
#define LIB_EXTENSION	".so"
#endif
#endif

typedef int (* menu_t)(void);
typedef void (* start_t)(RbcSerial &, std::ostream &);
typedef void (* stop_t)(RbcSerial &, std::ostream &);
typedef void (* task_t)(RbcSerial &, int, int, char **, std::ostream &);

void usage(void) {
	std::cout << "./main <config> <pluginname> (argv1...argvn)" << std::endl;
}

void read_config(const std::string & config_file, std::string & tty, std::string & pluginsfolder) {
	boost::property_tree::ptree p;
	
	try {
		boost::property_tree::ini_parser::read_ini(config_file.c_str(), p);
		tty = p.get<std::string>("TTY");
		pluginsfolder = p.get<std::string>("PLUGINSFOLDER");
	} catch(std::exception & ex) {
		throw ex.what();
	}
}

int main(int argc, char ** argv) {
	if(argc < 3) {
		usage();
		return -1;
	}

	const std::string configfile = argv[1];
	std::string tty, pluginsfolder;

	try {
		read_config(configfile, tty, pluginsfolder);
	} catch(const char * msg) {
		std::cerr << "Config ini file : " << msg << std::endl;
		return -1;
	}

	std::string pluginpath = pluginsfolder;
	pluginpath.append(boost::filesystem::path("/").native());
	pluginpath.append(argv[2]);
	pluginpath.append(LIB_EXTENSION);

	int speed = 115200;
	RbcSerial r(tty, speed);
	
	std::cout << "Tty : " << tty << std::endl;
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

	std::cout << "Plugin loaded : " << pluginpath << std::endl;

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
