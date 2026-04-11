#ifndef LC_STUB_TLHELP32_H
#define LC_STUB_TLHELP32_H

typedef struct tagPROCESSENTRY32 {
  unsigned long dwSize;
} PROCESSENTRY32;

#define TH32CS_SNAPPROCESS 0

static inline void* CreateToolhelp32Snapshot(unsigned long, unsigned long) { return 0; }
static inline int Process32First(void*, PROCESSENTRY32*) { return 0; }
static inline int Process32Next(void*, PROCESSENTRY32*) { return 0; }

#endif
