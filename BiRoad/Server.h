#pragma once
#include <asio/io_service.hpp>

class Server
{
public:
	int port = 0;
	//�м���client���ӲŻῪʼ������Ϸ
	int targetClientsNumb = 0;
	int curClientsNumb = 0;
	int init();
	asio::io_service io_service;
	Server(int port,int targetClientsNumb):
		port(port),targetClientsNumb(targetClientsNumb){}
};
