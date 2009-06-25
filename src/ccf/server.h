//
// ccf::server - Cluster Communication Framework
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
#ifndef CCF_SERVER_H__
#define CCF_SERVER_H__

#include "ccf/address.h"
#include "ccf/session_manager.h"
#include "ccf/managed_connection.h"
#include "ccf/listener.h"
#include "ccf/service.h"

namespace ccf {


class peer : public session {
public:
	peer(const address& peeraddr, basic_session_manager* manager) :
		session(manager), m_peeraddr(peeraddr) { }

public:
	const address& peeraddr() const { return m_peeraddr; }

private:
	address m_peeraddr;

private:
	peer();
	peer(const peer&);
};

typedef mp::shared_ptr<peer> shared_peer;
typedef mp::weak_ptr<peer>   weak_peer;


class server : public session_manager<address, server> {
public:
	class connection;

public:
	server() { }
	~server() { }

public:
	// override basic_session_manager::dispatch
	//virtual void dispatch(shared_session from,
	//		method_t method, msgobj param,
	//		session_responder response, auto_zone& z);

protected:
	friend class session_manager<address, server>;

	// override session_manager::new_session
	shared_session new_session(const identifier_t& id)
	{
		return shared_session(new peer(id, this));
	}

	// override session_manager::session_created
	void session_created(const identifier_t& id, shared_session s)
	{
		LOG_WARN("session created ",id);
		// FIXME needed?
		//if(!s->is_connected() && id.connectable()) {
		//	LOG_WARN("connectiong to ",id);
		//	std::cout << "connect " << id << std::endl;
		//	async_connect(id, id, s);
		//}
	}

	// override session_manager::session_unbound
	void session_unbound(shared_session s)
	{ }

protected:
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
	// from server_listener::accepted
	void accepted(int fd, const address& addr_from)
	{
		LOG_INFO("accepted ",addr_from);
		std::pair<bool, shared_session> bs = bind_session(addr_from);

		core::add_handler<connection>(fd, this, bs.second);

		if(bs.first) {
			session_created(addr_from, bs.second);
		}
	}
};


class server::connection : public managed_connection<connection> {
public:
	connection(int fd, server* manager, shared_session session) :
		managed_connection<connection>(fd, manager, session) { }

	~connection() { }

private:
	connection();
	connection(const connection&);
};


class server_listener : public listener<server_listener> {
public:
	server_listener(int fd, server* manager) :
		listener<server_listener>(fd),
		m_manager(manager) { }

	~server_listener() { }

public:
	void closed()
	{
		service::end();
	}

	void accepted(int fd, struct sockaddr* addr, socklen_t addrlen)
	{
		// FIXME check address
		address a(*(struct sockaddr_in*)addr);
		m_manager->accepted(fd, a);
	}

private:
	server* m_manager;

private:
	server_listener();
	server_listener(const server_listener&);
};


}  // namespace ccf

#endif /* ccf/server.h */

