//
// ccf::cluster - Cluster Communication Framework
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
#ifndef CCF_CLUSTER_H__
#define CCF_CLUSTER_H__

#include "ccf/address.h"
#include "ccf/session_manager.h"
#include "ccf/managed_connection.h"
#include "ccf/server.h"

namespace ccf {


class node : public session {
public:
	// FIXME multihome address
	node(const address& addr, basic_session_manager* manager) :
		session(manager), m_addr(addr) { }

public:
	const address& addr() const { return m_addr; }

private:
	address m_addr;

private:
	node();
	node(const node&);
};

typedef mp::shared_ptr<node> shared_node;
typedef mp::weak_ptr<node>   weak_node;


// FIXME multihome address
class cluster : public session_manager<address, cluster> {
public:
	class connection;

public:
	cluster() { }
	~cluster() { }

public:
	// get server interface.
	// it manages non-cluster clients.
	server& subsystem() { return m_subsystem; }

public:
	// session_manager interface
	void session_created(const identifier_t& addr, shared_session s)
	{
		LOG_WARN("session created ",addr);
		// FIXME multihome connect
		if(!s->is_connected() && addr.connectable()) {
			LOG_WARN("connectiong to ",addr);
			std::cout << "connect " << addr << std::endl;
			async_connect(addr, addr, s);
		}
	}

	// FIXME
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

	// override session_manager::session_unbound
	void session_unbound(shared_session s)
	{
		// reconnect
		address addr = static_cast<node*>(s.get())->addr();  // mp::static_pointer_cast<node>(s) ?
		async_connect(addr, addr, s);
	}

	// override session_manager::create_session
	shared_session create_session(const identifier_t& id)
	{
		return shared_session(new node(id, this));
	}

	// override session_manager::dispatch
	void dispatch(shared_session from,
			method_t method, msgobj param,
			session_responder response, auto_zone& z)
	{
		throw std::logic_error("cluster::dispatch called");
	}

	virtual void cluster_dispatch_request(
			shared_node from,
			method_t method, msgobj param,
			responder& response, auto_zone& z) = 0;

	virtual void subsys_dispatch_request(
			shared_peer from,
			method_t method, msgobj param,
			responder& response, auto_zone& z) = 0;

public:
	void accepted(int fd, const address& addr)
	{
		LOG_WARN("session created ",addr);
		// FIXME
		//   1. core::add_handler<connection>(fd, this);
		//   2. connection::process_init(): get_manager()->bind_session(id, addr, this);
		//bind_session(addr, addr, core::add_handler<connection>(fd, this));
		//bind_session(addr, connect_success_callback(fd, this, addr));
		//bind_session(addr, addr, core::add_handler<connection>(fd, this));
		core::add_handler<connection>(fd, this);
	}

private:
	class subsys : public server {
	public:
		subsys() { }
		~subsys();

	private:
		// override session_manager::dispatch
		void dispatch(shared_session from,
				method_t method, msgobj param,
				session_responder response, auto_zone& z)
		{
			throw std::logic_error("subsys::dispatch called");
		}

	private:
		subsys(const subsys&);
	};

	subsys m_subsystem;
};


class cluster::connection : public managed_state_connection<connection> {
public:
	// unknown connection
	connection(int fd, cluster* manager);

	// cluster connection
	connection(int fd, cluster* manager, shared_session session);

	~connection() { }

public:
	void process_init(msgobj msg, auto_zone z);

private:
	void cluster_state(msgobj msg, auto_zone z);
	void subsys_state(msgobj msg, auto_zone z);
	inline cluster* get_manager();

private:
	connection();
	connection(const connection&);
};


class cluster_listener : public listener<cluster_listener> {
public:
	cluster_listener(int fd, cluster* manager) :
		listener<cluster_listener>(fd),
		m_manager(manager) { }

	~cluster_listener() { }

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
	cluster* m_manager;

private:
	cluster_listener();
	cluster_listener(const cluster_listener&);
};


}  // namespace ccf

#endif /* ccf/cluster.h */

