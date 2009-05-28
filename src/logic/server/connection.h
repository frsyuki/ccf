#ifndef CONNECTION_H__
#define CONNECTION_H__

#include <ccf/service.h>
#include <msgpack.hpp>
#include <memory>
#include <errno.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#ifndef CONNECTION_INITIAL_BUFFER_SIZE
#define CONNECTION_INITIAL_BUFFER_SIZE (64*1024)
#endif

#ifndef CONNECTION_BUFFER_RESERVATION_SIZE
#define CONNECTION_BUFFER_RESERVATION_SIZE (8*1024)
#endif


template <typename IMPL>
class connection : public mp::wavy::handler {
public:
	connection(int fd);
	~connection();

public:
	void read_event();

	void read_data();

	//void process_message(msgpack::object msg, auto_zone& z);

protected:
	msgpack::unpacker m_pac;

private:
	connection();
	connection(const connection&);
};


template <typename IMPL>
connection<IMPL>::connection(int fd) :
	mp::wavy::handler(fd),
	m_pac(CONNECTION_INITIAL_BUFFER_SIZE) { }

template <typename IMPL>
connection<IMPL>::~connection() { }


template <typename IMPL>
void connection<IMPL>::read_data()
{
	m_pac.reserve_buffer(CONNECTION_BUFFER_RESERVATION_SIZE);

	ssize_t rl = ::read(fd(), m_pac.buffer(), m_pac.buffer_capacity());
	if(rl <= 0) {
		if(rl == 0) {
			throw mp::system_error(errno, "connection closed");
		}
		if(errno == EAGAIN || errno == EINTR) {
			return;
		} else {
			throw mp::system_error(errno, "read error");
		}
	}

	m_pac.buffer_consumed(rl);

	while(m_pac.execute()) {
		msgpack::object msg = m_pac.data();
		std::auto_ptr<msgpack::zone> z( m_pac.release_zone() );
		m_pac.reset();
		static_cast<IMPL*>(this)->process_message(msg, z);
	}
}

template <typename IMPL>
void connection<IMPL>::read_event()
try {

	static_cast<IMPL*>(this)->read_data();

} catch(msgpack::type_error& e) {
	//std::clog << "connection: type error" << std::endl;
	//LOG_ERROR("connection: type error");
	throw;
} catch(std::exception& e) {
	//std::clog << "connection: " << e.what() << std::endl;
	//LOG_WARN("connection: ", e.what());
	throw;
} catch(...) {
	//std::clog << "connection: unknown error" << std::endl;
	//LOG_ERROR("connection: unknown error");
	throw;
}


#endif /* connection.h */

