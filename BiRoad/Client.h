#pragma once
#include <string>
#include <asio/io_service.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/streambuf.hpp>
#include <asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <mutex>
#include <deque>
#include "NetworkMsg.h"
#include <iostream>
#include "Constant.h"
#include <boost/bind/bind.hpp>

using std::string;

class Client
	: public boost::enable_shared_from_this<Client>
{
public:
	typedef boost::shared_ptr<Client> pClient;
	static pClient create(const string &ip, int port)
	{
		return pClient(new Client(ip, port));
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
	void handle_write(const asio::error_code& /*error*/,
		size_t /*bytes_transferred*/)
	{}
	void write(const NetworkMsg& msg)
	{
		ioService.post(
			[this, msg]()
		{
			bool write_in_progress = !write_msgs_.empty();
			write_msgs_.push_back(msg);
			if (!write_in_progress)
			{
				do_write();
			}
		});
	}

	void close()
	{
		ioService.post([this]() { socket.close(); });
	}
	void do_connect(asio::ip::tcp::resolver::iterator endpoint_iterator)
	{
		asio::async_connect(socket, endpoint_iterator,
			[this](std::error_code ec, asio::ip::tcp::resolver::iterator)
		{
			if (!ec)
			{
				do_read_header();
			}
		});
	}


	void do_read_header()
	{
		asio::async_read(socket,
			asio::buffer(read_msg_.data(), NetworkMsg::header_length),
			[this](std::error_code ec, std::size_t /*length*/)
		{
			if (!ec && read_msg_.decode_header())
			{
				do_read_body();
			}
			else
			{
				socket.close();
			}
		});
	}

	void do_read_body()
	{
		asio::async_read(socket,
			asio::buffer(read_msg_.body(), read_msg_.body_length()),
			[this](std::error_code ec, std::size_t /*length*/)
		{
			if (!ec)
			{
				if(!isInit)
				{
					//第一次被初始化
					tackleFirstReceive();
				}else
				{
					std::cout.write(read_msg_.body(), read_msg_.body_length());
					std::cout << "\n";
					tacklemsg(read_msg_);
				}

				do_read_header();
			}
			else
			{
				isDown = true;
				socket.close();
			}
		});
	}

	void tacklemsg(NetworkMsg &msg)
	{
		recvMsgMutex.lock();
		try
		{
			std::string s;
			for(int i=0;i<(int)msg.body_length();i++)
			{
				if(msg.body()[i]=='\n')
				{
					recvMsg.push_back(s);
					s = "";
				}else
				{
					s += msg.body()[i];
				}
			}
			if(!s.empty())
			{
				recvMsg.push_back(s);
			}
		}
		catch (...)
		{
			std::cout << "client recv msg error" << std::endl;
		}
		std::cout << "client recv:" << recvMsg.back() << std::endl;
		recvMsgMutex.unlock();
	}

	void tackleFirstReceive();

	void do_write()
	{
		asio::async_write(socket,
			asio::buffer(write_msgs_.front().data(),
				write_msgs_.front().length()),
			[this](std::error_code ec, std::size_t /*length*/)
		{
			if (!ec)
			{
				write_msgs_.pop_front();
				if (!write_msgs_.empty())
				{
					do_write();
				}
			}
			else
			{
				socket.close();
			}
		});
	}

	Client(const string &ip, int port) :
		ioService(), ip(ip), port(port), socket(ioService)
	{
	}
	NetworkMsg read_msg_;
	std::deque<NetworkMsg> write_msgs_;
};
