#include "stdH.h"

#include "ThreadWrapper.h"

#include <assert.h>

#if defined(_WIN32) || defined(_WIN64)
#include <process.h>
#endif

cThreadWrapper::cThreadWrapper(UINT (WINAPI *pRunningFunction)(LPVOID))
: m_pRunningFunction(pRunningFunction)
#if defined(_WIN32) || defined(_WIN64)
, m_hThreadHandle(0)
#else
, m_hThreadHandle(nullptr)
, m_threadRunning(false)
, m_exitCode(0xFFFFFFFFu)
#endif
, m_uiThreadId(0)
, m_dwThreadSuspendCount(0)
{
  assert(pRunningFunction != nullptr);
}

cThreadWrapper::~cThreadWrapper()
{
#if defined(_WIN32) || defined(_WIN64)
  if (m_hThreadHandle != NULL) {
    CloseHandle(m_hThreadHandle);
  }
#else
  if (m_thread.joinable()) {
    m_thread.detach();
  }
#endif
}

DWORD cThreadWrapper::GetExitCode()
{
#if defined(_WIN32) || defined(_WIN64)
  if (m_hThreadHandle != NULL)
  {
    DWORD exitCode = 0;
    if (GetExitCodeThread(m_hThreadHandle, &exitCode))
    {
      return exitCode;
    }
  }
  return 0xFFFFFFFFu;
#else
  return m_exitCode.load();
#endif
}

BOOL cThreadWrapper::Start(void *parameter)
{
#if defined(_WIN32) || defined(_WIN64)
  if (m_hThreadHandle != NULL) {
    CloseHandle(m_hThreadHandle);
  }
  m_hThreadHandle = (HANDLE)_beginthreadex(NULL, 0, m_pRunningFunction, parameter, 0, &m_uiThreadId);
  if (m_hThreadHandle == NULL)
  {
    return FALSE;
  }
  return TRUE;
#else
  if (m_thread.joinable()) {
    m_thread.detach();
  }

  m_threadRunning.store(true, std::memory_order_release);
  m_exitCode.store(0xFFFFFFFFu, std::memory_order_release);
  m_thread = std::thread([this, parameter]() {
    m_uiThreadId = GetCurrentThreadId();
    const UINT exitCode = m_pRunningFunction(parameter);
    m_exitCode.store(exitCode, std::memory_order_release);
    m_threadRunning.store(false, std::memory_order_release);
  });
  m_hThreadHandle = nullptr;
  return TRUE;
#endif
}

BOOL cThreadWrapper::Terminate(DWORD exitCode)
{
#if defined(_WIN32) || defined(_WIN64)
  BOOL bRet = TerminateThread(m_hThreadHandle, exitCode);
  WaitForSingleObject(m_hThreadHandle, INFINITE);
  CloseHandle(m_hThreadHandle);
  m_hThreadHandle = 0;
  return bRet;
#else
  m_exitCode.store(exitCode, std::memory_order_release);
  return FALSE;
#endif
}

void cThreadWrapper::Resume()
{
#if defined(_WIN32) || defined(_WIN64)
  m_dwThreadSuspendCount = ResumeThread(m_hThreadHandle);
#endif
}

void cThreadWrapper::Suspend()
{
#if defined(_WIN32) || defined(_WIN64)
  m_dwThreadSuspendCount = SuspendThread(m_hThreadHandle);
#endif
}
