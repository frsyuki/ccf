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


template <typename IMPL>
class stream_connection : public connection<IMPL> {
public:
	stream_connection(int fd) :
		connection<IMPL>(fd), m_stream_func(NULL) { }

	~stream_connection() { }

public:
	typedef void (IMPL::*stream_func_t)(stream s);

	void reset_stream_func(stream_func_t* func = NULL)
	{
		m_stream_func = func;
	}

public:
	// from connection::read_event
	void read_data();

private:
	stream_func_t m_stream_func;
};


template <typename IMPL>
inline void stream_connection<IMPL>::read_data()
{
	if(m_stream_func) {
		(static_cast<IMPL*>(this)->*m_stream_func)(
				stream(connection<IMPL>::fd(), connection<IMPL>::m_pac) );
	} else {
		connection<IMPL>::read_data();
	}
}


}  // namespace ccf

#endif /* ccf/stream_connection.h */

