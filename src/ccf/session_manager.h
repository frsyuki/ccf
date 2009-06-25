//
// ccf::session_manager - Cluster Communication Framework
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
#ifndef CCF_SESSION_MANAGER_H__
#define CCF_SESSION_MANAGER_H__

#include "ccf/types.h"
#include "ccf/address.h"
#include "ccf/session.h"
#include "ccf/session_responder.h"
#include "ccf/util.h"
#include "ccf/service.h"

namespace ccf {


class basic_session_manager {
public:
	basic_session_manager() { }
	virtual ~basic_session_manager() { }

public:
	virtual void session_unbound(shared_session s) = 0;

	// called when message is received from managed connection.
	// from managed_connection::dispatch
	virtual void dispatch(shared_session from,
			method_t method, msgobj param,
			session_responder response, auto_zone& z) = 0;
};


template <typename Identifier, typename IMPL>
class session_manager : public basic_session_manager {
public:
	typedef Identifier identifier_t;

public:
	session_manager(
			unsigned long connect_timeout = 5000,  // msec
			unsigned short connect_retry_limit = 10);
	~session_manager();

public:
	void step_timeout();

	// get/create RPC stub instance identified by id.
	shared_session get_session(const Identifier& id);

protected:
	// called to create new session
	shared_session new_session(const Identifier& id)
	{
		return shared_session(new session());
	}

	// add connection to the session identified by id.
	// create session if the session doesn't exist.
	// return pair of 'if session is created' and 'bound session.'
	// you may want to call session_created if this function returns true.
	std::pair<bool, shared_session> bind_session(const Identifier& id);

	// called when new session is created.
	// from get_session
	void session_created(const Identifier& id, shared_session s) { }

	// called when the session is unbound.
	// from session::remove_connection
	void session_unbound(shared_session s) { }

protected:
	// connect to the address
	// if connect succeeded, connect_success() is called.
	// if connect failed and retry failed, connect_failed() is called?
	void async_connect(const Identifier& id, const address& locator, shared_session& s);
	// void connect_success(int fd, const Identifier& id, const address& locator, shared_session& s);
	// void connect_failed(int fd, const Identifier& id, const address& locator, shared_session& s);

private:
	void connect_callback(Identifier id, address locator, shared_session s, int fd, int err);

private:
	mp::pthread_mutex m_mutex;
	// FIXME std::tr1::unoredered_multimap
	typedef std::multimap<Identifier, weak_session> sessions_t;
	sessions_t m_sessions;

	unsigned long m_connect_timeout;
	unsigned short m_connect_retry_limit;

private:
	session_manager(const session_manager&);
};


template <typename Identifier, typename IMPL>
session_manager<Identifier, IMPL>::session_manager(
		unsigned long connect_timeout,
		unsigned short connect_retry_limit) :
	m_connect_timeout(connect_timeout),
	m_connect_retry_limit(connect_retry_limit)
{ }

template <typename Identifier, typename IMPL>
session_manager<Identifier, IMPL>::~session_manager() { }

template <typename Identifier, typename IMPL>
std::pair<bool, shared_session> session_manager<Identifier, IMPL>::bind_session(const Identifier& id)
{
	pthread_scoped_lock lk(m_mutex);

	std::pair<typename sessions_t::iterator, typename sessions_t::const_iterator> pair =
		m_sessions.equal_range(id);

	while(pair.first != pair.second) {
		shared_session s = pair.first->second.lock();
		if(s) {
			return std::make_pair(false, s);
		}
		++pair.first;
		//m_sessions.erase(pair.first++);  // session lost notify
	}

	shared_session s = static_cast<IMPL*>(this)->new_session(id);
	m_sessions.insert( typename sessions_t::value_type(id, weak_session(s)) );

	return std::make_pair(true, s);
}

template <typename Identifier, typename IMPL>
shared_session session_manager<Identifier, IMPL>::get_session(const Identifier& id)
{
	std::pair<bool, shared_session> bs = bind_session(id);
	const bool created = bs.first;
	shared_session& s = bs.second;

	if(created) {
		static_cast<IMPL*>(this)->session_created(id, s);
	}
	return s;
}


template <typename Identifier, typename IMPL>
void session_manager<Identifier, IMPL>::step_timeout()
{
	LOG_TRACE("step timeout ...");

	for(typename sessions_t::iterator it(m_sessions.begin()),
			it_end(m_sessions.end()); it != it_end; ) {
		shared_session s(it->second.lock());
		if(!s) {
			core::submit(&session::step_timeout, s, s);
			++it;
		} else {
			m_sessions.erase(it++);
		}
	}

	LOG_TRACE("step timeout done");
}


template <typename Identifier, typename IMPL>
void session_manager<Identifier, IMPL>::async_connect(const Identifier& id, const address& locator, shared_session& s)
{
	LOG_INFO("connecting to ",locator);

	char addrbuf[locator.addrlen()];
	locator.getaddr((sockaddr*)&addrbuf);

	using namespace mp::placeholders;
	core::connect_thread(PF_INET, SOCK_STREAM, 0,  // FIXME connect_event?
			(sockaddr*)addrbuf, sizeof(addrbuf),
			m_connect_timeout,
			mp::bind(
				&session_manager<Identifier, IMPL>::connect_callback,
				this, id, locator, s, _1, _2));
}


template <typename Identifier, typename IMPL>
void session_manager<Identifier, IMPL>::connect_callback(Identifier id, address locator, shared_session s, int fd, int err)
{
	if(fd < 0) {
		LOG_INFO("connect failed ",locator,": ",strerror(err));
		if(s->connect_failed() > m_connect_retry_limit) {
			static_cast<IMPL*>(this)->connect_failed(fd, id, locator, s);
		} else {
			// retry connect
			// FIXME: retry only when err == ETIMEDOUT?
			async_connect(id, locator, s);
		}
		return;
	}

	util::fd_setup(fd);

	LOG_INFO("connect success ",locator," fd(",fd,")");
	try {
		static_cast<IMPL*>(this)->connect_success(fd, id, locator, s);
	} catch (...) {
		::close(fd);
		throw;
	}
}


}  // namespace ccf

#endif /* ccf/session_manager.h */

