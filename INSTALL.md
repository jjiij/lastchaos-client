# Client Build (Windows)

This repository is currently build-tested for Windows via Visual Studio solution builds.

## Platform layout

- `platform/windows/` contains Windows-specific helper scripts.
- `platform/linux/` contains Linux-specific helper scripts.
- `platform/macos/` contains macOS-specific helper scripts and app-template files.

## Local build

1. Open `source/LCClient.sln` in Visual Studio 2022.
2. Select:
   - `Configuration`: `Template`
   - `Platform`: `Win32` or `x64`
3. Build solution.

Expected runtime output is collected under `source/Bin/`.

## CI build + distribution

GitHub Actions workflow:
- `.github/workflows/cross-platform-build.yml`

What it does:
1. Builds `source/LCClient.sln` for `Template|Win32` and `Template|x64`.
2. Packages `source/Bin/` into zip artifacts.
3. Uploads each zip to GoFile.
4. Appends GoFile URLs into the GitHub release notes when triggered from a published release.

## Required GitHub secrets

- `GOFILE_API_TOKEN`
- `GOFILE_FOLDER_ID`

Without these secrets, GoFile upload step fails.
