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

For macOS link-closure work, use strict mode:

```bash
LASTCHAOS_VALIDATE_NKSP_STRICT_LINK=1 ./scripts/validate_porting_matrix.sh
```
