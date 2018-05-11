#pragma once
#include <string>
using std::string;
class Starter
{
public:
	string title;
	int xpos;
	int ypos; 
	int width;
	int height;
	bool fullScreen;
	int pxSize;
	int selfId;
	bool startServer;
	string remoteServerIP;
	int remoteServerPort;
	bool robotStart;
	int openPort;
	int groupSize;
	int init();
};


