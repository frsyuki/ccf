//
// ccf::maddress - Cluster Communication Framework
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
#ifndef CCF_MADDRESS_H__
#define CCF_MADDRESS_H__

#include "ccf/address.h"
#include <stdlib.h>

#include <vector>

#ifndef CCF_MADDRESS_FIXBUF_SIZE
#define CCF_MADDRESS_FIXBUF_SIZE 4
#endif

namespace ccf {


typedef std::vector<address> maddress;
#if 0
// FIXME maddress
class maddress {
public:
	maddress() :
		m_begin(m_fixbuf), m_end(m_begin + CCF_MADDRESS_FIXBUF_SIZE),
		m_alloc_end(m_end) { }

	~maddress()
	{
		if(m_begin != m_fixbuf) { ::free(m_begin); }
	}

	void push_back(address a)
	{
		if(m_end == m_alloc_end) {
			expand(capacity() + 1);
		}
		*m_end = a;
		++m_end;
	}

	size_t size() const
	{
		return m_end - m_begin;
	}

	size_t capacity() const
	{
		return m_alloc_end - m_begin;
	}

	void resize(size_t after)
	{
		size_t current = size();
		if(after < current) {
			m_end -= current - after;
			return;
		}
		if(after > capacity()) {
			expand(after);
		}
		m_end = m_begin + after;
	}

	address& operator[] (size_t i)
	{
		return m_begin[i];
	}

	const address& operator[] (size_t i) const
	{
		return m_begin[i];
	}

	typedef address* iterator;

	address* begin() { return m_begin; }
	const address* begin() const { return m_begin; }

	address* end() { return m_end; }
	const address* end() const { return m_end; }

private:
	void expand(size_t req);

private:
	address* m_begin;
	address* m_end;
	address* m_alloc_end;
	address m_fixbuf[CCF_MADDRESS_FIXBUF_SIZE];
};
#endif


}  // namespace ccf

#endif /* ccf/maddress.h */

