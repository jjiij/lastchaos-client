# macOS App Packaging

## Scripts

| Script | Purpose |
|--------|---------|
| `client-source/scripts/build_macos_app.sh` | Minimal `.app` with `lastchaos_login_check` (or override binary) + `sl.dta` in `MacOS/` |
| `client-source/scripts/package_full_macos_app.sh` | Full game bundle: native client + **entire** `Data/` tree under `Contents/Resources/Game/` |
| `client-source/scripts/build_full_macos_app.sh` | Same as above with **checks**, optional `.lastchaos_full_bundle.env`, optional `LASTCHAOS_CREATE_DMG=1` |
| `client-source/scripts/create_macos_dmg.sh` | Pack `LastChaos.app` into a **single-file** `LastChaos.dmg` for distribution |

Output location (default):

- `client-source/build/macos/LastChaos.app`

## Code signing (local)

After the bundle is assembled, `sign_macos_app_bundle` runs **`codesign`** on macOS so Finder and Gatekeeper are less likely to block launch.

| Variable | Default | Meaning |
|----------|---------|---------|
| `CODESIGN_IDENTITY` | `-` | **Ad-hoc** signing (no Apple ID; good for local dev). |
| `LASTCHAOS_SIGN` | `1` | Set to `0` to skip signing. |
| `LASTCHAOS_CLEAR_QUARANTINE` | `1` | Removes `com.apple.quarantine` from the `.app` when present (e.g. after copying from a download). Set to `0` to leave xattrs unchanged. |

Use a **Development** or **Developer ID** identity when you need stricter testing:

```bash
CODESIGN_IDENTITY="Apple Development: Your Name (XXXXXXXXXX)" ./scripts/build_macos_app.sh
```

List identities: `security find-identity -v -p codesigning`

## Full game world inside the bundle

`package_full_macos_app.sh` copies your asset root (must contain `Data/` or `data/`) into:

- `LastChaos.app/Contents/Resources/Game/`

The engine resolves this root when the executable lives in `Contents/MacOS/` (see `AnalyzeApplicationPath()` in `Engine/Engine.cpp`): the assumed game directory is `…/Contents/Resources/Game/`.

Login servers are read from `sl.dta` next to that game data (engine uses `_fnmApplicationPath.FileDir() + "sl.dta"`). The packaging script copies `sl.dta` into **both** `MacOS/` (for small tools using the executable directory) and `Resources/Game/` (for the real client).

### Environment variables (`package_full_macos_app.sh`)

- `LASTCHAOS_CLIENT_BINARY` — path to the native macOS game executable  
- `LASTCHAOS_ASSET_ROOT` — directory whose contents include `Data/` (maps, textures, etc.)
- `LASTCHAOS_MACOS_ARCHS` — CMake `CMAKE_OSX_ARCHITECTURES` value (default: `x86_64;arm64`)

### Universal binary checks (`build_full_macos_app.sh`)

- `build_full_macos_app.sh` validates `LASTCHAOS_CLIENT_BINARY` architectures with `lipo` when available.
- Default required architecture list is `x86_64;arm64`.
- Override with:
  - `LASTCHAOS_EXPECT_CLIENT_ARCHS` (explicit required list), or
  - `LASTCHAOS_MACOS_ARCHS` (used as fallback expected list).

### Single downloadable file

macOS does not ship one monolithic “exe”; the standard pattern is a **`.dmg`** containing `LastChaos.app`. After `package_full_macos_app.sh`:

```bash
./scripts/create_macos_dmg.sh
```

produces `build/macos/LastChaos.dmg` by default.

## Double-click opens Terminal

The app’s main executable is a small script that runs `LastChaosInner.command` in **Terminal.app**, so you see login-check output and prompts. A raw CLI binary as `CFBundleExecutable` exits with no visible window when launched from Finder.

## Minimal / launcher-only bundle

`build_macos_app.sh` supports:

1. Default: `lastchaos_login_check` is installed as `LastChaosLauncher.bin`; wrappers are `LastChaosLauncher` + `LastChaosInner.command`.  
2. Optional **full `Data/` tree**: copy or symlink client files into `client-source/game-assets/` (with a `Data/` folder), **or** set `LASTCHAOS_ASSET_ROOT` when running the script.  
3. Override: `LASTCHAOS_CLIENT_BINARY=/path/to/binary` to replace the payload binary.
