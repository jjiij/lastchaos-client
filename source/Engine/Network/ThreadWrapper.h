#ifndef __THREADWRAPPER_H__
#define __THREADWRAPPER_H__

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#else
#include <Engine/Base/PlatformCompat.h>
#include <atomic>
#include <thread>
#endif

class ENGINE_API cThreadWrapper
{
public:
  cThreadWrapper(UINT (WINAPI *pRunningFunction)(LPVOID));
  ~cThreadWrapper();

  HANDLE GetHandle() { return m_hThreadHandle; }
  UINT GetId() { return m_uiThreadId; }
  DWORD GetSuspendCount() { return m_dwThreadSuspendCount; }
  DWORD GetExitCode();

  BOOL Start(void *parameter);
  BOOL Terminate(DWORD exitCode);
  void Resume();
  void Suspend();

private:
  UINT (WINAPI *m_pRunningFunction)(LPVOID);
#if defined(_WIN32) || defined(_WIN64)
  HANDLE m_hThreadHandle;
#else
  HANDLE m_hThreadHandle;
  std::thread m_thread;
  std::atomic<bool> m_threadRunning;
  std::atomic<DWORD> m_exitCode;
#endif
  UINT m_uiThreadId;
  DWORD m_dwThreadSuspendCount;
};

#endif
