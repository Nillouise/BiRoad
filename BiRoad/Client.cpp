#include"Client.h"
#include "World.h"
#include <asio/impl/connect.hpp>
#include <boost/bind/bind.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/impl/write.hpp>
#include <iostream>
#include <boost/bind/bind.hpp>
#include <asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include "Tool.h"
#include "Constant.h"
using namespace asio;
using namespace asio::ip;
using std::map;


void Client::init()
{
	isInit = false;
	isDown = false;

//	tcp::resolver resolver(ioService);
	tcp::endpoint endpoint(asio::ip::address_v4::from_string(ip), port);
	//fixme:客户端connect到底应不应该用阻塞式？
//	asio::connect(socket, endpoint);
	socket.connect(endpoint);
	timer.async_wait(boost::bind(&Client::timer_handler, this, asio::placeholders::error));
	asio::async_read_until(socket, recvbuf, '\n',
		boost::bind(&Client::firstReceive, shared_from_this(),
			asio::placeholders::error,
			asio::placeholders::bytes_transferred));
//	ioService.run();
}


bool Client::send(string msg)
{
	try
	{
		sendDataBuff = std::move(msg);
		Tool::newlineEnd(sendDataBuff);
	}
	catch (...)
	{
		std::cout << "client send msg error" << std::endl;
	}
	//这里应该不需要异步写，因为client这里本来就是开了个新线程的。
	asio::async_write(socket, asio::buffer(sendDataBuff), boost::bind(&Client::handle_write, shared_from_this(),
		asio::placeholders::error, asio::placeholders::bytes_transferred));
	return true;
}


bool Client::recv(const asio::error_code& err, size_t size)
{
	if (err)
	{
		isDown = true;
		return false;
	}

	std::istream is(&recvbuf);
	std::string s;
	std::getline(is, s);

	recvMsgMutex.lock();
	try
	{
		recvMsg += s+'\n';
	}catch(...)
	{
		std::cout << "client recv msg error" << std::endl;
	}
	std::cout << "client recv:" << recvMsg << std::endl;
	recvMsgMutex.unlock();
	asio::async_read_until(socket, recvbuf, '\n',
		boost::bind(&Client::firstReceive, shared_from_this(),
			asio::placeholders::error,
			asio::placeholders::bytes_transferred));
	return true;
}


void Client::firstReceive(const asio::error_code& err, size_t size)
{
	if(err)
	{
		isDown = true;
		return;
	}

	std::istream is(&recvbuf);
	std::string s;
	std::getline(is, s);

	map<string, string> kv = Tool::deserial_item_map(s);
	initData.insert(kv.begin(), kv.end());
	if(kv.find(Constant::GameMsg::isFinishInitMsg)!=kv.end())
	{
		isInit = true;
		asio::async_read_until(socket, recvbuf, '\n',
			boost::bind(&Client::recv, shared_from_this(),
				asio::placeholders::error,
				asio::placeholders::bytes_transferred));
		return;
	}else
	{
		asio::async_read_until(socket, recvbuf, '\n',
			boost::bind(&Client::firstReceive, shared_from_this(),
				asio::placeholders::error,
				asio::placeholders::bytes_transferred));
	}
}


string Client::getRecvMsg(bool clear)
{
	string res;
	recvMsgMutex.lock();
	try
	{
		res = recvMsg;
		if(clear)
		{
			recvMsg = "";
		}
	}catch(...)
	{
	}
	recvMsgMutex.unlock();
	return res;
}

void Client::timer_handler(const asio::error_code&)
{
	std::cout << "tiemr " << time(0) << std::endl;
	timer.expires_at(timer.expires_at() + boost::posix_time::seconds(5));
	timer.async_wait(boost::bind(&Client::timer_handler, this, asio::placeholders::error));
}
