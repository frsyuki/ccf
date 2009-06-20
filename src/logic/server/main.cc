#include <ccf/service.h>
#include <ccf/server.h>
#include <ccf/scoped_listen.h>
#include <cclog/cclog_tty.h>
#include "server/proto.h"
#include "server/stub.h"

namespace server {

class framework : public ccf::server {
public:
	framework() { }
	~framework() { }

	void dispatch(ccf::shared_session from,
			ccf::method_t method, ccf::msgobj param,
			ccf::session_responder response, ccf::auto_zone& z)
	{
		::server::dispatch(from, method, param, response, z);  // stub.h
	}
};

std::auto_ptr<framework> net;

void svr_Get(Get param, ccf::session_responder response,
		ccf::shared_session from, ccf::auto_zone& z)
{
	LOG_INFO("Get called: key=",param.key);
	response.result(true);
}

void svr_Set(Set param, ccf::session_responder response,
		ccf::shared_session from, ccf::auto_zone& z)
{
	LOG_INFO("Set called: key=",param.key," value=",param.value);
	response.result(true);
}

void init(int conf_sock)
{
	net.reset(new framework());

	ccf::core::add_handler<ccf::server_listener>(conf_sock, net.get());
}

}  // namespace server


int main(int argc, char* argv[])
{
	cclog::reset(new cclog_tty(cclog::TRACE, std::cout));
	ccf::service::init();

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(3000);

	ccf::scoped_listen lsock(addr);
	server::init(lsock.get());

	ccf::service::start(4);  // 4 threads
	ccf::service::join();
	LOG_INFO("end");
}

