#include <ccf/service.h>
#include <ccf/scoped_listen.h>
#include <mp/object_callback.h>
#include "server/connection.h"
#include "server/service_listener.h"

namespace mpecho {

class server : public connection<server> {
public:
	server(int fd) : connection<server>(fd) { }
	~server() { }

	void process_message(msgpack::object msg, std::auto_ptr<msgpack::zone>& z)
	{
		std::cout << msg << std::endl;

		msgpack::sbuffer sbuf;
		msgpack::pack(sbuf, msg);

		ccf::net::send(fd(), sbuf.data(), sbuf.size(), &::free, sbuf.data());
		sbuf.release();
	}
};

typedef service_listener<mpecho::server> server_listener;

}  // namespace mpecho


int main(int argc, char* argv[])
{
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(3000);

	ccf::service::init();

	ccf::scoped_listen lsock(addr);

	//ccf::service::daemonize("mpecho-server.pid");

	ccf::core::add_handler<mpecho::server_listener>(lsock.sock());

	ccf::service::start(4, 3);   // worker=4, sender=3 threads
	ccf::service::join();
}

