#include "Starter.h"
#include <fstream>
#include <map>
#include <vector>
#include "Tool.h"

int stoint(string ori,int defaultval=0)
{
	try
	{
		return std::stoi(ori);
	}catch(...)
	{
		return defaultval;
	}
}

int Starter::init()
{
	std::ifstream file("setting.ini");
	try
	{
		std::string line;
		std::map<std::string, std::string> bucket;
		if (file.is_open())
		{
			while (std::getline(file, line))
			{
				std::vector<std::string> vec = Tool::split(line, "=");
				if (vec.size() >= 2)
				{
					bucket[vec[0]] = vec[1];
				}
			}
			file.close();
		}

		startServer = bucket["isSetMyComputerToServer"] == "true";
		remoteServerIP = bucket["remoteServerIP"];
		remoteServerPort = stoint(bucket["remoteServerPort"]);
		robotStart = bucket["robotStart"] == "true";
		openPort = stoint(bucket["remoteServerPort"]);
		groupSize = stoint(bucket["groupeSize"], 1);
		title = "biroad";
		xpos = 100;
		ypos = 100;
		width = 500;
		height = 500;
		fullScreen = false;
		pxSize = 20;
	}
	catch (...)
	{
		file.close();
		return 1;
	}
	return 0;
}
