#include <ccf/service.h>
#include <mp/functional.h>
#include <msgpack.hpp>
#include <string.h>
#include "server/connection.h"

namespace mpecho {

class client : public connection<client> {
public:
	static void connect_callback(int fd, int err, int argc, char** argv)
	try {
		if(err) {
			throw mp::system_error(err, "connect failed");
		}
	
		ccf::core::add_handler<client>(fd, argc, argv);
	
		ccf::service::stop();
	
	} catch(std::exception& e) {
		std::clog << e.what() << std::endl;
		ccf::service::stop();
	}

public:
	client(int fd, int argc, char** argv) : connection<client>(fd)
	{
		msgpack::sbuffer sbuf;
		msgpack::packer<msgpack::sbuffer> pk(sbuf);
	
		pk.pack_array(argc);
		for(int i=0; i < argc; ++i) {
			size_t len = strlen(argv[i]);
			pk.pack_raw(len);
			pk.pack_raw_body(argv[i], len);
		}
	
		ccf::net::send(fd, sbuf.data(), sbuf.size(), &::free, sbuf.data());
		sbuf.release();
	}

	~client() { }

	void process_message(msgpack::object msg, std::auto_ptr<msgpack::zone>& z)
	{
		std::cout << msg << std::endl;

		ccf::service::stop();
	}
};

}  // namespace mpecho


int main(int argc, char* argv[])
{
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	addr.sin_port = htons(3000);

	ccf::service::init();

	using namespace mp::placeholders;

	ccf::core::connect(PF_INET, SOCK_STREAM, 0,
			(sockaddr*)&addr, sizeof(addr),
			10000,  // timeout (msec)
			mp::bind(
				&mpecho::client::connect_callback,
				_1, _2, argc, argv));

	ccf::service::start(4, 3);
	ccf::service::join();
}

