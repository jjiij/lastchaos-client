#ifndef LC_STUB_SHLWAPI_H
#define LC_STUB_SHLWAPI_H

#include <unistd.h>

static inline int PathFileExistsA(const char* p) {
  if (p == nullptr || p[0] == '\0') {
    return 0;
  }
  return access(p, F_OK) == 0 ? 1 : 0;
}
static inline int PathFileExists(const char* p) { return PathFileExistsA(p); }

#endif
