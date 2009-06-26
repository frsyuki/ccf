//
// ccf::mhclient - Cluster Communication Framework
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
#ifndef CCF_MHCLIENT_H__
#define CCF_MHCLIENT_H__

#include "ccf/maddress.h"
#include "ccf/session_manager.h"
#include "ccf/managed_connection.h"
#include <mp/pthread.h>
#include <map>

namespace ccf {


class mhclient_session : public session {
public:
	mhclient_session(const maddress& maddr, basic_session_manager* manager) :
		session(manager), m_maddr(maddr) { }

public:
	const maddress& maddr() const { return m_maddr; }

private:
	maddress m_maddr;

private:
	mhclient_session();
	mhclient_session(const mhclient_session&);
};

typedef mp::shared_ptr<mhclient_session> shared_client_session;
typedef mp::weak_ptr<mhclient_session>   weak_client_session;


template <typename Framework>
class mhclient : public session_manager<maddress, Framework> {
private:
	typedef session_manager<maddress, Framework> base_t;
	typedef typename base_t::identifier_t identifier_t;

public:
	class connection;

public:
	mhclient() { }
	~mhclient() { }

public:
	// override basic_session_manager::dispatch
	virtual void dispatch(shared_session from,
			method_t method, msgobj param,
			session_responder response, auto_zone& z)
	{
		throw msgpack::type_error();
	}

protected:
	friend class session_manager<maddress, Framework>;

	// override session_manager::new_session
	shared_session new_session(const identifier_t& id)
	{
		return shared_session(new mhclient_session(id, this));
	}

	// override session_manager::session_created
	void session_created(const identifier_t& id, shared_session s)
	{
		LOG_WARN("session created ",id);
		if(s->is_connected()) { return; }
		static_cast<Framework*>(this)->async_connect_all(id, s);
	}

	// override session_manager::session_unbound
	void session_unbound(shared_session s)
	{
		// reconnect
		const maddress& id = static_cast<mhclient_session*>(s.get())->maddr();
		static_cast<Framework*>(this)->async_connect_all(id, s);
	}

	// override session_manager::connect_success
	void connect_success(int fd, const identifier_t& id,
			const address& addr_to, shared_session& s)
	{
		core::add_handler<connection>(fd, static_cast<Framework*>(this), s);
	}

	// override session_manager::connect_failed
	void connect_failed(int fd, const identifier_t& id,
			const address& addr_to, shared_session& s)
	{ }

private:
	inline void async_connect_all(const identifier_t& id, shared_session& s)
	{
		for(typename identifier_t::const_iterator it(id.begin()),
				it_end(id.end()); it != it_end; ++it) {
			LOG_WARN("connectiong to ",*it);
			static_cast<Framework*>(this)->async_connect(id, *it, s);
		}
	}

public:
	//void accepted(int fd, const address& addr_from)
	//{
	//	std::pair<bool, shared_session> bs = bind_session(addr_from);
	//
	//	core::add_handler<connection>(fd, static_cast<Framework*>(this), bs.second, addr_from);
	//
	//	if(bs.first) {
	//		session_created(addr_from, bs.second);
	//	}
	//}
};


template <typename Framework>
class mhclient<Framework>::connection : public managed_connection<connection> {
public:
	connection(int fd, Framework* manager, shared_session session) :
		managed_connection<connection>(fd, manager, session) { }

	~connection() { }

private:
	connection();
	connection(const connection&);
};


}  // namespace ccf

#endif /* ccf/mhclient.h */

