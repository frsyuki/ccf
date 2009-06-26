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


template <typename NodeInfo>
class node : public session {
private:
	typedef NodeInfo node_info_t;

public:
	node(const node_info_t& info, basic_session_manager* manager) :
		session(manager), m_info(info) { }

public:
	const node_info_t& info() const { return m_info; }

private:
	node_info_t m_info;

private:
	node();
	node(const node&);
};


template <typename NodeInfo>
class cluster : public session_manager<NodeInfo, cluster<NodeInfo> > {
private:
	typedef session_manager<NodeInfo, cluster<NodeInfo> > base_t;
	typedef typename base_t::identifier_t identifier_t;
	typedef NodeInfo node_info_t;

public:
	typedef node<node_info_t> node_t;
	typedef mp::shared_ptr<node_t> shared_node;
	typedef mp::weak_ptr<node_t>   weak_node;

	class connection;
	class listener;

public:
	cluster(const node_info_t& self) : m_self(self) { }
	~cluster() { }

	const node_info_t& self() const { return m_self; }
	node_info_t& self() { return m_self; }

public:
	// get server interface.
	// it manages non-cluster clients.
	server& subsystem() { return m_subsystem; }

	virtual void cluster_dispatch(shared_node from,
			method_t method, msgobj param,
			session_responder response, auto_zone& z) = 0;

	virtual void subsys_dispatch(shared_peer from,
			method_t method, msgobj param,
			session_responder response, auto_zone& z) = 0;

protected:
	friend class session_manager<NodeInfo, cluster<NodeInfo> >;

	// override session_manager::new_session
	shared_session new_session(const identifier_t& id)
	{
		return shared_session(new node_t(id, this));
	}

	// session_manager interface
	void session_created(const identifier_t& id, shared_session s)
	{
		LOG_WARN("session created ",id);
		if(s->is_connected()) { return; }
		async_connect_all(id, s);
	}

