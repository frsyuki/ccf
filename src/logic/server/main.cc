#include <ccf/service.h>
#include <ccf/cluster.h>
#include <ccf/scoped_listen.h>
#include <cclog/cclog_tty.h>
#include "server/proto.h"
#include "server/stub.h"

namespace server {

class framework : public ccf::cluster<ccf::address, framework> {
private:
	typedef ccf::cluster<ccf::address, framework> base_t;

public:
	framework(ccf::address self) : base_t(self) { }
	~framework() { }
//public:
//	framework() { }
//	~framework() { }

	void dispatch(ccf::method_t method, ccf::msgobj param,
			ccf::session_responder response,
			const ccf::address& from, ccf::auto_zone& z)
	{
		::server::dispatch(method, param, response/*, from*/, z);
	}

	void cluster_dispatch(ccf::method_t method, ccf::msgobj param,
			ccf::session_responder response,
			const ccf::address& from, ccf::auto_zone& z)
	{
		::server::dispatch(method, param, response/*, from*/, z);
	}

	void subsys_dispatch(ccf::method_t method, ccf::msgobj param,
			ccf::session_responder response,
			const ccf::address& from, ccf::auto_zone& z)
	{
		::server::dispatch(method, param, response/*, from*/, z);
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

void init(ccf::address self, int conf_sock)
{
	net.reset(new framework(self));
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
	server::init(ccf::address(addr), lsock.get());

	ccf::service::start(4);  // 4 threads
	ccf::service::join();
	LOG_INFO("end");
}

