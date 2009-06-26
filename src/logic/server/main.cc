#include <ccf/service.h>
#include <ccf/cluster.h>
#include <ccf/scoped_listen.h>
#include <cclog/cclog_tty.h>
#include "server/proto.h"
#include "server/stub.h"

namespace server {

class framework : public ccf::cluster<ccf::address> {
public:
	framework() : ccf::cluster<ccf::address>(ccf::address()) { }
	~framework() { }

	void dispatch(ccf::shared_session from,
			ccf::method_t method, ccf::msgobj param,
			ccf::session_responder response, ccf::auto_zone& z)
	{
		::server::dispatch(from, method, param, response, z);
	}

	void cluster_dispatch(shared_node from,
			ccf::method_t method, ccf::msgobj param,
			ccf::session_responder response, ccf::auto_zone& z)
	{
		dispatch(from, method, param, response, z);
	}

	void subsys_dispatch(ccf::shared_peer from,
			ccf::method_t method, ccf::msgobj param,
			ccf::session_responder response, ccf::auto_zone& z)
	{
		dispatch(from, method, param, response, z);
	}
};

std::auto_ptr<framework> net;

SVR_IMPL(Get, req, zone)
{
	LOG_INFO("Get called: key=",req.param.key);
	req.result(true);
}

SVR_IMPL(Set, req, zone)
{
	LOG_INFO("Set called: key=",req.param.key," value=",req.param.value);
	req.result(true);
}

void init(int conf_sock)
{
	net.reset(new framework());
	ccf::core::add_handler<framework::listener>(conf_sock, net.get());
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

