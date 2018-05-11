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

	//�տ�ʼ����Ϣ�Ƿ��ʼ�����
	bool isInit;
	//���������ǲ���down��
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
	void handle_connect(const asio::error_code& error/*error*/)
	{
		if (!error)
		{
			asio::async_read(socket,
				asio::buffer(read_msg_.data(), NetworkMsg::header_length), //��ȡ���ݱ�ͷ  
				bind(&Client::handle_read_header, shared_from_this(),
					asio::placeholders::error));
		}
	}


	void handle_read_header(asio::error_code& error)
	{
		if (!error && read_msg_.decode_header()) //�ֱ������ݱ�ͷ�����ݲ���  
		{
			asio::async_read(socket,
				asio::buffer(read_msg_.body(), read_msg_.body_length()),//��ȡ���ݰ����ݲ���  
				bind(&Client::handle_read_body, shared_from_this(),
					asio::placeholders::error));
		}
		else
		{
//			do_close();
		}
	}

	void handle_read_body(asio::error_code& error)
	{
		if (!error)
		{
			std::cout.write(read_msg_.body(), read_msg_.body_length()); //�����Ϣ  
			std::cout << "\n";
			asio::async_read(socket,
				asio::buffer(read_msg_.data(),	NetworkMsg::header_length), //�������ȡ��һ�����ݰ�ͷ  
				bind(&Client::handle_read_header, shared_from_this(),
					asio::placeholders::error)); //���һ�ζ�������������һ�����ݰ���  ������һ�ζ�����  
		}
		else
		{
//			do_close();
		}
	}

	Client(const string &ip, int port) :
		ioService(), ip(ip), port(port), socket(ioService)
	{
	}
	NetworkMsg read_msg_;
};
