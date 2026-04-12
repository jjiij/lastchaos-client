# Porting Baseline

This repository now has a first cross-platform scaffold in place:

- `client-source/CMakeLists.txt`
- `client-source/porting/`
- `Engine/Base/PlatformCompat.h`

Current intent:

- preserve the existing Windows Visual Studio workflow
- add a separate cross-platform probe target
- start moving shared code behind portable wrappers
- provide a small login connectivity check for local server wiring

The probe is intentionally small. It is the first check that the toolchain, C++ standard level, filesystem handling, and platform detection behave consistently on Windows, Linux, and macOS.

## Quick validation helper

Use:

```bash
./scripts/validate_porting_matrix.sh
```

This runs native host probe targets and prints Windows x64 baseline commands for CI/manual validation.

### Explicit baseline commands

Linux x64:

```bash
cmake -S . -B build/linux-x64 -G "Unix Makefiles"
cmake --build build/linux-x64 --target lastchaos_porting_probe lastchaos_login_check -j "$(nproc)"
```

Windows x64:

```powershell
cmake -S . -B build\win64 -G "Visual Studio 17 2022" -A x64
cmake --build build\win64 --config Release --target GameMP
```

For macOS link-closure work, use strict mode:

```bash
LASTCHAOS_VALIDATE_NKSP_STRICT_LINK=1 ./scripts/validate_porting_matrix.sh
```

On macOS, validation also emits unresolved-symbol artifacts for `nksp_probe` under the chosen build directory:

- `nksp_unresolved_symbols.txt`
- `nksp_unresolved_symbols_summary.txt`
- `nksp_unresolved_symbols_by_subsystem.txt`
- `nksp_unresolved_symbols_subsystem_summary.txt`
