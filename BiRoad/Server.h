#pragma once
#include <asio/io_service.hpp>

class Server
{
public:
	int port;
	int init();
	asio::io_service io_service;

};
