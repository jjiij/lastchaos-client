#ifndef LC_STUB_MMSYSTEM_H
#define LC_STUB_MMSYSTEM_H

#include <cstdint>

typedef std::uint32_t MMRESULT;
#define TIMERR_NOERROR 0

static inline std::uint32_t timeGetTime() {
  return GetTickCount();
}

#endif
