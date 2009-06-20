//
// ccf::managed_connection - Cluster Communication Framework
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
#ifndef CCF_MANAGED_CONNECTION_H__
#define CCF_MANAGED_CONNECTION_H__

#include "ccf/types.h"
#include "ccf/address.h"
#include "ccf/rpc_connection.h"
#include "ccf/state_connection.h"
#include "ccf/session.h"
#include "ccf/session_responder.h"

namespace ccf {


template <typename IMPL>
class managed_connection : public rpc_connection<IMPL> {
public:
	managed_connection(int fd, basic_session_manager* manager,
			shared_session session = shared_session(),
			address locator = address()) :
		rpc_connection<IMPL>(fd), m_manager(manager), m_session(session)
	{
		if(m_session) {
			m_session->add_connection(rpc_connection<IMPL>::fd(), locator);
		}
	}

	~managed_connection()
	{
		if(m_session) try {
			m_session->remove_connection(rpc_connection<IMPL>::fd());
		} catch(...) { }
	}

public:
	void process_request(method_t method, msgobj param,
			msgid_t msgid, auto_zone& z)
	{
		// FIXME !!m_session
		m_manager->dispatch(m_session, method, param,
				session_responder(weak_session(m_session), msgid),
				z);
	}

	void process_response(msgobj result, msgobj error,
			msgid_t msgid, auto_zone& z)
	{
		if(!m_session) { return; }  // FIXME
		m_session->process_response(result, error, msgid, z);
	}

public:
	// from framework::bind_session? FIXME
	void session_rebind(shared_session s, const address& locator)
	{
		if(m_session) {
			m_session->remove_connection(m_session, rpc_connection<IMPL>::fd());
			m_session.reset();
		}
		m_session->add_connection(rpc_connection<IMPL>::fd(), locator);
		m_session = s;
	}

protected:
	// from cluster::connection::get_manager
	basic_session_manager* m_manager;

	shared_session m_session;

private:
	managed_connection();
	managed_connection(const managed_connection&);
};


template <typename IMPL>
class managed_state_connection : public state_connection<IMPL> {
public:
	managed_state_connection(int fd, basic_session_manager* manager,
			shared_session session = shared_session(),
			address locator = address()) :
		state_connection<IMPL>(fd), m_manager(manager), m_session(session)
	{
		if(m_session) {
			m_session->add_connection(state_connection<IMPL>::fd(), locator);
		}
	}

	~managed_state_connection()
	{
		if(m_session) try {
			m_session->remove_connection(state_connection<IMPL>::fd());
		} catch(...) { }
	}

public:
	// void process_init(msgobj msg, auto_zone z);

	void process_request(method_t method, msgobj param,
			msgid_t msgid, auto_zone z)
	{
		// FIXME !!m_session
		m_manager->dispatch(m_session, method, param,
				session_responder(weak_session(m_session), msgid),
				z);
	}

	void process_response(msgobj result, msgobj error,
			msgid_t msgid, auto_zone z)
	{
		if(!m_session) { return; }
		m_session->process_response(result, error, msgid, z);
	}

public:
	// from framework::bind_session
	void session_rebind(shared_session s, const address& locator)
	{
		if(m_session) {
			m_session->remove_connection(m_session, connection<IMPL>::fd());
			m_session.reset();
		}
		m_session->add_connection(connection<IMPL>::fd(), locator);
		m_session = s;
	}


protected:
	// from cluster::connection::get_manager
	basic_session_manager* m_manager;

	shared_session m_session;

private:
	managed_state_connection();
	managed_state_connection(const managed_state_connection&);
};


}  // namespace ccf

#endif /* ccf/managed_connection.h */

