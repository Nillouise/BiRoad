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

//	tcp::resolver resolver(ioService);(
//	tcp::endpoint endpoint(asio::ip::address_v4::from_string(ip), port);
	tcp::resolver resolver(ioService);
	auto endpoint_iterator = resolver.resolve({ ip, std::to_string(port) });
	//fixme:客户端connect到底应不应该用阻塞式？
//	asio::connect(socket, endpoint);)
	//设置no delay	
	socket.open(tcp::v4());
	socket.set_option(tcp::no_delay(true));
	do_connect(endpoint_iterator);
//	socket.connect(endpoint);
//	asio::async_read_until(socket, recvbuf, '\n',
//		boost::bind(&Client::firstReceive, shared_from_this(),
//			asio::placeholders::error,
//			asio::placeholders::bytes_transferred));
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
	std::cout << "recv time \t:" << time(0) % 100000 << "\t size: " << size << std::endl;
	if (err)
	{
		isDown = true;
		return false;
	}
	recvMsgMutex.lock();
	try
	{
		std::istream is(&recvbuf);
		std::string s;
		size_t len = 0;
		while(std::getline(is, s)&&len<size)
		{
			recvMsg.push_back(s);
			len += s.size()+1;
		}
	}catch(...)
	{
		std::cout << "client recv msg error" << std::endl;
	}
	std::cout << "client recv:" << recvMsg.back() << std::endl;
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
	while(std::getline(is, s))
	{
		map<string, string> kv = Tool::deserial_item_map(s);
		initData.insert(kv.begin(), kv.end());
	}
	if(initData.find(Constant::GameMsg::isFinishConnectMsg)!= initData.end())
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


std::vector<string> Client::popFrameMsg(bool clear)
{
	std::vector<string> res;
	recvMsgMutex.lock();
	try
	{
		int isFinishedAFrame = 0;
		for(string &a:recvMsg)
		{
			res.push_back(a);
			if(a.find(Constant::GameMsg::isFrameFinish+"="+Constant::bool_true)!= std::string::npos)
			{
				isFinishedAFrame = 1;
				break;
			}
		}
		if(isFinishedAFrame)
		{
			while(!recvMsg.empty()&&clear)
			{
				if (recvMsg.front().find(Constant::GameMsg::isFrameFinish + "=" + Constant::bool_true) != std::string::npos)
				{
					recvMsg.pop_front();
					break;
				}else
				{
					recvMsg.pop_front();
				}
			}
		}else
		{
			res = {};
		}
	}catch(...)
	{
	}
	recvMsgMutex.unlock();
	return res;
}

void Client::tackleFirstReceive()
{
	//第一次被初始化
	string s;
	for (int i = 0; i<read_msg_.body_length(); i++)
	{
		if (read_msg_.body()[i] == '\n')
		{
			std::map<string, string> kv = Tool::deserial_item_map(s);
			initData.insert(kv.begin(), kv.end());
			s = "";
		}
		else
		{
			s += read_msg_.body()[i];
		}
	}
	if (!s.empty())
	{
		std::map<string, string> kv = Tool::deserial_item_map(s);
		initData.insert(kv.begin(), kv.end());
	}
	isInit = true;
}


string Client::getFrameMsg(bool clear)
{
	string res;
	recvMsgMutex.lock();
	try
	{
		for(auto &a:recvMsg)
		{
			res += a + '\n';
		}
		if(clear)
		{
			recvMsg.clear();
		}
	}catch(...)
	{
	}
	recvMsgMutex.unlock();
	return res;
}

