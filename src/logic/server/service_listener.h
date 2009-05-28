#ifndef SERVICE_LISTENER_H__
#define SERVICE_LISTENER_H__

#include <ccf/service.h>
#include <ccf/listener.h>


template <typename Handler>
class service_listener : public ccf::listener<service_listener<Handler> > {
public:
	service_listener(int fd) : ccf::listener<service_listener>(fd) { }
	~service_listener() { }

	void accepted(int fd, struct sockaddr* addr, socklen_t addrlen)
	{
		ccf::core::add_handler<Handler>(fd);
	}

	void closed()
	{
		ccf::service::stop();
	}
};


#endif /* service_listener.h */

