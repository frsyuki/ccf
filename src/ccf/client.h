//
// ccf::client - Cluster Communication Framework
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
#ifndef CCF_CLIENT_H__
#define CCF_CLIENT_H__

#include "ccf/address.h"
#include "ccf/session_manager.h"
#include "ccf/managed_connection.h"

namespace ccf {


class client_session : public session {
public:
	client_session(const address& addr, basic_session_manager* manager) :
		session(manager), m_addr(addr) { }

public:
	const address& addr() const { return m_addr; }

private:
	address m_addr;

private:
	client_session();
	client_session(const client_session&);
};

typedef mp::shared_ptr<client_session> shared_client_session;
typedef mp::weak_ptr<client_session>   weak_client_session;


template <typename Framework>
class client : public session_manager<address, Framework> {
private:
	typedef session_manager<address, Framework> base_t;
	typedef typename base_t::identifier_t identifier_t;

public:
	class connection;

public:
	client() { }
	~client() { }

public:
	// override me
	void dispatch(method_t method, msgobj param,
			session_responder response,
			const address& from, auto_zone& z)
	{
		throw msgpack::type_error();
	}

protected:
	friend class session_manager<address, Framework>;

	// override session_manager::new_session
	shared_session new_session(const identifier_t& id)
	{
		return shared_session(new client_session(id, this));
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
		address id = static_cast<client_session*>(s.get())->addr();
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
		LOG_WARN("connectiong to ",id);
		static_cast<Framework*>(this)->async_connect(id, id, s);
	}

public:
	//void accepted(int fd, const address& addr_from)
	//{
	//	std::pair<bool, shared_session> bs = bind_session(addr_from);
	//
	//	core::add_handler<connection>(fd, static_cast<Framework*>(this), bs.second);
	//
	//	if(bs.first) {
	//		session_created(addr_from, bs.second);
	//	}
	//}
};


template <typename Framework>
class client<Framework>::connection : public managed_connection<connection> {
private:
	typedef managed_connection<connection> base_t;

public:
	connection(int fd, Framework* manager, shared_session session) :
		base_t(fd, session), m_manager(manager) { }

	~connection() { }

	void dispatch(method_t method, msgobj param, msgid_t msgid, auto_zone z)
	{
		m_manager->dispatch(method, param,
				session_responder(weak_session(base_t::m_session), msgid),
				static_cast<client_session*>(base_t::m_session.get())->addr(), z);
	}

private:
	Framework* m_manager;

private:
	connection();
	connection(const connection&);
};


}  // namespace ccf

#endif /* ccf/client.h */

