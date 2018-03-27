#include <ctime>
#include <iostream>
#include <string>
#include <asio.hpp>
#include <boost/bind/bind.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/impl/write.hpp>
#include <iostream>
#include <boost/bind/bind.hpp>
#include <asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
using asio::ip::tcp;

std::string make_daytime_string()
{
	using namespace std; // For time_t, time and ctime;
	time_t now = time(nullptr);
	return ctime(&now);
}


class tcp_connection
	: public boost::enable_shared_from_this<tcp_connection>
{
public:
	typedef boost::shared_ptr<tcp_connection> pointer;

	static pointer create(asio::io_service& io_service)
	{
		return pointer(new tcp_connection(io_service));
	}

	tcp::socket& socket()
	{
		return socket_;
	}


	void start()
	{
		message_ = make_daytime_string();
		asio::async_read_until(socket_, sbuf, '\n',
			boost::bind(&tcp_connection::handle_read, shared_from_this(),
				asio::placeholders::error,
				asio::placeholders::bytes_transferred));

		asio::async_write(socket_, asio::buffer(message_),
			boost::bind(&tcp_connection::handle_write, shared_from_this(),
				asio::placeholders::error,
				asio::placeholders::bytes_transferred));
	}


private:
	tcp_connection(asio::io_service& io_service)
		: socket_(io_service)
	{}


	void handle_write(const asio::error_code& /*error*/,
		size_t /*bytes_transferred*/)
	{

	}


	void handle_read(const asio::error_code &err, size_t size)
	{
//		sbuf.commit(size);
		std::istream is(&sbuf);
		std::string s;
		std::getline(is, s);
		std::cout << "size: " << s.size() << std::endl << s << std::endl;
		asio::async_read_until(socket_, sbuf, '\n',
			boost::bind(&tcp_connection::handle_read, shared_from_this(),
				asio::placeholders::error,
				asio::placeholders::bytes_transferred));
	}
	asio::streambuf sbuf;
	tcp::socket socket_;
	std::string message_;
	std::string recv_message;
};

class tcp_server
{
public:
	tcp_server(asio::io_service& io_service, int port)
		: acceptor_(io_service, tcp::endpoint(tcp::v4(), port))
	{
		start_accept();
	}

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
			new_connection->start();
		}

		start_accept();
	}

	tcp::acceptor acceptor_;
};

int main()
{
	try
	{
		asio::io_service io_service;
		tcp_server server(io_service,3000);
		io_service.run();
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}

	return 0;
}