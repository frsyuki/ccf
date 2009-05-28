//
// mp::wavy::service
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

#ifndef MP_WAVY_SERVICE_H__
#define MP_WAVY_SERVICE_H__

#include "mp/wavy/core.h"
#include "mp/wavy/net.h"

namespace mp {
namespace wavy {


template <typename Instance>
struct service {

	struct core {
		static void init(size_t threads = 0);

		static void add_thread(size_t num);

		static void join();
		static void detach();
		static void end();

		typedef wavy::core::handler handler;

		typedef wavy::core::connect_callback_t connect_callback_t;
		static void connect(
				int socket_family, int socket_type, int protocol,
				const sockaddr* addr, socklen_t addrlen,
				int timeout_msec, connect_callback_t callback);
	
		typedef wavy::core::listen_callback_t listen_callback_t;
		static void listen(int lsock, listen_callback_t callback);
	
		typedef wavy::core::timer_callback_t timer_callback_t;
		static void timer(const timespec* interval, timer_callback_t callback);

		template <typename Handler>
		static Handler* add_handler(int fd);
MP_ARGS_BEGIN
		template <typename Handler, MP_ARGS_TEMPLATE>
		static Handler* add_handler(int fd, MP_ARGS_PARAMS);
MP_ARGS_END

		template <typename F>
		static void submit(F f);
MP_ARGS_BEGIN
		template <typename F, MP_ARGS_TEMPLATE>
		static void submit(F f, MP_ARGS_PARAMS);
MP_ARGS_END

	private:
		static wavy::core* s_core;

		core();
	};

	struct net {
		static void init(size_t threads = 0);

		static void add_thread(size_t num);

		static void join();
		static void detach();
		static void end();

		typedef wavy::net::finalize_t finalize_t;
		typedef wavy::net::xfer xfer;

		static void send(int sock, const void* buf, size_t count);
	
		static void send(int sock, const void* buf, size_t count,
				finalize_t fin, void* user);
	
		static void send(int sock, const struct iovec* vec, size_t veclen,
				finalize_t fin, void* user);
	
		static void send(int sock, int fd, uint64_t offset, size_t count,
				finalize_t fin, void* user);
	
		static void send(int sock,
				const void* header, size_t header_len,
				int fd, uint64_t offset, size_t count,
				finalize_t fin, void* user);
	
		static void send(int sock,
				const struct iovec* header_vec, size_t header_veclen,
				int fd, uint64_t offset, size_t count,
				finalize_t fin, void* user);
	
		static void send(int sock, xfer* xf);

	private:
		static wavy::net* s_net;

		net();
	};

private:
	service();
};


template <typename Instance>
core* service<Instance>::core::s_core;

template <typename Instance>
net* service<Instance>::net::s_net;

template <typename Instance>
void service<Instance>::core::init(size_t threads)
{
	s_core = new wavy::core();
	add_thread(threads);
}

template <typename Instance>
void service<Instance>::net::init(size_t threads)
{
	s_net = new wavy::net();
	add_thread(threads);
}

template <typename Instance>
void service<Instance>::core::add_thread(size_t num)
	{ s_core->add_thread(num); }

template <typename Instance>
void service<Instance>::net::add_thread(size_t num)
	{ s_net->add_thread(num); }

template <typename Instance>
void service<Instance>::core::join()
{
	s_core->join();
}

template <typename Instance>
void service<Instance>::net::join()
{
	s_net->join();
}

template <typename Instance>
void service<Instance>::core::detach()
{
	s_core->core::detach();
}

template <typename Instance>
void service<Instance>::net::detach()
{
	s_net->net::detach();
}

template <typename Instance>
void service<Instance>::core::end()
{
	s_core->core::end();
}

template <typename Instance>
void service<Instance>::net::end()
{
	s_net->end();
}


template <typename Instance>
inline void service<Instance>::core::connect(
		int socket_family, int socket_type, int protocol,
		const sockaddr* addr, socklen_t addrlen,
		int timeout_msec, connect_callback_t callback)
{
	s_core->connect(socket_family, socket_type, protocol,
			addr, addrlen, timeout_msec, callback);
}


template <typename Instance>
inline void service<Instance>::core::listen(int lsock, listen_callback_t callback)
	{ s_core->listen(lsock, callback); }


template <typename Instance>
inline void service<Instance>::core::timer(
		const timespec* interval, timer_callback_t callback)
	{ s_core->timer(interval, callback); }


template <typename Instance>
inline void service<Instance>::net::send(int sock, const void* buf, size_t count)
	{ s_net->send(sock, buf, count); }

template <typename Instance>
inline void service<Instance>::net::send(int sock, const void* buf, size_t count,
		finalize_t fin, void* user)
	{ s_net->send(sock, buf, count, fin, user); }

template <typename Instance>
inline void service<Instance>::net::send(int sock, const struct iovec* vec, size_t veclen,
		finalize_t fin, void* user)
	{ s_net->send(sock, vec, veclen, fin, user); }

template <typename Instance>
inline void service<Instance>::net::send(int sock, int fd, uint64_t offset, size_t count,
		finalize_t fin, void* user)
	{ s_net->send(sock, fd, offset, count, fin, user); }

template <typename Instance>
inline void service<Instance>::net::send(int sock,
		const void* header, size_t header_len,
		int fd, uint64_t offset, size_t count,
		finalize_t fin, void* user)
	{ s_net->send(sock, header, header_len, fd, offset, count, fin, user); }

template <typename Instance>
inline void service<Instance>::net::send(int sock,
		const struct iovec* header_vec, size_t header_veclen,
		int fd, uint64_t offset, size_t count,
		finalize_t fin, void* user)
	{ s_net->send(sock, header_vec, header_veclen, fd, offset, count, fin, user); }

template <typename Instance>
inline void service<Instance>::net::send(int sock, xfer* xf)
	{ s_net->send(sock, xf); }



template <typename Instance>
template <typename Handler>
inline Handler* service<Instance>::core::add_handler(int fd)
	{ return s_core->add<Handler>(fd); }
MP_ARGS_BEGIN
template <typename Instance>
template <typename Handler, MP_ARGS_TEMPLATE>
inline Handler* service<Instance>::core::add_handler(int fd, MP_ARGS_PARAMS)
	{ return s_core->add<Handler, MP_ARGS_TYPES>(fd, MP_ARGS_FUNC); }
MP_ARGS_END

template <typename Instance>
template <typename F>
inline void service<Instance>::core::submit(F f)
	{ s_core->submit<F>(f); }
MP_ARGS_BEGIN
template <typename Instance>
template <typename F, MP_ARGS_TEMPLATE>
inline void service<Instance>::core::submit(F f, MP_ARGS_PARAMS)
	{ s_core->submit<F, MP_ARGS_TYPES>(f, MP_ARGS_FUNC); }
MP_ARGS_END


}  // namespace wavy
}  // namespace mp

#endif /* mp/wavy/service.h */

