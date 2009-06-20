//
// ccf::session - Cluster Communication Framework
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
#include "ccf/session.h"
#include "ccf/session_manager.h"
#include "ccf/service.h"
#include <cclog/cclog.h>
#include <algorithm>

namespace ccf {


namespace {

class callback_entry {
public:
	callback_entry();
	callback_entry(callback_t callback, shared_zone life,
			unsigned short timeout_steps);

public:
	void callback(msgobj res, msgobj err, auto_zone& z);
	void callback(msgobj res, msgobj err);
	inline void callback_submit(msgobj res, msgobj err);
	inline bool step_timeout();  // Note: NOT thread-safe

private:
	void callback_real(
			msgobj res, msgobj err, auto_zone z);

	static void callback_submit_real(
			callback_t callback,
			msgobj res, msgobj err, shared_zone life);

private:
	unsigned short m_timeout_steps;
	callback_t m_callback;
	shared_zone m_life;
};

class callback_table {
public:
	callback_table();
	~callback_table();

public:
	void insert(msgid_t msgid, const callback_entry& entry);
	bool out(msgid_t msgid, callback_entry* result);
	template <typename F> void for_each_clear(F f);
	template <typename F> void erase_if(F f);

private:
	static const size_t PARTITION_NUM = 4;  // FIXME
	typedef std::map<msgid_t, callback_entry> callbacks_t;
	mp::pthread_mutex m_callbacks_mutex[PARTITION_NUM];
	callbacks_t m_callbacks[PARTITION_NUM];

private:
	callback_table(const callback_table&);
};


callback_entry::callback_entry() { }

callback_entry::callback_entry(
		callback_t callback, shared_zone life,
		unsigned short timeout_steps) :
	m_timeout_steps(timeout_steps),
	m_callback(callback),
	m_life(life) { }


void callback_entry::callback_real(
		msgobj res, msgobj err, auto_zone z)
try {
	if(m_life) {
		z->allocate<shared_zone>(m_life);
	}

	m_callback(res, err, z);

} catch (std::exception& e) {
	LOG_ERROR("response callback error: ",e.what());
} catch (...) {
	LOG_ERROR("response callback error: unknown error");
}

void callback_entry::callback_submit_real(
		callback_t callback,
		msgobj res, msgobj err, shared_zone life)
try {
	auto_zone z(new msgpack::zone());

	if(life) {
		z->allocate<shared_zone>(life);
	}

	callback(res, err, z);

} catch (std::exception& e) {
	LOG_ERROR("response callback error: ",e.what());
} catch (...) {
	LOG_ERROR("response callback error: unknown error");
}


inline void callback_entry::callback(
		msgobj res, msgobj err,
		auto_zone& z)
{
	// msgpack::zone::push_finalizer is not thread-safe
	// m_life may null. see {basic_,}session::call
	//m_life->push_finalizer(&mp::object_delete<msgpack::zone>, z.release());
	callback_real(res, err, z);
}

inline void callback_entry::callback(
		msgobj res, msgobj err)
{
	auto_zone z(new msgpack::zone());
	callback_real(res, err, z);
}

inline void callback_entry::callback_submit(
		msgobj res, msgobj err)
{
	core::submit(&callback_entry::callback_submit_real,
			m_callback, res, err, m_life);
}


bool callback_entry::step_timeout()
{
	if(m_timeout_steps > 0) {
		--m_timeout_steps;  // FIXME atomic?
		return true;
	}
	return false;
}


callback_table::callback_table() { }

callback_table::~callback_table() { }

bool callback_table::out(
		msgid_t msgid, callback_entry* result)
{
	pthread_scoped_lock lk(m_callbacks_mutex[msgid % PARTITION_NUM]);

	callbacks_t& cbs(m_callbacks[msgid % PARTITION_NUM]);
	callbacks_t::iterator it(cbs.find(msgid));
	if(it == cbs.end()) {
		return false;
	}

	*result = it->second;
	cbs.erase(it);

	return true;
}

template <typename F>
void callback_table::for_each_clear(F f)
{
	for(size_t i=0; i < PARTITION_NUM; ++i) {
		pthread_scoped_lock lk(m_callbacks_mutex[i]);
		callbacks_t& cbs(m_callbacks[i]);
		std::for_each(cbs.begin(), cbs.end(), f);
		cbs.clear();
	}
}

template <typename F>
void callback_table::erase_if(F f)
{
	for(size_t i=0; i < PARTITION_NUM; ++i) {
		pthread_scoped_lock lk(m_callbacks_mutex[i]);
		callbacks_t& cbs(m_callbacks[i]);
		for(callbacks_t::iterator it(cbs.begin()), it_end(cbs.end());
				it != it_end; ) {
			if(f(*it)) {
				cbs.erase(it++);
			} else {
				++it;
			}
		}
		//cbs.erase(std::remove_if(cbs.begin(), cbs.end(), f), cbs.end());
	}
}

void callback_table::insert(
		msgid_t msgid, const callback_entry& entry)
{
	pthread_scoped_lock lk(m_callbacks_mutex[msgid % PARTITION_NUM]);
	std::pair<callbacks_t::iterator, bool> pair =
		m_callbacks[msgid % PARTITION_NUM].insert(
				callbacks_t::value_type(msgid, entry));
	if(!pair.second) {
		pair.first->second = entry;
	}
}

}  // noname namespace


#define ANON_m_cbtable \
	reinterpret_cast<callback_table*>(m_cbtable)


session::session(basic_session_manager* manager) :
	m_msgid_rr(0),  // FIXME randomize?
	m_connect_failed_count(0),
	m_manager(manager)
{
	m_cbtable = reinterpret_cast<void*>(new callback_table());
}

session::~session()
{
	msgpack::object res;
	res.type = msgpack::type::NIL;
	msgpack::object err;
	err.type = msgpack::type::POSITIVE_INTEGER;
	err.via.u64 = error_code::TRANSPORT_LOST_ERROR;

	force_lost(res, err);
	delete ANON_m_cbtable;
}


namespace {
	struct remove_if_step_timeout {
		remove_if_step_timeout()
		{
			res.type = msgpack::type::NIL;
			err.type = msgpack::type::POSITIVE_INTEGER;
			err.via.u64 = error_code::TIMEOUT_ERROR;
		}
		template <typename T>  // T = callback_entry
		bool operator() (T& pair)
		{
			if(!pair.second.step_timeout()) {
				LOG_DEBUG("callback timeout id=",pair.first);
				pair.second.callback_submit(res, err);  // client::step_timeout;
				//pair.second.callback(res, err);  // client::step_timeout;
				return true;
			}
			return false;
		}
	private:
		msgobj res;
		msgobj err;
	};
}  // noname namespace

void session::step_timeout(shared_session self)
{
	ANON_m_cbtable->erase_if(remove_if_step_timeout());
}


void session::process_response(
		msgobj result, msgobj error,
		msgid_t msgid, auto_zone& z)
{
	pthread_scoped_lock clk(m_connections_mutex);
	callback_entry e;
	LOG_DEBUG("process callback this=",(void*)this," id=",msgid," result:",result," error:",error);
	if(!ANON_m_cbtable->out(msgid, &e)) {
		LOG_DEBUG("callback not found id=",msgid);
		return;
	}
	e.callback(result, error, z);
}

void session::add_connection(int fd, const address& locator)
{
	m_connect_failed_count = 0;
	pthread_scoped_lock clk(m_connections_mutex);
	m_connections.push_back( con_t(fd, locator) );
	if(!m_pending_queue.empty()) {
		send_pending(clk);  // this unlocks clk
	}
}

namespace {
	struct remove_if_fd_equal {
		remove_if_fd_equal(int fd) : m_fd(fd) { }
		template <typename T>  // T = session::con_t
		bool operator() (const T& con)
		{
			return con.fd() == m_fd;
		}
	private:
		int m_fd;
		remove_if_fd_equal();
	};
}  // noname namespace

void session::remove_connection(int fd)
{
	pthread_scoped_lock clk(m_connections_mutex);
	// FIXME reconnect?
	m_connections.erase(
			std::remove_if(m_connections.begin(), m_connections.end(), remove_if_fd_equal(fd)),
			m_connections.end());

	if(m_connections.empty()) {
		clk.unlock();
		m_manager->session_unbound(shared_from_this());
	}
}

unsigned short session::connect_failed()
{
	return ++m_connect_failed_count;
}


namespace {
	struct each_callback_submit {
		each_callback_submit(msgobj r, msgobj e) :
			res(r), err(e) { }
		template <typename T>
		void operator() (T& pair) const
		{
			pair.second.callback_submit(res, err);
		}
	private:
		msgobj res;
		msgobj err;
		each_callback_submit();
	};
}

void session::force_lost(msgobj res, msgobj err)
{
	ANON_m_cbtable->for_each_clear(each_callback_submit(res, err));
}


void session::call_real(msgid_t msgid, std::auto_ptr<msgpack::vrefbuffer> buffer,
		shared_zone life, callback_t callback, unsigned short timeout_steps)
{
	//if(!life) { life.reset(new msgpack::zone()); }

	ANON_m_cbtable->insert(msgid, callback_entry(callback, life, timeout_steps));

	/* FIXME
	if(is_lost()) {
		//throw std::runtime_error("lost session");
		// FIXME XXX forget the error for robustness and wait timeout.
		return;
	}
	*/

	send_datav(buffer, life);
}


void session::send_datav(std::auto_ptr<msgpack::vrefbuffer> buffer,
		shared_zone life)
{
	pthread_scoped_lock clk(m_connections_mutex);

	if(m_connections.empty()) {
		if(!life) { life.reset(new msgpack::zone()); }
		pthread_scoped_lock plk(m_pending_queue_mutex);

		life->push_finalizer(&mp::object_destructor<msgpack::vrefbuffer>, buffer.get());
		msgpack::vrefbuffer* buf = buffer.release();

		m_pending_queue.push_back(
				pending_t(buf->vector(), buf->vector_size(), life)
				);
		return;
	}

#ifndef NO_AD_HOC_CONNECTION_LOAD_BALANCE
	int fd = m_connections[++m_msgid_rr % m_connections.size()].fd();
#else
	int fd = m_connections[0].fd();
#endif
	// FIXME clk.unlock() ?
	if(life) {
		life->push_finalizer(&mp::object_delete<msgpack::vrefbuffer>, buffer.get());
		msgpack::vrefbuffer* buf = buffer.release();
		core::writev(fd, buf->vector(), buf->vector_size(), life);
	} else {
		core::writev(fd, buffer->vector(), buffer->vector_size(), buffer);
	}
}

void session::send_data(msgpack::sbuffer& buffer,
		shared_zone life)
{
	pthread_scoped_lock clk(m_connections_mutex);

	if(m_connections.empty()) {
		if(!life) { life.reset(new msgpack::zone()); }
		pthread_scoped_lock plk(m_pending_queue_mutex);

		life->push_finalizer(&::free, buffer.data());
		size_t size = buffer.size();
		void* data  = buffer.release();

		struct iovec *vec = (struct iovec*)life->malloc(
				sizeof(struct iovec));
		vec->iov_len  = size;
		vec->iov_base = data;
		m_pending_queue.push_back(
				pending_t(vec, 1, life)
				);
		return;
	}

#ifndef NO_AD_HOC_CONNECTION_LOAD_BALANCE
	int fd = m_connections[++m_msgid_rr % m_connections.size()].fd();
#else
	int fd = m_connections[0].fd();
#endif
	// FIXME clk.unlock() ?
	core::write(fd, buffer.data(), buffer.size(), &::free, buffer.data());
	buffer.release();
}


void session::send_pending(pthread_scoped_lock& clk)
{
	pending_queue_t pendings;
	{
		pthread_scoped_lock plk(m_pending_queue_mutex);
		pendings.swap(m_pending_queue);
	}
	clk.unlock();

	for(pending_queue_t::iterator it(pendings.begin()),
			it_end(pendings.end()); it != it_end; ++it) {
#ifndef NO_AD_HOC_CONNECTION_LOAD_BALANCE
		int fd = m_connections[++m_msgid_rr % m_connections.size()].fd();
#else
		int fd = m_connections[0].fd();
#endif
		core::writev(fd, it->vec, it->veclen, it->life);
	}
}


}  // namespace ccf

