#pragma once
#include<httplib.h>
#include<thread>
#include"utils.h"

#define STATIC_FILES_PATH (getModuleDirectory()/"etstoolbox").string()

class ServerWrapper {
private:
	httplib::Server server;
public:
	ServerWrapper();
	void listen();
	void shutdown();
};
