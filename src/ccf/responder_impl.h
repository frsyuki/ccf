//
// ccf::responder - Cluster Communication Framework
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
#ifndef CCF_RESPONDER_IMPL_H__
#define CCF_RESPONDER_IMPL_H__

#include "ccf/service.h"
#include <mp/object_callback.h>
#include <msgpack/vrefbuffer.hpp>

namespace ccf {


inline responder::responder(int fd, msgid_t msgid) :
	m_fd(fd), m_msgid(msgid) { }

inline responder::~responder() { }

template <typename Result, typename Error>
inline void responder::call(Result& res, Error& err)
{
	msgpack::sbuffer buf;  // FIXME use vrefbuffer?
	message_response<Result&, Error> msgres(res, err, m_msgid);
	msgpack::pack(buf, msgres);

	core::write(m_fd, buf.data(), buf.size(), &::free, buf.data());
	buf.release();
}

template <typename Result, typename Error>
inline void responder::call(Result& res, Error& err, auto_zone z)
{
	msgpack::vrefbuffer* buf = z->allocate<msgpack::vrefbuffer>();  // FIXME use sbuffer?
	message_response<Result&, Error> msgres(res, err, m_msgid);
	msgpack::pack(*buf, msgres);

	core::writev(m_fd, buf->vector(), buf->vector_size(), z);
}

template <typename Result>
void responder::result(Result res)
{
	msgpack::type::nil err;
	call(res, err);
}

template <typename Result>
void responder::result(Result res, auto_zone z)
{
	msgpack::type::nil err;
	call(res, err, z);
}

template <typename Error>
void responder::error(Error err)
{
	msgpack::type::nil res;
	call(res, err);
}

template <typename Error>
void responder::error(Error err, auto_zone z)
{
	msgpack::type::nil res;
	call(res, err, z);
}

inline void responder::null()
{
	msgpack::type::nil res;
	msgpack::type::nil err;
	call(res, err);
}


}  // namespace ccf

#endif /* ccf/responder_impl.h */