	// override session_manager::session_unbound
	void session_unbound(shared_session s)
	{
		// reconnect
		const node_info_t& id = static_cast<node_t*>(s.get())->info();
		async_connect_all(id, s);
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

	// override session_manager::dispatch
	void dispatch(shared_session from,
			method_t method, msgobj param,
			session_responder response, auto_zone& z)
	{
		throw std::logic_error("cluster::dispatch called");
	}

private:
	inline void async_connect_all(const identifier_t& id, shared_session& s)
	{
		if(id.connectable()) {
			async_connect(id, id, s);
		}
		/* FIXME node_info_t
		for(identifier_t::const_iterator it(id.begin()),
				it_end(id.end()); it != it_end; ++it) {
			LOG_WARN("connectiong to ",*it);
			async_connect(id, *it, s);
		}
		*/
	}

public:
	void accepted(int fd, const address& addr_from)
	{
		LOG_WARN("session created ",addr_from);
		// FIXME
		//   1. core::add_handler<connection>(fd, this);
		//   2. connection::process_init(): get_manager()->bind_session(id, addr, this);
		//bind_session(addr, addr, core::add_handler<connection>(fd, this));
		//bind_session(addr, connect_success_callback(fd, this, addr));
		//bind_session(addr, addr, core::add_handler<connection>(fd, this));
		core::add_handler<connection>(fd, this, addr_from);
	}

private:
	class subsys : public server {
	public:
		subsys() { }
		~subsys() { }

	public:
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

private:
	NodeInfo m_self;

	struct send_init_mixin;
};


template <typename NodeInfo>
struct cluster<NodeInfo>::send_init_mixin {
	typedef cluster<NodeInfo> cluster_t;

	send_init_mixin() { }

	send_init_mixin(int fd, cluster_t* manager)
	{
		send_init(fd, manager);
	}

	void send_init(int fd, cluster_t* manager);
};


template <typename NodeInfo>
class cluster<NodeInfo>::connection : protected send_init_mixin, public managed_state_connection<connection> {
private:
	typedef cluster<NodeInfo> cluster_t;
	typedef managed_state_connection<connection> base_t;

public:
	// unknown connection
	connection(int fd, cluster_t* manager, const address& peer_addr);

	// cluster connection
	connection(int fd, cluster_t* manager, shared_session session);

	~connection() { }

public:
	void process_init(msgobj msg, auto_zone z);

private:
	void cluster_state(msgobj msg, auto_zone z);

	void subsys_state(msgobj msg, auto_zone z);

	inline cluster_t* get_manager()
	{
		// managed_connection::m_manager
		return static_cast<cluster_t*>(base_t::m_manager);
	}

	void send_init()
	{
		send_init_mixin::send_init(base_t::fd(), get_manager());
	}

private:
	address m_peer_addr;

	connection();
	connection(const connection&);
};


template <typename NodeInfo>
class cluster<NodeInfo>::listener : public ccf::listener<typename cluster<NodeInfo>::listener> {
private:
	typedef cluster<NodeInfo> cluster_t;
	typedef ccf::listener<typename cluster<NodeInfo>::listener> base_t;

public:
	listener(int fd, cluster_t* manager) :
		base_t(fd),
		m_manager(manager) { }

	~listener() { }

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
	cluster_t* m_manager;

private:
	listener();
	listener(const listener&);
};


template <typename NodeInfo>
cluster<NodeInfo>::connection::connection(int fd, cluster_t* manager,
		const address& peer_addr) :
	send_init_mixin(), // don't send init
	base_t(fd, manager), m_peer_addr(peer_addr)
{ }

template <typename NodeInfo>
cluster<NodeInfo>::connection::connection(int fd, cluster_t* manager,
		shared_session session) :
	send_init_mixin(fd, manager),  // send init
	base_t(fd, manager, session)
{ }


template <typename NodeInfo>
void cluster<NodeInfo>::send_init_mixin::send_init(int fd, cluster_t* manager)
{
	message_init<const node_info_t&> init(manager->self());
	msgpack::sbuffer buf;
	msgpack::pack(buf, init);

	core::write(fd, buf.data(), buf.size(), &::free, buf.data());
	buf.release();
	LOG_TRACE("sent init message");
}


template <typename NodeInfo>
void cluster<NodeInfo>::connection::process_init(msgobj msg, auto_zone z)
{
	rpc_message rpc; msg.convert(&rpc);

	if(!rpc.is_init()) {
		// subsys
		LOG_DEBUG("enter subsys state");
		if(base_t::m_session) { throw msgpack::type_error(); }

		base_t::session_rebind( get_manager()->subsystem().get_session(m_peer_addr) );

		set_process(&connection::subsys_state);

		// re-process this message
		base_t::process_message(msg, z);
		return;
	}

	// cluster node
	message_init<node_info_t> init; msg.convert(&init);

	LOG_TRACE("receive init message: "/*,init.info()*/);

	if(!base_t::m_session) {
		send_init();

		// FIXME move this into cluster::bind_connection
		// FIXME try/catch?
		std::pair<bool, shared_session> bs = get_manager()->bind_session(init.info());
		base_t::session_rebind(bs.second);
		if(bs.first) {
			get_manager()->session_created(init.info(), bs.second);
		}
	}

	set_process(&connection::cluster_state);
}


template <typename NodeInfo>
void cluster<NodeInfo>::connection::subsys_state(msgobj msg, auto_zone z)
{
	LOG_TRACE("receive rpc message: ",msg);
	rpc_message rpc; msg.convert(&rpc);

	switch(rpc.type()) {
	case message_code::REQUEST: {
			message_request<msgobj> req; msg.convert(&req);
			get_manager()->subsys_dispatch(
					mp::static_pointer_cast<peer>(base_t::m_session),
					req.method(), req.param(),
					session_responder(weak_session(base_t::m_session), req.msgid()), z);
		}
		break;

	case message_code::RESPONSE: {
			message_response<msgobj, msgobj> res; msg.convert(&res);
			base_t::process_response(  // managed_connection::process_response
					res.result(), res.error(), res.msgid(), z);
		}
		break;

	default:
		throw msgpack::type_error();
	}
}


template <typename NodeInfo>
void cluster<NodeInfo>::connection::cluster_state(msgobj msg, auto_zone z)
{
	LOG_TRACE("receive rpc message: ",msg);
	rpc_message rpc; msg.convert(&rpc);

	switch(rpc.type()) {
	case message_code::REQUEST: {
			message_request<msgobj> req; msg.convert(&req);
			get_manager()->cluster_dispatch(
					mp::static_pointer_cast<node_t>(base_t::m_session),
					req.method(), req.param(),
					session_responder(weak_session(base_t::m_session), req.msgid()), z);
		}
		break;

	case message_code::RESPONSE: {
			message_response<msgobj, msgobj> res; msg.convert(&res);
			base_t::process_response(  // managed_connection::process_response
					res.result(), res.error(), res.msgid(), z);
		}
		break;

	default:
		throw msgpack::type_error();
	}
}


}  // namespace ccf

#endif /* ccf/cluster.h */

