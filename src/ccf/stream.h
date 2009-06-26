//
// ccf::stream - Cluster Communication Framework
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
#ifndef CCF_STREAM_H__
#define CCF_STREAM_H__

#include <mp/exception.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

namespace ccf {


class stream {
public:
	stream(int fd, msgpack::unpacker& pac) :
		m_fd(fd), m_pac(pac) { }

	~stream() { }

public:
	ssize_t read(void* buf, size_t count)
	{
		size_t sz = m_pac.nonparsed_size();
		if(sz > 0) {
			sz = std::max(sz, count);
			memcpy(buf, m_pac.nonparsed_buffer(), sz);
			m_pac.buffer_consumed(sz);
			return sz;
		} else {
			// FIXME readv() if count < X
			return ::read(m_fd, buf, count);
		}
	}

	void read_all(void* buf, size_t count)
	{
		{
			size_t sz = m_pac.nonparsed_size();
			if(sz > 0) {
				if(count <= sz) {
					memcpy(buf, m_pac.nonparsed_buffer(), count);
					m_pac.buffer_consumed(count);
					return;
				}
				memcpy(buf, m_pac.nonparsed_buffer(), sz);
				m_pac.buffer_consumed(sz);
				buf = (char*)buf + sz;
				count -= sz;
			}
		}

		if(::fcntl(m_fd, F_SETFL, 0) < 0) {
			throw mp::system_error(errno, "failed to reset nonblock flag");
		}

		do {
			ssize_t sz = ::read(m_fd, buf, count);
			if(sz <= 0) {
				if(sz == 0) { throw mp::system_error(errno, "read_all failed"); }
				if(errno == EINTR) { continue; }
				throw mp::system_error(errno, "read_all failed");
			}
			buf = (char*)buf + sz;
			count -= sz;
		} while(count > 0);

		if(::fcntl(m_fd, F_SETFL, O_NONBLOCK) < 0) {
			throw mp::system_error(errno, "failed to set nonblock flag");
		}
	}

	msgpack::unpacker& buffer()
	{
		return m_pac;
	}

	const msgpack::unpacker& buffer() const
	{
		return m_pac;
	}

private:
	int m_fd;
	msgpack::unpacker& m_pac;

private:
	stream();
};


}  // namespace ccf

#endif /* ccf/stream.h */

