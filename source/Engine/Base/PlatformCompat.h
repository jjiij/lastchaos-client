#ifndef SE_INCL_PLATFORM_COMPAT_H
#define SE_INCL_PLATFORM_COMPAT_H

#if !defined(_WIN32) && !defined(_WIN64)

#include <atomic>
#include <cerrno>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <dlfcn.h>
#include <fcntl.h>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <string>
#include <thread>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>

#ifndef WINAPI
#define WINAPI
#endif

#ifndef PASCAL
#define PASCAL
#endif

#ifndef CALLBACK
#define CALLBACK
#endif

#ifndef APIENTRY
#define APIENTRY
#endif

#ifndef TEXT
#define TEXT(x) x
#endif

#ifndef MAX_PATH
#define MAX_PATH 4096
#endif

#ifndef MK_LBUTTON
#define MK_LBUTTON 0x0001
#endif

#ifndef EM_ZERODIVIDE
#define EM_ZERODIVIDE 0x00000008
#endif

#ifndef MCW_EM
#define MCW_EM 0x0008001f
#endif

typedef void* HANDLE;
typedef void* HMODULE;
typedef void* FARPROC;
typedef uintptr_t DWORD_PTR;
typedef uintptr_t ULONG_PTR;
typedef intptr_t LONG_PTR;
typedef int SOCKET;

#ifndef INVALID_SOCKET
#define INVALID_SOCKET (-1)
#endif

struct MSG {
  void* hwnd;
  std::uint32_t message;
  std::uintptr_t wParam;
  std::intptr_t lParam;
  std::uint32_t time;
  long pt_x;
  long pt_y;
};

struct SECompatEvent {
  bool manual_reset = false;
  bool signaled = false;
  std::mutex mutex;
  std::condition_variable cv;
};

static constexpr std::uint32_t WAIT_OBJECT_0 = 0;
static constexpr std::uint32_t WAIT_TIMEOUT = 258;
static constexpr std::uint32_t WAIT_FAILED = 0xFFFFFFFFu;
static constexpr std::uint32_t INFINITE = 0xFFFFFFFFu;
static const HANDLE INVALID_HANDLE_VALUE = reinterpret_cast<HANDLE>(static_cast<intptr_t>(-1));

static inline HANDLE CreateEvent(void*, bool manualReset, bool initialState, const char*) {
  auto* event = new SECompatEvent();
  event->manual_reset = manualReset;
  event->signaled = initialState;
  return reinterpret_cast<HANDLE>(event);
}

static inline bool CloseHandle(HANDLE handle) {
  if (handle == nullptr || handle == INVALID_HANDLE_VALUE) {
    return false;
  }
  delete reinterpret_cast<SECompatEvent*>(handle);
  return true;
}

static inline bool SetEvent(HANDLE handle) {
  if (handle == nullptr || handle == INVALID_HANDLE_VALUE) {
    return false;
  }
  auto* event = reinterpret_cast<SECompatEvent*>(handle);
  {
    std::lock_guard<std::mutex> lock(event->mutex);
    event->signaled = true;
  }
  event->cv.notify_all();
  return true;
}

static inline bool ResetEvent(HANDLE handle) {
  if (handle == nullptr || handle == INVALID_HANDLE_VALUE) {
    return false;
  }
  auto* event = reinterpret_cast<SECompatEvent*>(handle);
  std::lock_guard<std::mutex> lock(event->mutex);
  event->signaled = false;
  return true;
}

static inline std::uint32_t WaitForSingleObject(HANDLE handle, std::uint32_t milliseconds) {
  if (handle == nullptr || handle == INVALID_HANDLE_VALUE) {
    return WAIT_FAILED;
  }

  auto* event = reinterpret_cast<SECompatEvent*>(handle);
  std::unique_lock<std::mutex> lock(event->mutex);
  if (!event->signaled) {
    if (milliseconds == INFINITE) {
      event->cv.wait(lock, [&] { return event->signaled; });
    } else {
      const auto timeout = std::chrono::milliseconds(milliseconds);
      if (!event->cv.wait_for(lock, timeout, [&] { return event->signaled; })) {
        return WAIT_TIMEOUT;
      }
    }
  }

  if (!event->manual_reset) {
    event->signaled = false;
  }
  return WAIT_OBJECT_0;
}

