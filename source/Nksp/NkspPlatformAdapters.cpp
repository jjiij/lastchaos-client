#include "NkspPlatformAdapters.h"

#ifndef PLATFORM_WIN32
#include <errno.h>
#include <fcntl.h>
#include <sys/file.h>
#include <unistd.h>
#else
#include <process.h>
#include <SharedMemory/Ext_ipc_event.h>
#endif

int NkspCheckSingleInstanceUnix(const char* lock_path, int* out_errno)
{
#ifdef PLATFORM_WIN32
  (void)lock_path;
  if (out_errno) {
    *out_errno = 0;
  }
  return 0;
#else
  static int s_unix_single_instance_fd = -1;
  if (s_unix_single_instance_fd == -1) {
    s_unix_single_instance_fd = open(lock_path, O_CREAT | O_RDWR, 0644);
    if (s_unix_single_instance_fd < 0) {
      if (out_errno) {
        *out_errno = errno;
      }
      return -1;
    }
  }

  if (flock(s_unix_single_instance_fd, LOCK_EX | LOCK_NB) != 0) {
    if (out_errno) {
      *out_errno = errno;
    }
    if (errno == EWOULDBLOCK) {
      return 1;
    }
    return -1;
  }

  if (out_errno) {
    *out_errno = 0;
  }
  return 0;
#endif
}

int NkspExecReplaceProcess(const char* command, const char* const* argv)
{
#ifdef PLATFORM_WIN32
  return _execv(command, argv);
#else
  return execv(command, const_cast<char* const*>(argv));
#endif
}

void NkspReleaseIPCBridge(void)
{
#ifdef PLATFORM_WIN32
  XExtIPCManager<IPCEventInfo> IPCMgr;
  for (int retry = 3; retry > 0; --retry)
  {
    int ret = IPCMgr.XExtIPCEventCreate(1, FALSE);
    if (ret == 0)
    {
      IPCEventInfo eventInfo;
      eventInfo.EventID = -1;
      IPCMgr.XExtIPCEventPost(&eventInfo);
      IPCMgr.XExtIPCEventRelease(FALSE);
      break;
    }
    Sleep(500);
  }
#endif
}
