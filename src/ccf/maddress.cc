#include "ccf/maddress.h"
#include <stdexcept>

namespace ccf {


#if 0
// FIXME maddress
void maddress::expand(size_t req)
{
	size_t csize = size();
	size_t nsize = csize * 2;
	while(nsize < req) { nsize *= 2; }

	address* tmp;
	if(m_begin == m_fixbuf) {
		tmp = (address*)::malloc(sizeof(address)*nsize);
		if(!tmp) { throw std::bad_alloc(); }
		memcpy(tmp, m_fixbuf, sizeof(address)*CCF_MADDRESS_FIXBUF_SIZE);

	} else {
		tmp  = (address*)::realloc(m_begin, sizeof(address)*nsize);
		if(!tmp) { throw std::bad_alloc(); }
	}

	m_begin     = tmp;
	m_end       = tmp + csize;
	m_alloc_end = tmp + nsize;
}
#endif

std::ostream& operator<< (std::ostream& stream, const maddress& maddr)
{
	stream << '[';
	if(!maddr.empty()) {
		maddress::const_iterator it(maddr.begin());
		stream << *it;
		++it;
		for(maddress::const_iterator it_end(maddr.end());
				it != it_end; ++it) {
			stream << ", " << *it;
		}
	}
	stream << ']';
}


}  // namespace ccf

