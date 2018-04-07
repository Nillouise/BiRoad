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
#include <mutex>
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
	std::map<string, string> recv_map;
	Point initPoint;
	bool isInitPlayerMsg = false;
	std::mutex lock;

	static pointer create(asio::io_service& io_service)
	{
		return pointer(new tcp_connection(io_service));
	}


	bool send(const string &sendMsg)
	{
		this->sendMsgBuff = sendMsg;
		Tool::newlineEnd(sendMsgBuff);
		asio::async_write(socket_, asio::buffer(this->sendMsgBuff),
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
		Tool::newlineEnd(message_);
		asio::async_write(socket_, asio::buffer(message_),
			boost::bind(&tcp_connection::handle_write, shared_from_this(),
				asio::placeholders::error,
				asio::placeholders::bytes_transferred));
	}


	void sendPlayerMsg(vector<string> playerMsg)
	{
		sendPlayerMsgBuff = "";
		for (auto &a : playerMsg)
		{
			sendPlayerMsgBuff = sendPlayerMsgBuff + a + '\n';
		}
		Tool::newlineEnd(sendPlayerMsgBuff);
		std::cout << "sendPlayerMsgBuff: " << sendPlayerMsgBuff << std::endl;
		asio::async_write(socket_, asio::buffer(sendPlayerMsgBuff),
			boost::bind(&tcp_connection::playerMsgHandler, shared_from_this(),
				asio::placeholders::error,
				asio::placeholders::bytes_transferred));
	}


	void playerMsgHandler(const asio::error_code& /*error*/,
		size_t bytes_transfer/*bytes_transferred*/)
	{
		isInitPlayerMsg = true;
		std::cout << "bytes_transfer" << bytes_transfer << std::endl;
	}
private:
	asio::streambuf sbuf;
	string sendMsgBuff;
	string sendPlayerMsgBuff;
	tcp::socket socket_;
	std::string message_;
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
		lock.lock();
		try
		{
			//sbuf.commit(size);
			std::istream is(&sbuf);
			std::string s;
			std::getline(is, s);
			recv_map = Tool::deserial_item_map(s);


			asio::async_read_until(socket_, sbuf, '\n',
				boost::bind(&tcp_connection::handle_read, shared_from_this(),
					asio::placeholders::error,
					asio::placeholders::bytes_transferred));
		}
		catch (...)
		{
			std::cerr << "client tcp connection " << id << " error" << std::endl;
			throw;
		}
		lock.unlock();
	}
};


//此类会定时发送消息给客户端
class Scheduler
{
public:
	Scheduler(vector<tcp_connection::pointer> clients, asio::io_service& io, int currentFrame) :
		clients(std::move(clients)), timer_(io, gap), currentFrame(currentFrame) {}

	int start();
	boost::posix_time::seconds gap = boost::posix_time::seconds(1);

private:
	vector<tcp_connection::pointer> clients;
	asio::deadline_timer timer_;
	int currentFrame = 0;
	void scheduleSend(const asio::error_code& /*error*/);
};


int Scheduler::start()
{
	timer_.async_wait(boost::bind(&Scheduler::scheduleSend, this, asio::placeholders::error));
	return 0;
}

void Scheduler::scheduleSend(const asio::error_code& err)
{
	if (err)
	{
		return;
	}
	string res;

	for (auto &a : clients)
	{
		a->lock.lock();
		try
		{
			if (a->recv_map[Constant::GameMsg::timeStamp] == to_string(currentFrame))
			{
				res += Tool::serial_map(a->recv_map) + '\n';
			}
		}
		catch (...)
		{
			cerr << "Scheduler::scheduleSend error" << std::endl;
		}
		a->lock.unlock();
	}
	res += Tool::serial_map({ {Constant::GameMsg::isFrameFinish,Constant::bool_true } });
	Tool::newlineEnd(res);
	for (auto &a : clients)
	{
		a->send(res);
	}
	currentFrame++;
	timer_.expires_at(timer_.expires_at() + gap);
	timer_.async_wait(boost::bind(&Scheduler::scheduleSend, this, asio::placeholders::error));
}



class tcp_server
{
public:
	tcp_server(asio::io_service& io_service, int port,
		int targetClientsNumb, int curClientsNumb = 0)
		: io(io_service),
		acceptor_(io_service, tcp::endpoint(tcp::v4(), port)),
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
		std::cout << "server accept new connection" << std::endl;
		if (!error)
		{

			new_connection->id = curClientsNumb;
			new_connection->groupId = connectionsSeed;
			//发出地图信息
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
				sendStartGameMsg(connectionsSeed);
				connectionsSeed++;
			}
		}

		start_accept();
	}


	void sendStartGameMsg(int xgroup)
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
			but[Constant::GameMsg::objType] = Constant::GameMsg::snakeType;
			totalData.push_back(Tool::serial_map(but));
		}
		totalData.push_back(Tool::serial_map({
			{ Constant::GameMsg::objType ,Constant::GameMsg::ballType },
			{Constant::GameMsg::pointC,to_string(1)},
			{Constant::GameMsg::pointR,to_string(5)}
		}));
		totalData.push_back(Tool::serial_map({ { Constant::GameMsg::randomSeed ,to_string(engine())} }));
		totalData.push_back(Tool::serial_map({ {Constant::GameMsg::isFinishInitMsg,Constant::bool_true } }));
		shared_ptr<Scheduler> scheduler = make_shared<Scheduler>(connections[xgroup], io, 0);
		//发出每个客户端各自的信息
		for (auto &conn : connections[xgroup])
		{
			conn->scheduler = scheduler;
			conn->sendPlayerMsg(totalData);
		}
		scheduler->start();
	}


	std::default_random_engine engine = std::default_random_engine(2222);
	int connectionsSeed = 0;
	map<int, vector<tcp_connection::pointer>> connections;
	io_service &io;
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
