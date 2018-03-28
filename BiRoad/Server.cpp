#include "Server.h"
#include <iostream>
#include <asio/ip/tcp.hpp>
#include <asio.hpp>
#include <boost/enable_shared_from_this.hpp>
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


class tcp_connection
	: public boost::enable_shared_from_this<tcp_connection>
{
public:
	typedef boost::shared_ptr<tcp_connection> pointer;


	int id = 0;
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
			new_connection->start(
			{
					{ Constant::GameMsg::selfId,to_string(new_connection->id) },
					{ Constant::GameMsg::width,to_string(width)},
					{ Constant::GameMsg::height, to_string(height) },
					{ Constant::GameMsg::pxWidth,to_string(pxWidth) }
			}
			);
			connections.back().push_back(new_connection);

			curClientsNumb++;
			if (curClientsNumb%targetClientsNumb == 0)
			{
				startGameMsg();

				connections.emplace_back();
			}
		}

		start_accept();
	}


	void startGameMsg()
	{
		//这里设置每条connect的数据
		for (auto &a : connections.back())
		{
			a->initPoint = Point(a->id * 2 + 2, a->id * 2 + 2);
		}

		vector<string> totalData;
		for (auto &a : connections.back())
		{
			map<string, string> but;
			but[Constant::GameMsg::snakeId] = to_string(a->id);
			but[Constant::GameMsg::pointC] = to_string(a->initPoint.c);
			but[Constant::GameMsg::pointR] = to_string(a->initPoint.r);

			totalData.push_back(Tool::serial_map(but));
		}
		map<string, string> seed;
		seed[Constant::GameMsg::randomSeed] = to_string(engine());
		totalData.push_back(Tool::serial_map(seed));

		for (auto &conn : connections.back())
		{
			for (auto &data : totalData)
			{
				conn->send(data);
			}
			conn->send(Tool::serial_map({ {Constant::GameMsg::isFinishInitMsg,Constant::bool_true} }));
		}
	}

	std::default_random_engine engine = std::default_random_engine(2222);
	vector<vector<tcp_connection::pointer>> connections = { vector<tcp_connection::pointer>() };
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
		io_service.run();
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}
	return 0;
}
