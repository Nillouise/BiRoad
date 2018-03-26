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
using namespace asio;
using namespace asio::ip;
using std::map;


void Client::init()
{
	asio::io_service io_service;

	tcp::resolver resolver(io_service);
//	tcp::resolver::query query(ip, "daytime");//这里的daytime就是用daytime这个服务的端口（因为daytime这个服务是个广为人知的协议，有自己的专门端口）
//	tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
	tcp::endpoint endpoint(asio::ip::address_v4::from_string(ip), port);
	asio::connect(socket, endpoint);
//	tcp::resolver r(io_service);
//	tcp::resolver::query query(ip,port );
//	tcp::resolver::iterator endpoint_iterator = static_cast<>(r) (query);
//	asio::connect(socket, endpoint_iterator);

}


bool Client::send()
{



}


bool Client::recv()
{


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

	map<string


	asio::async_read_until(socket, recvbuf, '\n',
		boost::bind(&Client::firstReceive, shared_from_this(),
			asio::placeholders::error,
			asio::placeholders::bytes_transferred));


}
