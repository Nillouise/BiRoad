#include "Server.h"
#include <iostream>
#include <asio/ip/tcp.hpp>
#include <asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <utility>
#include <vector>
#include "Point.h"
#include "Constant.h"
#include "Tool.h"
#include <boost/bind/bind.hpp>
using namespace asio;
using namespace asio::ip;
using std::cerr;
using asio::ip::tcp;
using std::vector;
using std::string;
using std::map;
using std::to_string;
class Scheduler;

class tcp_connection
	: public boost::enable_shared_from_this<tcp_connection>
{
public:
	typedef boost::shared_ptr<tcp_connection> pointer;


	int id = 0;
	int groupId = 0;
	shared_ptr<Scheduler> scheduler;
	Point initPoint;
	bool isInitPlayerMsg = false;


	static pointer create(asio::io_service& io_service)
	{
		return pointer(new tcp_connection(io_service));
	}


	bool send(const string &sendMsg)
	{
		string tmp = sendMsg;
		asio::async_write(socket_, asio::buffer(tmp),
			boost::bind(&tcp_connection::handle_write, shared_from_this(),
				asio::placeholders::error,
				asio::placeholders::bytes_transferred));
		return true;
	}


	tcp::socket& socket()
	{
		return socket_;
	}


	void start(map<string, string> initMsg)
	{
		asio::async_read_until(socket_, sbuf, '\n',
			boost::bind(&tcp_connection::handle_read, shared_from_this(),
				asio::placeholders::error,
				asio::placeholders::bytes_transferred));

		message_ = Tool::serial_map(initMsg);
		asio::async_write(socket_, asio::buffer(message_),
			boost::bind(&tcp_connection::handle_write, shared_from_this(),
				asio::placeholders::error,
				asio::placeholders::bytes_transferred));
	}
	

	void playerMsg(vector<string> playerMsg)
	{
		string res;
		for (auto &a:playerMsg)
		{
			res = res+a+'\n';
		}

		asio::async_write(socket_, asio::buffer(res),
			boost::bind(&tcp_connection::playerMsgHandler, shared_from_this(),
				asio::placeholders::error,
				asio::placeholders::bytes_transferred));
	}


	void playerMsgHandler(const asio::error_code& /*error*/,
		size_t /*bytes_transferred*/)
	{
		isInitPlayerMsg = true;
	}
private:
	asio::streambuf sbuf;
	tcp::socket socket_;
	std::string message_;
	std::string recv_message;

	tcp_connection(asio::io_service& io_service)
		: socket_(io_service)
	{
	}


	void handle_write(const asio::error_code& /*error*/,
		size_t /*bytes_transferred*/)
	{

	}


	void handle_read(const asio::error_code &err, size_t size)
	{
		if (err)
		{
			return;
		}

		//sbuf.commit(size);
		std::istream is(&sbuf);
		std::string s;
		std::getline(is, s);
		std::cout << "size: " << s.size() << std::endl << s << std::endl;
		asio::async_read_until(socket_, sbuf, '\n',
			boost::bind(&tcp_connection::handle_read, shared_from_this(),
				asio::placeholders::error,
				asio::placeholders::bytes_transferred));
	}
};


//此类会定时发送消息给客户端
class Scheduler
{
public:
	Scheduler(vector<tcp_connection::pointer> clients) :
		clients(std::move(clients)) {}

	int start();

private:
	vector<tcp_connection::pointer> clients;
};


class tcp_server
{
public:
	tcp_server(asio::io_service& io_service, int port,
		int targetClientsNumb, int curClientsNumb = 0)
		: acceptor_(io_service, tcp::endpoint(tcp::v4(), port)),
		targetClientsNumb(targetClientsNumb),
		curClientsNumb(curClientsNumb)
	{
		start_accept();
	}
	int width;
	int height;
	int pxWidth;

private:
	void start_accept()
	{
		tcp_connection::pointer new_connection =
			tcp_connection::create(acceptor_.get_io_service());

		acceptor_.async_accept(new_connection->socket(),
			boost::bind(&tcp_server::handle_accept, this, new_connection,
				asio::placeholders::error));
	}


	void handle_accept(tcp_connection::pointer new_connection,
		const asio::error_code& error)
	{
		if (!error)
		{
			new_connection->id = curClientsNumb;
			new_connection->groupId = connectionsSeed;
			new_connection->start(
			{
					{ Constant::GameMsg::selfId,to_string(new_connection->id) },
					{ Constant::GameMsg::width,to_string(width)},
					{ Constant::GameMsg::height, to_string(height) },
					{ Constant::GameMsg::pxWidth,to_string(pxWidth) }
			}
			);

			connections[connectionsSeed].push_back(new_connection);
			curClientsNumb++;
			if (connections[connectionsSeed].size() == targetClientsNumb)
			{
				startGameMsg(connectionsSeed);
				connectionsSeed++;
			}
		}

		start_accept();
	}


	void startGameMsg(int xgroup)
	{
		//这里设置每条connect的数据
		for (auto &a : connections[xgroup])
		{
			a->initPoint = Point(a->id * 2 + 2, a->id * 2 + 2);
		}

		vector<string> totalData;
		for (auto &a : connections[xgroup])
		{
			map<string, string> but;
			but[Constant::GameMsg::snakeId] = to_string(a->id);
			but[Constant::GameMsg::pointC] = to_string(a->initPoint.c);
			but[Constant::GameMsg::pointR] = to_string(a->initPoint.r);

			totalData.push_back(Tool::serial_map(but));
		}
		totalData.push_back(Tool::serial_map({ Constant::GameMsg::randomSeed ,to_string(engine())}));
		totalData.push_back(Tool::serial_map( { Constant::GameMsg::isFinishInitMsg,Constant::bool_true }));
		shared_ptr<Scheduler> scheduler = make_shared<Scheduler>(connections[xgroup]);
		for (auto &conn : connections[xgroup])
		{
			conn->scheduler = scheduler;
			conn->playerMsg(totalData);
		}
		scheduler->start();
	}

	std::default_random_engine engine = std::default_random_engine(2222);
	int connectionsSeed = 0;
	//FIXME：这种数据结构不适合并行呀
	map<int,vector<tcp_connection::pointer>> connections;
	tcp::acceptor acceptor_;
	int targetClientsNumb = 0;
	int curClientsNumb = 0;
};


int Server::init(int height, int width, int pxWidth)
{
	try
	{
		tcp_server s(io_service, port, targetClientsNumb);
		s.height = height;
		s.width = width;
		s.pxWidth = pxWidth;
		std::cout << "server init successful" << std::endl;
		io_service.run();
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return 1;
	}
	return 0;
}
