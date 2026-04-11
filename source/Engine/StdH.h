
#if		defined(_MSC_VER) && (_MSC_VER >= 1600)
#	define WINDOW_SDK_70A
#endif

#define ENGINE_INTERNAL 1
#define ENGINE_EXPORTS 1

#pragma warning(disable : 4786)
#pragma warning(disable : 4391)		// Platform SDK Warning
#pragma warning(disable : 4996)		// CRT _s
#pragma warning(disable : 4200)		// packet zero-base array
#pragma warning(disable : 4819)		// packet zero-base array
#pragma warning(disable : 4603)		// packet zero-base array

#include <stdlib.h>
#include <malloc.h>
#include <stdarg.h>
#include <stdio.h>
#if defined(_WIN32) || defined(_WIN64)
#include <conio.h>
#include <crtdbg.h>
#endif
#include <string.h>
#include <stddef.h>
#include <time.h>
#include <math.h>
#include <search.h>   // for qsort
#include <float.h>    // for FPU control

extern "C"
{
    #include <Engine/lua/lua.h>
    #include <Engine/lua/lualib.h>
    #include <Engine/lua/lauxlib.h>
};

#include <Engine/Xbox/XKeyboard.h>

#include <Engine/Base/Types.h>
#include <Engine/Base/Assert.h>

#if defined(_WIN32) || defined(_WIN64)
  #include <winsock2.h>
  #include <windows.h>
  #include <mmsystem.h> // for timers
#else
  #include <Engine/Base/PlatformCompat.h>
#endif

#if		!defined(WORLD_EDITOR)
#include <boost/thread.hpp>
#include <boost/format.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <boost/shared_ptr.hpp>
#endif	// WORLD_EDITOR

#ifdef	_DEBUG
#	pragma comment(lib, "zlib.lib")
#	pragma comment(lib, "libpng.lib")
#else	// _DEBUG
#	pragma comment(lib, "zlib.lib")
#	pragma comment(lib, "libpng.lib")
#endif	// _DEBUG

