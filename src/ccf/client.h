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


class client : public session_manager<address, client> {
public:
	class connection;
	class listener;

public:
	client() { }
	~client() { }

public:
	// override basic_session_manager::dispatch
	virtual void dispatch(shared_session from,
			method_t method, msgobj param,
			session_responder response, auto_zone& z)
	{
		throw msgpack::type_error();
	}

protected:
	friend class session_manager<address, client>;

	// override session_manager::new_session
	shared_session new_session(const identifier_t& id)
	{
		return shared_session(new client_session(id, this));
	}

	// override session_manager::session_created
	void session_created(const identifier_t& id, shared_session s)
	{
		LOG_WARN("session created ",id);
		if(!s->is_connected()) {
			LOG_WARN("connectiong to ",id);
			async_connect(id, id, s);
		}
	}

	// override session_manager::session_unbound
	void session_unbound(shared_session s)
	{
		// reconnect
		address id = static_cast<client_session*>(s.get())->addr();
		async_connect(id, id, s);
	}

	// override session_manager::connect_success
	void connect_success(int fd, const identifier_t& id,
			const address& addr_to, shared_session& s)
	{
		core::add_handler<connection>(fd, this, s);
	}

	// override session_manager::connect_failed
	void connect_failed(int fd, const identifier_t& id,
			const address& addr_to, shared_session& s)
	{ }

public:
	//void accepted(int fd, const address& addr_from)
	//{
	//	std::pair<bool, shared_session> bs = bind_session(addr_from);
	//
	//	core::add_handler<connection>(fd, this, bs.second);
	//
	//	if(bs.first) {
	//		session_created(addr_from, bs.second);
	//	}
	//}
};


class client::connection : public managed_connection<connection> {
public:
	connection(int fd, client* manager, shared_session session) :
		managed_connection<connection>(fd, manager, session) { }

	~connection() { }

private:
	connection();
	connection(const connection&);
};


}  // namespace ccf

#endif /* ccf/client.h */

