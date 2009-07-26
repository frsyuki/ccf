#include <stdio.h>
#include <unwind.h>
#include <dlfcn.h>

static _Unwind_Reason_Code print_stacktrace(struct _Unwind_Context* ctx, void*)
{
	void* p = reinterpret_cast<void*>( _Unwind_GetIP(ctx) );
	Dl_info info;
	if(dladdr(p, &info) && info.dli_saddr) {
		long d = reinterpret_cast<long>(p) - reinterpret_cast<long>(info.dli_saddr);
		printf("%p %s+0x%lx\n", p, info.dli_sname, d);
	}
	return _URC_NO_REASON;
}

extern "C"
void __real___cxa_throw(void* thrown_exception, std::type_info* tinfo,
		void (*dest)(void*)) __attribute__(( noreturn ));

extern "C"
void __wrap___cxa_throw(void* thrown_exception, std::type_info* tinfo,
		void (*dest)(void*))
{
	_Unwind_Backtrace(print_stacktrace, 0);
	__real___cxa_throw(thrown_exception, tinfo, dest);
}

