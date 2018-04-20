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
#include <deque>

using std::string;

class Client
	: public boost::enable_shared_from_this<Client>
{
public:
	typedef boost::shared_ptr<Client> pClient;
	static pClient create(const string &ip, int port)
	{
		return pClient(new Client(ip,port));
	}
	Client() = delete;
	void init();
	bool send(string msg);
	bool recv(const asio::error_code& err, size_t size);
	void firstReceive(const asio::error_code& err, size_t size);

	//刚开始的信息是否初始化完成
	bool isInit;
	//网络连接是不是down了
	bool isDown;
	asio::io_service ioService;
	asio::ip::tcp::socket socket;
	std::mutex recvMsgMutex;
	string getFrameMsg(bool clear = false);
	std::vector<string> popFrameMsg(bool clear = false);
	std::map<string, string> initData;
private:
	std::deque<string> recvMsg;
	string sendDataBuff;
	string ip;
	int port;
	asio::streambuf recvbuf;
	asio::deadline_timer timer;
	void handle_write(const asio::error_code& /*error*/,
		size_t /*bytes_transferred*/)
	{}
	void timer_handler(const asio::error_code&);

	Client(const string &ip, int port) :
		ioService(), ip(ip), port(port), socket(ioService),timer(ioService, boost::posix_time::seconds(5))
	{
	}
};
