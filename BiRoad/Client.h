#pragma once
#include <string>
#include <asio/io_service.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/streambuf.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/impl/write.hpp>
#include <iostream>
#include <boost/bind/bind.hpp>
#include <asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <mutex>
using std::string;

class Client
	: public boost::enable_shared_from_this<Client>
{
public:
	Client(const string &ip, int port):
		ioService(),ip(ip), port(port),socket(ioService) {}
	void init();
	bool send();
	bool recv(const asio::error_code& err, size_t size);
	bool firstReceive(const asio::error_code& err, size_t size);

	//刚开始的信息是否初始化完成
	bool isInit;
	//网络连接是不是down了
	bool isDown;
	asio::io_service ioService;
	asio::ip::tcp::socket socket;
	std::mutex sendMsgMutex;
	string getSendMsg();
	std::mutex recvMsgMutex;
	string getRecvMsg();
	std::map<string, string> initData;
private:
	string sendMsg;
	string recvMsg;
	string ip;
	int port;
	asio::streambuf recvbuf;

	void handle_write(const asio::error_code& /*error*/,
		size_t /*bytes_transferred*/)
	{}
};
