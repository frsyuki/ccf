//
// ccf::connection - Cluster Communication Framework
//
// Copyright (C) 2009 FURUHASHI Sadayuki
//
//    Licensed under the Apache License, Version 2.0 (the "License");
//    you may not use this file except in compliance with the License.
//    You may obtain a copy of the License at
//
//        http://www.apache.org/licenses/LICENSE-2.0
//
//    Unless required by applicable law or agreed to in writing, software
//    distributed under the License is distributed on an "AS IS" BASIS,
//    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//    See the License for the specific language governing permissions and
//    limitations under the License.
//
#ifndef CCF_CONNECTION_H__
#define CCF_CONNECTION_H__

#include "ccf/service.h"
#include "ccf/types.h"
#include <cclog/cclog.h>
#include <msgpack.hpp>
#include <memory>
#include <errno.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#ifndef CCF_CONNECTION_INITIAL_BUFFER_SIZE
#define CCF_CONNECTION_INITIAL_BUFFER_SIZE (64*1024)
#endif

#ifndef CCF_CONNECTION_BUFFER_RESERVATION_SIZE
#define CCF_CONNECTION_BUFFER_RESERVATION_SIZE (8*1024)
#endif

namespace ccf {


template <typename IMPL>
class connection : public core::handler {
public:
	connection(int fd) :
		core::handler(fd),
		m_pac(CCF_CONNECTION_INITIAL_BUFFER_SIZE) { }

	~connection() { }

public:
	// from wavy: readable notification
	void read_event();

	void read_data();

	// void process_message(msgpack::object msg, auto_zone z);

protected:
	msgpack::unpacker m_pac;

private:
	connection();
	connection(const connection&);
};


template <typename IMPL>
void connection<IMPL>::read_data()
{
	m_pac.reserve_buffer(CCF_CONNECTION_BUFFER_RESERVATION_SIZE);

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
		msgobj msg = m_pac.data();
		LOG_TRACE("obj received: ",msg);
		auto_zone z( m_pac.release_zone() );
		m_pac.reset();
		static_cast<IMPL*>(this)->process_message(msg, z);
	}
}

template <typename IMPL>
void connection<IMPL>::read_event()
try {

	static_cast<IMPL*>(this)->read_data();

} catch(msgpack::type_error& e) {
	LOG_ERROR("connection: type error");
	throw;
} catch(std::exception& e) {
	LOG_WARN("connection: ", e.what());
	throw;
} catch(...) {
	LOG_ERROR("connection: unknown error");
	throw;
}


}  // namespace ccf

#endif /* ccf/connection.h */

