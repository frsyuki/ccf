//
// ccf::state_connection - Cluster Communication Framework
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
#ifndef CCF_STATE_CONNECTION_H__
#define CCF_STATE_CONNECTION_H__

#include "ccf/connection.h"

namespace ccf {


template <typename IMPL>
class state_connection : public connection<IMPL> {
public:
	typedef void (IMPL::*state_type)(msgobj, auto_zone);

	state_connection(int fd, state_type state = &IMPL::process_init) :
		connection<IMPL>(fd),
		m_state(state) { }

	~state_connection() { }

public:
	void process_message(msgobj msg, auto_zone z)
	{
		(static_cast<IMPL*>(this)->*m_state)(msg, z);
	}

	// void process_init(msgobj msg, auto_zone z);

protected:
	void set_process(state_type state)
	{
		m_state = state;
	}

private:
	state_type m_state;

private:
	state_connection();
	state_connection(const state_connection&);
};


}  // namespace ccf

#endif /* ccf/state_connection.h */

