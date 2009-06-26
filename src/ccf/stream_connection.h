//
// ccf::stream_connection - Cluster Communication Framework
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
#ifndef CCF_STREAM_CONNECTION_H__
#define CCF_STREAM_CONNECTION_H__

#include "ccf/connection.h"
#include "ccf/stream.h"

namespace ccf {


class basic_stream_connection {
public:
	basic_stream_connection() : m_stream_func(NULL) { }
	~basic_stream_connection() { }

public:
	typedef mp::function<void (stream)> stream_func_t;

	void reset_stream_func(stream_func_t func = stream_func_t())
	{
		m_stream_func = func;
	}

protected:
	stream_func_t m_stream_func;
};


template <typename IMPL>
class stream_connection : public connection<IMPL>, protected basic_stream_connection {
public:
	stream_connection(int fd) : connection<IMPL>(fd) { }
	~stream_connection() { }

public:
	// from connection::read_event
	void read_data()
	{
		if(m_stream_func) {
			m_stream_func( stream(connection<IMPL>::fd(), connection<IMPL>::m_pac) );
		} else {
			connection<IMPL>::read_data();
		}
	}
};


class stream_connection_access {
public:
	template <typename IMPL>
	stream_connection_access(IMPL* self) :
		m_stream(self->fd(), self->buffer()), m_self(self) { }

	~stream_connection_access() { }

public:
	typedef basic_stream_connection::stream_func_t stream_func_t;

	void reset_stream_func(stream_func_t func = stream_func_t())
	{
		m_self->reset_stream_func(func);
	}

	stream& streaming() { return m_stream; }
	const stream& streaming() const { return m_stream; }

private:
	stream m_stream;
	basic_stream_connection* m_self;
};


}  // namespace ccf

#endif /* ccf/stream_connection.h */