static inline void Sleep(std::uint32_t milliseconds) {
  std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

static inline std::uint32_t GetCurrentThreadId() {
  static std::atomic<std::uint32_t> next_id{1};
  thread_local const std::uint32_t thread_id = next_id.fetch_add(1, std::memory_order_relaxed);
  return thread_id;
}

static inline std::uint32_t TlsAlloc() {
  static std::atomic<std::uint32_t> next_slot{1};
  return next_slot.fetch_add(1, std::memory_order_relaxed);
}

static inline bool TlsFree(std::uint32_t) {
  return true;
}

static inline bool TlsSetValue(std::uint32_t slot, void* value) {
  thread_local std::unordered_map<std::uint32_t, void*> tls_values;
  tls_values[slot] = value;
  return true;
}

static inline void* TlsGetValue(std::uint32_t slot) {
  thread_local std::unordered_map<std::uint32_t, void*> tls_values;
  const auto it = tls_values.find(slot);
  if (it == tls_values.end()) {
    return nullptr;
  }
  return it->second;
}

template <typename T>
static inline long InterlockedIncrement(volatile T* value) {
  return static_cast<long>(__sync_add_and_fetch(const_cast<T*>(value), 1));
}

template <typename T>
static inline long InterlockedDecrement(volatile T* value) {
  return static_cast<long>(__sync_sub_and_fetch(const_cast<T*>(value), 1));
}

static inline unsigned int _controlfp(unsigned int newval, unsigned int mask) {
  (void)newval;
  (void)mask;
  return 0;
}

static inline int WSAStartup(std::uint16_t, void*) {
  return 0;
}

static inline int WSACleanup() {
  return 0;
}

static inline int WSAGetLastError() {
  return errno;
}

static inline int closesocket(int socket_fd) {
  return ::close(socket_fd);
}

static inline int ioctlsocket(int socket_fd, long request, unsigned long* argument) {
  if (request != FIONBIO) {
    errno = ENOTSUP;
    return -1;
  }

  const int flags = fcntl(socket_fd, F_GETFL, 0);
  if (flags < 0) {
    return -1;
  }

  int updated_flags = flags;
  if (argument != nullptr && *argument != 0) {
    updated_flags |= O_NONBLOCK;
  } else {
    updated_flags &= ~O_NONBLOCK;
  }

  return fcntl(socket_fd, F_SETFL, updated_flags);
}

static inline void* LoadLibraryA(const char* library_name) {
  return dlopen(library_name, RTLD_NOW | RTLD_LOCAL);
}

static inline void* LoadLibrary(const char* library_name) {
  return LoadLibraryA(library_name);
}

static inline void* GetProcAddress(void* library_handle, const char* symbol_name) {
  return dlsym(library_handle, symbol_name);
}

static inline bool FreeLibrary(void* library_handle) {
  if (library_handle == nullptr) {
    return false;
  }
  return dlclose(library_handle) == 0;
}

static inline std::uint32_t GetLastError() {
  return static_cast<std::uint32_t>(errno);
}

static inline void SetLastError(std::uint32_t error_code) {
  errno = static_cast<int>(error_code);
}

// Win32 UI and System stubs
#define MB_OK 0x00000000L
#define SM_CXSCREEN 0
#define SM_CYSCREEN 1
#define SW_MINIMIZE 6
#define SW_SHOWNORMAL 1
#define DLL_PROCESS_DETACH 0
#define DLL_PROCESS_ATTACH 1
#define DLL_THREAD_ATTACH 2
#define DLL_THREAD_DETACH 3
#define WA_INACTIVE 0
#define WA_ACTIVE 1
#define WA_CLICKACTIVE 2
#define WM_MOUSEFIRST 0x0200
#define WM_MOUSELAST 0x0209
#define WM_KEYDOWN 0x0100
#define WM_CHAR 0x0102
#define WM_SYSKEYDOWN 0x0104
#define WM_SYSCOMMAND 0x0112
#define WM_QUIT 0x0012
#define WM_CLOSE 0x0010
#define WM_ACTIVATE 0x0006
#define WM_CANCELMODE 0x001F
#define WM_KILLFOCUS 0x0008
#define WM_ACTIVATEAPP 0x001C
#define WM_SETFOCUS 0x0007
#define WM_IME_NOTIFY 0x0282
#define IMN_CLOSESTATUSWINDOW 0x0001
#define WM_LBUTTONDOWN 0x0201
#define WM_RBUTTONDOWN 0x0204
#define SC_MINIMIZE 0xF020
#define SC_RESTORE 0xF120
#define SC_MAXIMIZE 0xF030
#define PM_REMOVE 0x0001
#define VK_F10 0x79
#define GMEM_MOVEABLE 0x0002

#ifndef _RECT_DEFINED
#define _RECT_DEFINED
typedef struct {
  long left;
  long top;
  long right;
  long bottom;
} RECT;
#endif

#ifndef _POINT_DEFINED
#define _POINT_DEFINED
typedef struct tagPOINT {
  long x;
  long y;
} POINT, *PPOINT, *LPPOINT;
#endif

typedef struct _FILETIME {
  std::uint32_t dwLowDateTime;
  std::uint32_t dwHighDateTime;
} FILETIME, *PFILETIME, *LPFILETIME;

typedef struct _SYSTEMTIME {
  std::uint16_t wYear;
  std::uint16_t wMonth;
  std::uint16_t wDayOfWeek;
  std::uint16_t wDay;
  std::uint16_t wHour;
  std::uint16_t wMinute;
  std::uint16_t wSecond;
  std::uint16_t wMilliseconds;
} SYSTEMTIME, *PSYSTEMTIME, *LPSYSTEMTIME;

typedef struct _OSVERSIONINFOA {
  std::uint32_t dwOSVersionInfoSize;
  std::uint32_t dwMajorVersion;
  std::uint32_t dwMinorVersion;
  std::uint32_t dwBuildNumber;
  std::uint32_t dwPlatformId;
  char szCSDVersion[128];
} OSVERSIONINFOA, OSVERSIONINFO, *POSVERSIONINFOA, *POSVERSIONINFO;

#ifndef _MEMORYSTATUS_DEFINED
#define _MEMORYSTATUS_DEFINED
typedef struct _MEMORYSTATUS {
  std::uint32_t dwLength;
  std::size_t dwTotalPhys;
  std::size_t dwAvailPhys;
  std::size_t dwTotalPageFile;
  std::size_t dwAvailPageFile;
  std::size_t dwTotalVirtual;
  std::size_t dwAvailVirtual;
} MEMORYSTATUS, *LPMEMORYSTATUS;
#endif

typedef struct _MEMORY_BASIC_INFORMATION {
  void* BaseAddress;
  void* AllocationBase;
  std::uint32_t AllocationProtect;
  std::size_t RegionSize;
  std::uint32_t State;
  std::uint32_t Protect;
  std::uint32_t Type;
} MEMORY_BASIC_INFORMATION, *PMEMORY_BASIC_INFORMATION;

#define VER_PLATFORM_WIN32s 0
#define VER_PLATFORM_WIN32_WINDOWS 1
#define VER_PLATFORM_WIN32_NT 2

#define PAGE_READWRITE 0x04
#define PAGE_WRITECOPY 0x08
#define PAGE_EXECUTE_READWRITE 0x40
#define PAGE_EXECUTE_WRITECOPY 0x80
#define MEM_COMMIT 0x1000
#define MEM_PRIVATE 0x20000

static inline int MessageBox(void*, const char*, const char*, std::uint32_t) { return 1; }
static inline int MessageBoxW(void*, const wchar_t*, const wchar_t*, std::uint32_t) { return 1; }
static inline int GetSystemMetrics(int) { return 1024; } // Mock value
static inline void* FindWindow(const char*, const char*) { return nullptr; }
static inline void* LoadIcon(void*, const char*) { return nullptr; }
static inline std::uintptr_t SetClassLongPtr(void*, int, long) { return 0; }
static inline std::uint32_t GetModuleFileName(void*, char* path, std::uint32_t size) { 
  if (size > 0) path[0] = '\0';
  return 0; 
}
static inline std::uint32_t GetFileVersionInfoSize(const char*, std::uint32_t*) { return 0; }
static inline void* GlobalAlloc(std::uint32_t, std::uint32_t) { return nullptr; }
static inline void* GlobalLock(void*) { return nullptr; }
static inline int GetFileVersionInfo(const char*, std::uint32_t, std::uint32_t, void*) { return 0; }
static inline int VerQueryValue(void*, const char*, void**, std::uint32_t*) { return 0; }
static inline int GlobalUnlock(void*) { return 0; }
static inline void GlobalFree(void*) {}
static inline void ShowCursor(int) {}
static inline std::uint32_t MapVirtualKey(std::uintptr_t vk, std::uint32_t flags) {
  (void)vk;
  (void)flags;
  return 0;
}
static inline std::intptr_t SendMessage(void*, std::uint32_t, std::uintptr_t, std::intptr_t) { return 0; }
static inline bool PeekMessage(MSG*, void*, std::uint32_t, std::uint32_t, std::uint32_t) { return false; }
static inline bool TranslateMessage(const MSG*) { return false; }
static inline std::intptr_t DispatchMessage(const MSG*) { return 0; }
static inline bool ShowWindow(void*, int) { return true; }
static inline void* SetFocus(void*) { return nullptr; }
static inline bool IsIconic(void*) { return false; }
static inline bool GetClientRect(void*, RECT* rect) { 
  if (rect) { rect->left = rect->top = 0; rect->right = 1024; rect->bottom = 768; }
  return true; 
}
static inline void SetRectEmpty(RECT* rect) {
  if (rect) {
    rect->left = rect->top = rect->right = rect->bottom = 0;
  }
}

typedef void* HCURSOR;
typedef void* HKL;
#ifndef IDC_ARROW
#define IDC_ARROW ((const char*)32512)
#endif
static inline HCURSOR LoadCursor(void*, const char* id) {
  (void)id;
  return nullptr;
}
static inline void SetCursor(HCURSOR) {}

#ifndef PRIMARYLANGID
#define PRIMARYLANGID(lid) ((std::uint16_t)((std::uintptr_t)(lid)) & 0x3ffu)
#endif
#ifndef SUBLANGID
#define SUBLANGID(lid) ((std::uint16_t)(((std::uintptr_t)(lid)) >> 10))
#endif
#ifndef MAKELANGID
#define MAKELANGID(primary, sub) \
  ((std::uint16_t)((((std::uint16_t)(sub)) << 10) | ((std::uint16_t)(primary) & 0x3ffu)))
#endif
#ifndef LANG_CHINESE
#define LANG_CHINESE 0x04
#endif
#ifndef SUBLANG_CHINESE_TRADITIONAL
#define SUBLANG_CHINESE_TRADITIONAL 0x01
#endif
#ifndef SUBLANG_CHINESE_SIMPLIFIED
#define SUBLANG_CHINESE_SIMPLIFIED 0x02
#endif

static inline std::uint32_t GetCurrentDirectory(std::uint32_t size, char* buffer) {
  if (size > 0 && buffer) { getcwd(buffer, size); return strlen(buffer); }
  return 0;
}
static inline bool DeleteFile(const char* path) { return unlink(path) == 0; }
static inline bool MoveFile(const char* old_path, const char* new_path) { return rename(old_path, new_path) == 0; }
static inline std::uint16_t GetSystemDefaultLangID() { return 0x0409; } // en-US
static inline std::uint32_t GetCurrentProcessId() { return static_cast<std::uint32_t>(getpid()); }
static inline std::uint32_t GetTickCount() { 
  auto now = std::chrono::steady_clock::now();
  return std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
}
static inline void GlobalMemoryStatus(MEMORYSTATUS* ms) {
  if (!ms) return;
  memset(ms, 0, sizeof(*ms));
  ms->dwLength = sizeof(*ms);
  ms->dwTotalPhys = 8ull * 1024ull * 1024ull * 1024ull;
  ms->dwAvailPhys = ms->dwTotalPhys / 2;
  ms->dwTotalPageFile = ms->dwTotalPhys;
  ms->dwAvailPageFile = ms->dwAvailPhys;
  ms->dwTotalVirtual = ms->dwTotalPhys;
  ms->dwAvailVirtual = ms->dwAvailPhys;
}
static inline int GetVolumeInformation(const char*, char*, std::uint32_t, std::uint32_t* serial, std::uint32_t*, std::uint32_t*, char*, std::uint32_t) {
  if (serial) *serial = 0;
  return 1;
}
static inline int GetDiskFreeSpace(const char*, std::uint32_t* sectors, std::uint32_t* bytes, std::uint32_t* free_clusters, std::uint32_t* clusters) {
  if (sectors) *sectors = 64;
  if (bytes) *bytes = 512;
  if (free_clusters) *free_clusters = 1024;
  if (clusters) *clusters = 2048;
  return 1;
}
static inline void* GetModuleHandle(const char*) { return nullptr; }
static inline std::size_t VirtualQuery(const void*, MEMORY_BASIC_INFORMATION*, std::size_t) { return 0; }

#define GENERIC_WRITE 0x40000000L
#define GENERIC_READ 0x80000000L
#define FILE_SHARE_READ 0x00000001L
#define CREATE_ALWAYS 2
#define OPEN_EXISTING 3
#define FILE_ATTRIBUTE_NORMAL 0x00000080
#define FILE_FLAG_DELETE_ON_CLOSE 0x04000000
#define FILE_FLAG_NO_BUFFERING 0x20000000
static inline void* CreateFile(const char*, std::uint32_t, std::uint32_t, void*, std::uint32_t, std::uint32_t, void*) { return nullptr; }
static inline std::uint32_t GetFileSize(void*, std::uint32_t*) { return 0; }
static inline int GetFileTime(void*, FILETIME*, FILETIME*, FILETIME* out_last_write) {
  if (out_last_write) {
    out_last_write->dwLowDateTime = 0;
    out_last_write->dwHighDateTime = 0;
  }
  return 1;
}
static inline int FileTimeToSystemTime(const FILETIME*, SYSTEMTIME* out_system_time) {
  if (!out_system_time) return 0;
  memset(out_system_time, 0, sizeof(*out_system_time));
  out_system_time->wYear = 1970;
  out_system_time->wMonth = 1;
  out_system_time->wDay = 1;
  return 1;
}
static inline int GetVersionEx(OSVERSIONINFO* info) {
  if (!info) return 0;
  info->dwMajorVersion = 10;
  info->dwMinorVersion = 0;
  info->dwBuildNumber = 0;
  info->dwPlatformId = VER_PLATFORM_WIN32_NT;
  info->szCSDVersion[0] = '\0';
  return 1;
}

#define LOWORD(l) ((std::uint16_t)(((std::uintptr_t)(l)) & 0xffff))
#define HIWORD(l) ((std::uint16_t)((((std::uintptr_t)(l)) >> 16) & 0xffff))

#ifndef LPCTSTR
#define LPCTSTR const char*
#endif

#ifndef MAKEINTRESOURCE
#define MAKEINTRESOURCE(i) ((char*)(std::uintptr_t)(std::uint16_t)((i) & 0xFFFF))
#endif

#ifndef stricmp
#define stricmp strcasecmp
#endif
#ifndef strnicmp
#define strnicmp strncasecmp
#endif

#endif

#endif
