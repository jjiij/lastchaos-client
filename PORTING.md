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

`validate_porting_matrix.sh` now enables strict `nksp_probe` linking by default on macOS.
If local bring-up needs temporary permissive mode, explicitly opt out:

```bash
LASTCHAOS_VALIDATE_NKSP_STRICT_LINK=0 ./scripts/validate_porting_matrix.sh
```

On macOS, validation also emits unresolved-symbol artifacts for `nksp_probe` under the chosen build directory:

- `nksp_unresolved_symbols.txt`
- `nksp_unresolved_symbols_summary.txt`
- `nksp_unresolved_symbols_by_subsystem.txt`
- `nksp_unresolved_symbols_subsystem_summary.txt`


## macOS signed bundle host validation

Use the host-side validator after packaging to capture signing/runtime evidence per architecture host:

```bash
./scripts/validate_macos_bundle.sh /path/to/LastChaos.app
```

On Intel and Apple Silicon hosts this writes `build/macos/bundle_validation_<arch>.txt` and checks bundle archs, signing, Gatekeeper assessment, and bundled login/data inputs. Optional login smoke can be attached with `LASTCHAOS_RUN_LOGIN_SMOKE=1` and `LASTCHAOS_LOGIN_SMOKE_CMD`.


## Distribution bundles with external assets

Cross-platform distribution staging (Windows/Linux/macOS) is documented in `DISTRIBUTION.md` and implemented by `scripts/build_distribution_bundle.sh`, which consumes the external assets repository `jjiij/lastchaos-client-assets` and applies platform-specific runtime replacement rules.
