# Native macOS Build Blockers (Current)

Date: 2026-04-11

## Current state

- macOS launcher app bundle builds successfully (again):
  - `/Users/tin/_data/files/Projects/0_Concepts/lastchaos/client-source/build/macos/LastChaos.app`
- MoltenVK runtime is now bundled into the `.app` (`Contents/Frameworks`) so users do not need a separate install.
- `porting` CMake graph now configures and builds reliably for baseline targets.
- `nksp_probe` target is now wired into top-level CMake and builds successfully on macOS.
- `Nksp/Nksp.cpp` now compiles to object (`-c`) with portability flags and local native/directx headers.
- Include-path case normalization was applied across live `.h/.cpp` client sources, reducing case-sensitive filesystem breakage on macOS/Linux.

## Newly resolved in this pass

- CMake stabilization for macOS build graph:
  - `porting/CMakeLists.txt` now uses correct source-root paths.
  - `Engine/VulkanAbstraction/VulkanContext.cpp` is optional behind:
    - `-DLASTCHAOS_ENABLE_VULKAN_CONTEXT=ON`
  - Baseline mac targets no longer fail when experimental Vulkan abstraction is incomplete.
- Vulkan runtime bundling:
  - `scripts/lib_macos_app_bundle.sh` can copy/probe MoltenVK/Vulkan dylib and install into:
    - `LastChaos.app/Contents/Frameworks`
  - Integrated into:
    - `scripts/build_macos_app.sh`
    - `scripts/package_full_macos_app.sh`
  - Added loader-side support for bundled Vulkan dylib lookup in:
    - `vulkan/vulkan_loader.cpp`
- Vulkan abstraction compile progression (experimental path):
  - `Engine/VulkanAbstraction/VulkanContext.h`
    - optional MoltenVK include via `__has_include`
    - added `VK_USE_PLATFORM_METAL_EXT` on macOS
  - `Engine/VulkanAbstraction/VulkanContext.cpp`
    - fixed platform type declaration guards
    - removed invalid platform-specific surface destroy usage
    - fixed missing `<cstring>` and variable redefinition
  - Result: `lastchaos_porting` builds with `-DLASTCHAOS_ENABLE_VULKAN_CONTEXT=ON`.
- `Nksp` portability progression:
  - fixed non-Windows Win-compat geometry include path in `Engine/Base/Input.h`
  - added `POINT` Win-compat type in `Engine/Base/PlatformCompat.h`
  - isolated Windows-only IPC event block in `Nksp/Nksp.cpp` (`IPCEventInfo` path now guarded for `PLATFORM_WIN32`)
  - fixed command-line path parse bug in `Nksp/Nksp.cpp` (`if (pstr[0] == '\\')`)
  - added CMake `Nksp_probe` wiring under:
    - `CMakeLists.txt` (`LASTCHAOS_ENABLE_NKSP_PROBE` option)
    - `Nksp_probe/CMakeLists.txt` (portable include/define parity with manual probe)
  - fixed include-case mismatch in `Nksp/Nksp.cpp`:
    - `"GlSettings.h"` -> `"GLSettings.h"`

- Login endpoint configurability (new):
  - `porting/CMakeLists.txt` now exposes cache vars for generated `sl.dta`:
    - `LASTCHAOS_LOGIN_NAME`
    - `LASTCHAOS_LOGIN_HOST`
    - `LASTCHAOS_LOGIN_PORT`
    - `LASTCHAOS_LOGIN_VERSION`
    - `LASTCHAOS_LOGIN_USERS`
  - `scripts/build_macos_app.sh` and `scripts/package_full_macos_app.sh` now pass these values through to CMake.
  - `.lastchaos_full_bundle.env.example` documents these optional endpoint overrides.

- C++ template portability fixes:
  - `Engine/Help/LoadLod.h` (`typename` on dependent iterators)
  - `Engine/Contents/function/TitleData.h` (`typename` on dependent iterators)
  - `Engine/World/World.h` (removed extra class qualification)
- Effect/tag API compatibility:
  - `Engine/Effect/CWorldTag.h` now accepts const refs for temporary args.
- Windows include/path cleanup:
  - `Engine/Interface/UIBuff.h` include typo fixed.
  - `Engine/Network/EntityHashing.h` / `Engine/World/WorldEntityHashing.h` slash portability fixes.
