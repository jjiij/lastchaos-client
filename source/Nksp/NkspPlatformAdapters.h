#ifndef SE_INCL_NKSP_PLATFORM_ADAPTERS_H
#define SE_INCL_NKSP_PLATFORM_ADAPTERS_H

// Non-Windows helper used during macOS/Linux bring-up to emulate
// single-instance behavior while Win32 ToolHelp paths are still primary.
// Returns:
//  - 1: another instance appears to be running
//  - 0: no conflicting instance detected
//  - -1: adapter could not run (see out_errno when provided)
int NkspCheckSingleInstanceUnix(const char* lock_path, int* out_errno);

#endif
