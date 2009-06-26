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


template <typename Framework>
class server : public session_manager<address, Framework> {
private:
	typedef session_manager<address, Framework> base_t;
	typedef typename base_t::identifier_t identifier_t;

public:
	class connection;
	class listener;

public:
	server() { }
	~server() { }

public:
	// override me
	//void dispatch(method_t method, msgobj param,
	//		session_responder response,
	//		const address& from, auto_zone& z);

protected:
	friend class session_manager<address, Framework>;

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
		core::add_handler<connection>(fd, static_cast<Framework*>(this), s);
	}

	// override session_manager::connect_failed
	void connect_failed(int fd, const identifier_t& id,
			const address& addr_to, shared_session& s)
	{ }

public:
	// from listener::accepted
	void accepted(int fd, const address& addr_from)
	{
		LOG_INFO("accepted ",addr_from);
		std::pair<bool, shared_session> bs = static_cast<Framework*>(this)->bind_session(addr_from);

		core::add_handler<connection>(fd, static_cast<Framework*>(this), bs.second);

		if(bs.first) {
			static_cast<Framework*>(this)->session_created(addr_from, bs.second);
		}
	}
};


template <typename Framework>
class server<Framework>::connection : public managed_connection<connection> {
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
				static_cast<peer*>(base_t::m_session.get())->peeraddr(), z);
	}

private:
	Framework* m_manager;

private:
	connection();
	connection(const connection&);
};


template <typename Framework>
class server<Framework>::listener : public ccf::listener<typename server<Framework>::listener> {
public:
	listener(int fd, Framework* manager) :
		ccf::listener<typename server<Framework>::listener>(fd),
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
	Framework* m_manager;

private:
	listener();
	listener(const listener&);
};


}  // namespace ccf

#endif /* ccf/server.h */

