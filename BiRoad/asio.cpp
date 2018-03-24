#include <iostream>
#include <asio.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
using std::cout;
using std::endl;
class Printer
{
public:
	Printer(asio::io_service &io) :
		count(0),
		strand_(io),
		timer1(io, boost::posix_time::seconds(1)),
		timer2(io, boost::posix_time::seconds(1))
	{
		timer1.async_wait(strand_.wrap(boost::bind(&Printer::print1, this)));
		timer2.async_wait(strand_.wrap(boost::bind(&Printer::print2, this)));
	}
	void print1()
	{
		if (count < 15)
		{
			count++;
			cout <<"call timer1: "<< count << endl;
			timer1.expires_at(timer1.expires_at() + boost::posix_time::seconds(1));
			timer1.async_wait(strand_.wrap(boost::bind(&Printer::print1, this)));//成员函数有个隐藏的参数this吧
		}
	}

	void print2()
	{
		if (count < 15)
		{
			count++;
			cout << "call timer2: " << count << endl;
			timer2.expires_at(timer2.expires_at() + boost::posix_time::seconds(1));
			timer2.async_wait(strand_.wrap(boost::bind(&Printer::print2, this)));
		}
	}

	~Printer()
	{
		cout << "Printer finished" << endl;
	}

private:
	int count;
	asio::io_service::strand strand_;
	asio::deadline_timer timer1;
	asio::deadline_timer timer2;
};

static int local_main()
{
	asio::io_service io;
	Printer p(io);
	asio::thread t(boost::bind(&asio::io_service::run, &io));
	io.run();
	t.join();
	//std::cout << "Final count is " << count << std::endl;

	return 0;
}