- Non-Windows compatibility shims:
  - `Engine/Base/PlatformCompat.h` added `MK_LBUTTON`, `_controlfp`, and FPU mask constants.
  - `Engine/Rendering/Render_internal.h` now avoids hard `d3dx9.h` include on non-Windows and provides minimal D3DX type aliases.
  - `Engine/Sound/SoundData.h` / `Engine/Sound/SoundLibrary.h` now build without `DSound.h` on non-Windows headers.
- Temporary generated-header stubs added:
  - `EntitiesMP/Global.h`
  - `EntitiesMP/Player.h`
- Temporary Win-compat header stubs added for parser progression:
  - `io.h`, `process.h`, `shlwapi.h`, `crtdbg.h`, `TlHelp32.h`

## Current hard blockers

1. Native game executable (`Nksp`) is not fully link-resolved and runnable on macOS yet.
- `nksp_probe` now compiles/links with `-undefined dynamic_lookup`, but runtime fails with unresolved engine symbols (currently first observed: `__bClientApp`).
- Next phase is wiring actual engine/object libraries instead of relying on dynamic unresolved lookup.

2. `SharedMemory/Ext_ipc_event.h` and other IPC/process-launch code paths are still Windows-specific and require non-Windows stubs or compile-time exclusion.
   - Update (2026-04-11): `Ext_ipc_event` and `SharedMemory_queue` now have non-Windows fallback implementations to unblock compilation and basic in-process IPC event flow; cross-process parity still pending.

3. Temporary stubs in `EntitiesMP/` and Win headers are only for compile progression; real generated/script outputs are still required for a fully functional game client.

4. Runtime validation against Docker login server is currently blocked in this environment:
- `docker.sock` was unavailable (`Cannot connect to the Docker daemon`), so `lastchaos_login_check` could not complete endpoint handshake verification.
5. The app bundle still cannot be "fully playable" from this repository alone because:
- no real `Data/` runtime assets are present under `client-source/game-assets/`
- no fully link-resolved native macOS game executable is produced yet (current bundle payload defaults to `lastchaos_login_check` unless an external client Mach-O is supplied).

## macOS app bundle layout (implemented)

- `Engine/Engine.cpp` — `AnalyzeApplicationPath()` detects `…/Something.app/Contents/MacOS/…` and sets the engine root to `…/Contents/Resources/Game/` (falls back to the executable’s directory for loose binaries).
- `Engine/Base/FileName.cpp` — `FileDir()` / `FileName()` treat `/` as a path separator on Unix so `_fnmApplicationPath` can use POSIX-style paths.
- `Engine/Base/Stream.cpp` — `fopen` paths normalize `\` → `/` on Unix when opening loose files.
- `scripts/package_full_macos_app.sh` — rsync asset tree into `Resources/Game/` and copies `sl.dta` there (and under `MacOS/` for tooling).
- `scripts/lib_macos_app_bundle.sh` — bundles MoltenVK/Vulkan dylib into `Contents/Frameworks` and re-signs app bundle.

## Step Status (Port Plan 1-7)

1. CMake/build-system layer while preserving Windows flow:
- In progress, major blocker resolved for mac baseline pipeline.

2. Platform abstraction in shared runtime:
- In progress, major `Nksp` compile blockers reduced; remaining work is link/runtime integration.

3. Linux/mac non-Windows bring-up path:
- In progress, compile frontier improved; runtime still not game-complete.

4. Rendering/window/input for native startup/login/scene:
- In progress, `Nksp` compile target exists and builds, but not yet backed by full engine linkage/runtime bootstrap.

5. Re-enable/verify Windows parity on portable path:
- Not started in this pass.

6. Linux x86 + mac packaging hardening:
- In progress for mac packaging (app bundle + bundled runtime), not complete for playable native client.

7. CI/build matrix + packaging scripts:
- Partially started (scripts exist), no full matrix yet.

## Next critical implementation steps

1. Continue isolating Windows-only launcher/runtime blocks in `Nksp/Nksp.cpp` behind `#ifdef PLATFORM_WIN32` and add Unix entry flow.
2. Replace temporary Win process/ToolHelp/file-lock/Dialog code with platform adapter APIs and mac implementations.
3. Replace temporary `EntitiesMP` stubs with generated real headers from `.es` pipeline.
4. Start explicit `nksp_probe` link integration:
   - add concrete object/library linkage for engine symbols (first unresolved: `__bClientApp`)
   - remove reliance on `-undefined dynamic_lookup` incrementally.
5. Move from compile-only to link+runtime for native game binary; validate launch, login, and scene render.
5. Re-run login/server tests once Docker daemon access is restored in this environment.
