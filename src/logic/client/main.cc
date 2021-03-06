#include <ccf/service.h>
#include <ccf/mhclient.h>
#include <cclog/cclog_tty.h>
#include "server/proto.h"

namespace client {

class framework : public ccf::mhclient<framework> {
public:
	framework() { }
	~framework() { }
};

std::auto_ptr<framework> net;

int req = 0;

void cb_Get(ccf::msgobj res, ccf::msgobj err, ccf::auto_zone z, int* context)
{
	LOG_INFO("Get callback: res=",res," err=",err," context=",*context);
	if(--req == 0) { ccf::service::end(); }
}

void cb_Set(ccf::msgobj res, ccf::msgobj err, ccf::auto_zone z, int* context)
{
	LOG_INFO("Set callback: res=",res," err=",err," context=",*context);
	if(--req == 0) { ccf::service::end(); }
}

void init(ccf::maddress conf_addr)
{
	net.reset(new framework());

	ccf::shared_session session = net->get_session(conf_addr);

	using namespace mp::placeholders;

	ccf::shared_zone life(new msgpack::zone());
	int* context = life->allocate<int>(10);

	server::Set set("test", "test");
	session->call(set, life, mp::bind(cb_Set, _1, _2, _3, context));
	++req;

	server::Get get("test");
	session->call(get, life, mp::bind(cb_Get, _1, _2, _3, context));
	++req;
}

}  // namespace client


int main(int argc, char* argv[])
{
	cclog::reset(new cclog_tty(cclog::TRACE, std::cout));
	ccf::service::init();

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	addr.sin_port = htons(3000);

	ccf::maddress maddr;
	maddr.push_back( ccf::address(addr) );
	client::init(maddr);
	//client::init( ccf::address(addr) );

	//ccf::service::start(4);  // 4 threads
	//ccf::service::join();
	while(!ccf::service::is_end()) {
		ccf::service::step_next();
	}
}

