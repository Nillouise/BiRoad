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
	bool send(string msg);
	bool recv(const asio::error_code& err, size_t size);
	bool firstReceive(const asio::error_code& err, size_t size);

	//�տ�ʼ����Ϣ�Ƿ��ʼ�����
	bool isInit;
	//���������ǲ���down��
	bool isDown;
	asio::io_service ioService;
	asio::ip::tcp::socket socket;
	std::mutex recvMsgMutex;
	string getRecvMsg(bool clear = false);
	std::map<string, string> initData;
private:
	string recvMsg;
	string sendDataBuff;
	string ip;
	int port;
	asio::streambuf recvbuf;

	void handle_write(const asio::error_code& /*error*/,
		size_t /*bytes_transferred*/)
	{}
};
