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

	tcp::resolver resolver(ioService);
	tcp::endpoint endpoint(asio::ip::address_v4::from_string(ip), port);
	asio::connect(socket, endpoint);

	asio::async_read_until(socket, recvbuf, '\n',
		boost::bind(&Client::firstReceive, shared_from_this(),
			asio::placeholders::error,
			asio::placeholders::bytes_transferred));
}


bool Client::send()
{
	string sendData;
	sendMsgMutex.lock();
	try
	{
		sendData = sendMsg;
	}
	catch (...)
	{
		std::cout << "client send msg error" << std::endl;
	}
	sendMsgMutex.unlock();

	asio::async_write(socket, asio::buffer(sendData), nullptr);
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
		recvMsg = s;
	}catch(...)
	{
		std::cout << "client recv msg error" << std::endl;
	}
	recvMsgMutex.unlock();
	return true;
}


bool Client::firstReceive(const asio::error_code& err, size_t size)
{
	if(err)
	{
		isDown = true;
		return false;
	}

	std::istream is(&recvbuf);
	std::string s;
	std::getline(is, s);

	map<string, string> kv = Tool::deserial_item_map(s);
	initData.insert(kv.begin(), kv.end());
	if(kv.find(Constant::GameMsg::isInitMsg)!=kv.end())
	{
		isInit = true;
		asio::async_read_until(socket, recvbuf, '\n',
			boost::bind(&Client::recv, shared_from_this(),
				asio::placeholders::error,
				asio::placeholders::bytes_transferred));
		return true;
	}


	asio::async_read_until(socket, recvbuf, '\n',
		boost::bind(&Client::firstReceive, shared_from_this(),
			asio::placeholders::error,
			asio::placeholders::bytes_transferred));

	return true;
}

string Client::getSendMsg()
{
	string res;
	sendMsgMutex.lock();
	try
	{
		res = sendMsg;
	}
	catch (...)
	{
	}
	sendMsgMutex.unlock();
	return res;
}

string Client::getRecvMsg()
{
	string res;
	recvMsgMutex.lock();
	try
	{
		res = recvMsg;
	}catch(...)
	{
	}
	recvMsgMutex.unlock();
	return res;
}
