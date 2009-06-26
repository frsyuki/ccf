//
// ccf::types - Cluster Communication Framework
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
#ifndef CCF_TYPES_H__
#define CCF_TYPES_H__

#include <msgpack.hpp>
#include <mp/memory.h>
#include <mp/pthread.h>

namespace ccf {


typedef uint32_t msgid_t;
typedef uint32_t method_t;


typedef msgpack::object msgobj;
typedef std::auto_ptr<msgpack::zone> auto_zone;
typedef mp::shared_ptr<msgpack::zone> shared_zone;

class session;
typedef mp::shared_ptr<session> shared_session;
typedef mp::weak_ptr<session> weak_session;

using mp::pthread_scoped_lock;
using mp::pthread_scoped_rdlock;
using mp::pthread_scoped_wrlock;


}  // namespace ccf

#endif /* ccf/types.h */

