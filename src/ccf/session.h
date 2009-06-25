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
#ifndef CCF_SESSION_H__
#define CCF_SESSION_H__

#include "ccf/types.h"
#include "ccf/protocol.h"
#include "ccf/address.h"
#include <cclog/cclog.h>
#include <mp/memory.h>
#include <msgpack/vrefbuffer.hpp>

namespace ccf {


class basic_session_manager;

class session : public mp::enable_shared_from_this<session> {
public:
	// from session_manager::create_session
	session(basic_session_manager* manager);

	virtual ~session();

public:
	// step callback timeout count.
	// from session_manager
	// FIXME virtual?
	virtual void step_timeout(shared_session self);

	// call remote procedure.
	// if this session is not connected, it will try
	// to connect asynchronously.
	// Message is requred to inherit rpc::message.
	template <typename Message>
	void call(Message& params,
			shared_zone life, callback_t callback,
			unsigned short timeout_steps = 10);

	// process RPC response.
	// from managed_connection::process_response
	void process_response(
			msgobj result, msgobj error,
			msgid_t msgid, auto_zone& z);

	// add connection.
	// from managed_connection::managed_connection
	void add_connection(int fd);

	// remove connection.
	// from managed_connection::~managed_connection
	void remove_connection(int fd);

	// increment connect failed count
	// from client::connect_callback
	unsigned short connect_failed();

	// return true if connected
	// from session_manager::session_created
	bool is_connected() const { return !m_connections.empty(); }

	// from session_responder::call
	// from call_real
	void send_datav(std::auto_ptr<msgpack::vrefbuffer> buffer,
			shared_zone life);

	// from session_responder::call
	void send_data(msgpack::sbuffer& buffer,
			shared_zone life);

protected:
	void force_lost(msgobj res, msgobj err);

protected:
	template <typename Message>
	msgid_t pack(msgpack::vrefbuffer& buffer, Message& param);

	void call_real(msgid_t msgid, std::auto_ptr<msgpack::vrefbuffer> buffer,
			shared_zone life, callback_t callback, unsigned short timeout_steps);

protected:
	msgid_t m_msgid_rr;

	void* m_cbtable;  // anonymous:session.cc

	struct con_t {
		con_t(int fd) : m_fd(fd) { }
	public:
		int fd() const { return m_fd; }
	private:
		int m_fd;
	};

	mp::pthread_mutex m_connections_mutex;
	typedef std::vector<con_t> connections_t;
	connections_t m_connections;

	unsigned short m_connect_failed_count;

private:
	// Note: this funciton unlocks clk
	void send_pending(pthread_scoped_lock& clk);

	struct pending_t {
		pending_t(const struct iovec* vec_, size_t veclen_, shared_zone life_) :
			vec(vec_), veclen(veclen_), life(life_) { }
		pending_t() { }
		~pending_t() { }
		const struct iovec* vec;
		size_t veclen;
		shared_zone life;
	};

	mp::pthread_mutex m_pending_queue_mutex;
	typedef std::vector<pending_t> pending_queue_t;
	pending_queue_t m_pending_queue;

private:
	basic_session_manager* m_manager;

private:
	session();
	session(const session&);
};


template <typename Message>
inline msgid_t session::pack(msgpack::vrefbuffer& buffer, Message& param)
{
	msgid_t msgid = __sync_add_and_fetch(&m_msgid_rr, 1);
	message_request<Message> msgreq(Message::method, param, msgid);
	msgpack::pack(buffer, msgreq);
	return msgid;
}


template <typename Message>
inline void session::call(
		Message& param,
		shared_zone life, callback_t callback,
		unsigned short timeout_steps)
{
	LOG_DEBUG("send request method=",Message::method);

	std::auto_ptr<msgpack::vrefbuffer> buffer(new msgpack::vrefbuffer());
	msgid_t msgid = pack(*buffer, param);

	call_real(msgid, buffer, life, callback, timeout_steps);
}


}  // namespace ccf

#endif /* ccf/session.h */

