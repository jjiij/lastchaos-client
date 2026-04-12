# Distribution

Windows client artifacts are distributed through the GitHub workflow:
- `.github/workflows/cross-platform-build.yml`

Release flow:
1. Publish a GitHub release.
2. Workflow builds Windows `Win32` and `x64` client packages.
3. Packages are uploaded to GoFile.
4. GoFile links are appended to the release body under `## GoFile Downloads`.

Package format:
- `lastchaos-client-windows-win32.zip`
- `lastchaos-client-windows-x64.zip`

Each archive is built from `source/Bin/`.
