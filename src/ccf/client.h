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
#include <mp/pthread.h>
#include <map>

namespace ccf {


class client_session : public session {
public:
	client_session(const address& addr,
			basic_session_manager* manager) :
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


class client : public session_manager<address, client> {
public:
	class connection;
	class listener;

public:
	client() { }
	~client() { }

public:
	// override session_manager::session_created
	void session_created(const identifier_t& addr, shared_session s)
	{
		LOG_WARN("session created ",addr);
		if(!s->is_connected() && addr.connectable()) {
			LOG_WARN("connectiong to ",addr);
			std::cout << "connect " << addr << std::endl;
			async_connect(addr, addr, s);
		}
	}

	// override session_manager::connect_success
	void connect_success(int fd, const identifier_t& id, const address& locator, shared_session& s)
	{
		// FIXME atomic m_sessions, m_manager
		// session_manager::bind_session
		//bind_session(id, locator, core::add_handler<connection>(fd, this));
		//FIXME bind_session(id, connect_success_callback(fd, this, locator));
		core::add_handler<connection>(fd, this, s, locator);
	}

	// override session_manager::connect_failed
	void connect_failed(int fd, const identifier_t& id, const address& locator, shared_session& s)
	{ }

	// override session_manager::session_unbound
	void session_unbound(shared_session s)
	{
		// reconnect
		address addr = static_cast<client_session*>(s.get())->addr();  // mp::static_pointer_cast<client_session>(s) ?
		async_connect(addr, addr, s);
	}

	// override session_manager::create_session
	shared_session create_session(const identifier_t& id)
	{
		return shared_session(new client_session(id, this));
	}

	// override session_manager::dispatch
	virtual void dispatch(shared_session from,
			method_t method, msgobj param,
			session_responder response, auto_zone& z)
	{
		throw msgpack::type_error();
	}

public:
	// FIXME needed?
	//void accepted(int fd, const address& addr)
	//{
	//	LOG_WARN("session created ",addr);
	//	bind_session(addr, addr, core::add_handler<connection>(fd));
	//}
};


class client::connection : public managed_connection<connection> {
public:
	connection(int fd, client* manager,
			shared_session session, const address& locator) :
		managed_connection<connection>(fd, manager, session, locator) { }

	~connection() { }

private:
	connection();
	connection(const connection&);
};


}  // namespace ccf

#endif /* ccf/client.h */

