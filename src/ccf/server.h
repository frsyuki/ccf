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
#include <mp/pthread.h>
#include <map>

namespace ccf {


class peer : public session {
public:
	peer(const address& peeraddr,
			basic_session_manager* manager) :
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


// FIXME multihome address
class server : public session_manager<address, server> {
public:
	class connection;

public:
	server() { }
	~server() { }

public:
	// override session_manager::session_created
	void session_created(const identifier_t& addr, shared_session s)
	{
		LOG_WARN("session created ",addr);
		// FIXME needed?
		//if(!s->is_connected() && addr.connectable()) {
		//	LOG_WARN("connectiong to ",addr);
		//	std::cout << "connect " << addr << std::endl;
		//	async_connect(addr, addr, s);
		//}
	}

	// FIXME
	struct connect_success_callback {
		connect_success_callback(int fd_, server* manager_, const address& locator_) :
			fd(fd_), manager(manager_), locator(locator_) { }
		void operator() (shared_session s)
		{
			core::add_handler<connection>(fd, manager, s, locator);
		}
	private:
		int fd;
		server* manager;
		const address& locator;
	};

	// override session_manager::connect_success
	void connect_success(int fd, const identifier_t& id, const address& locator, shared_session& s)
	{
		//FIXME bind_session(id, connect_success_callback(fd, this, locator));
		core::add_handler<connection>(fd, this, s, locator);
	}

	// override session_manager::connect_failed
	void connect_failed(int fd, const identifier_t& id, const address& locator, shared_session& s)
	{ }

	// override session_manager::session_unbound
	void session_unbound(shared_session s)
	{ }

	// override session_manager::create_session
	shared_session create_session(const identifier_t& id)
	{
		return shared_session(new peer(id, this));
	}

	// override session_manager::dispatch
	//virtual void dispatch(shared_session from,
	//		method_t method, msgobj param,
	//		session_responder response, auto_zone& z);

public:
	// from server_listener::accepted
	void accepted(int fd, const address& addr)
	{
		LOG_WARN("session created ",addr);
		bind_session(addr, connect_success_callback(fd, this, addr));
		//bind_session(addr, addr, core::add_handler<connection>(fd, this));
	}
};


class server::connection : public managed_connection<connection> {
public:
	connection(int fd, server* manager,
			shared_session session, const address& locator) :
		managed_connection<connection>(fd, manager, session, locator) { }

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

