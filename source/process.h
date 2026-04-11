#ifndef LC_STUB_PROCESS_H
#define LC_STUB_PROCESS_H

typedef unsigned(__stdcall *lc_beginthreadex_proc)(void*);
static inline void* _beginthreadex(void*, unsigned, lc_beginthreadex_proc, void*, unsigned, unsigned*) { return 0; }
static inline void _endthreadex(unsigned) {}

#endif
