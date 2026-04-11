#include <Engine/Base/PlatformCompat.h>
#include <Engine/Base/Types.h>
#include <Engine/Base/Synchronization.h>
#include <Engine/Base/Timer.h>
#include <Engine/Base/TLVar.h>
#include <Engine/Base/Stream.h>
#ifndef HRESULT
typedef int HRESULT;
#endif
#include <Engine/Network/Web.h>
#include <mutex>

namespace {
struct StubCriticalSection {
  std::recursive_mutex mutex;
  long recursion = 0;
};
}

const TIME CTimer::TickQuantum = TIME(1 / 20.0);
std::vector<TLVarBase*> TLVarBase::m_Vars;
BOOL _bIMEProc = FALSE;
CTFileName _fnmApplicationPath;
CTFileName _fnmMod;
CTString _strModName;
CTString _strModURL;
CTString _strModExt;
CTFileName _fnmCDPath;
CTFileName _fnmApplicationExe;
void* _pAnimStock = nullptr;
void* _pTextureStock = nullptr;
void* _pFontStock = nullptr;
void* _pSoundStock = nullptr;
void* _pModelStock = nullptr;
void* _pEntityClassStock = nullptr;
void* _pMeshStock = nullptr;
void* _pSkeletonStock = nullptr;
void* _pAnimSetStock = nullptr;
void* _pShaderStock = nullptr;
void* _pModelInstanceStock = nullptr;

void FreeUnusedStock(void) {}
void UseApplicationPath(void) {}
void IgnoreApplicationPath(void) {}
__int64 GetDiskFreeSpace(const CTString&) { return 0; }
void NotifyReadingError(const CTFileName&) {}
void NotifyWritingError(const CTFileName&) {}
void SetErrorReadingCallback(void(*)(const CTFileName&)) {}
void SetErrorWritingCallback(void(*)(const CTFileName&)) {}

CTCriticalSection::CTCriticalSection(void) {
  cs_iIndex = -1;
  cs_pvObject = new StubCriticalSection();
}

CTCriticalSection::~CTCriticalSection(void) {
  delete static_cast<StubCriticalSection*>(cs_pvObject);
  cs_pvObject = nullptr;
}

INDEX CTCriticalSection::Lock(void) {
  auto* obj = static_cast<StubCriticalSection*>(cs_pvObject);
  obj->mutex.lock();
  return static_cast<INDEX>(++obj->recursion);
}

INDEX CTCriticalSection::TryToLock(void) {
  auto* obj = static_cast<StubCriticalSection*>(cs_pvObject);
  if (!obj->mutex.try_lock()) {
    return 0;
  }
  return static_cast<INDEX>(++obj->recursion);
}

INDEX CTCriticalSection::Unlock(void) {
  auto* obj = static_cast<StubCriticalSection*>(cs_pvObject);
  if (obj->recursion > 0) {
    --obj->recursion;
  }
  obj->mutex.unlock();
  return static_cast<INDEX>(obj->recursion);
}

CTSingleLock::CTSingleLock(CTCriticalSection* pcs, BOOL bLock) : sl_cs(*pcs) {
  sl_bLocked = FALSE;
  sl_iLastLockedIndex = -2;
  if (bLock) {
    Lock();
  }
}

CTSingleLock::~CTSingleLock(void) {
  if (sl_bLocked) {
    Unlock();
  }
}

void CTSingleLock::Lock(void) {
  if (!sl_bLocked) {
    sl_cs.Lock();
    sl_bLocked = TRUE;
  }
}

BOOL CTSingleLock::TryToLock(void) {
  if (sl_bLocked) {
    return TRUE;
  }
  if (sl_cs.TryToLock() > 0) {
    sl_bLocked = TRUE;
    return TRUE;
  }
  return FALSE;
}

BOOL CTSingleLock::IsLocked(void) {
  return sl_bLocked;
}

void CTSingleLock::Unlock(void) {
  if (sl_bLocked) {
    sl_cs.Unlock();
    sl_bLocked = FALSE;
  }
}

cWeb::cWeb()
  : m_eStatus(WS_PREBEGIN),
    m_pThread(nullptr),
    m_hWebWnd(nullptr),
    m_WebDlgID(0),
    m_fnWebCallBack(nullptr),
    m_nPosX(0),
    m_nPosY(0),
    m_nWidth(0),
    m_nHeight(0),
    m_pixMinI(0),
    m_pixMinJ(0),
    m_pixMaxI(0),
    m_pixMaxJ(0) {}

cWeb::~cWeb() {}

BOOL cWeb::Begin() { m_eStatus = WS_READYREQUEST; return TRUE; }
BOOL cWeb::End() { m_eStatus = WS_PREBEGIN; return TRUE; }
void cWeb::Request(const char*) { m_eStatus = WS_READYREAD; }
BOOL cWeb::Read(std::string&, std::string&) { return FALSE; }
BOOL cWeb::OpenWebPage(HWND) { return FALSE; }
void cWeb::SetWebMoveWindow(void) {}
BOOL cWeb::SendWebPageOpenMsg(BOOL) { return FALSE; }
BOOL cWeb::CloseWebPage(HWND) { return TRUE; }
void cWeb::SetWebPosition(INDEX, INDEX) {}
void cWeb::SetPos(int x, int y) { m_nPosX = x; m_nPosY = y; }
void cWeb::SetSize(int width, int height) { m_nWidth = width; m_nHeight = height; }
void cWeb::UpdatePos() {}
void cWeb::SetWebUrl(std::string&) {}
