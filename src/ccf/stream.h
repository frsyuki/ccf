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

#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <string.h>

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

