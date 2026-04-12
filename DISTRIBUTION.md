# Cross-Platform Distribution Packaging (with external assets repo)

The game assets are stored outside this repository in:

- `https://github.com/jjiij/lastchaos-client-assets`

Use `scripts/build_distribution_bundle.sh` to stage platform distributions that always include the full asset payload and then apply platform-specific runtime adjustments.

## Why platform adjustments are required

The assets repository includes Windows-only launch/runtime files (for example top-level `*.exe`, `launcher.bat`, and `Bin/*.dll`/`*.exe`).

For Linux and macOS distributions, the packaging script removes those Windows-only payloads and injects native replacements (launcher/app/runtime) when provided.

## Local usage

```bash
# Linux staging (inject native launcher binary from local build output)
./scripts/build_distribution_bundle.sh \
  --platform linux \
  --assets-root /path/to/lastchaos-client-assets \
  --runtime-root "$PWD/build/linux-x64/porting" \
  --archive

# Windows staging
./scripts/build_distribution_bundle.sh \
  --platform windows \
  --assets-root /path/to/lastchaos-client-assets \
  --archive

# macOS staging (inject LastChaos.app)
./scripts/build_distribution_bundle.sh \
  --platform macos \
  --assets-root /path/to/lastchaos-client-assets \
  --runtime-root "$PWD/build/macos" \
  --archive
```

Archives are written under `build/dist/archives/`.

## CI wiring

`.github/workflows/distribution-matrix.yml` checks out this repository plus `jjiij/lastchaos-client-assets`, then builds and stages distribution artifacts for:

- Linux x64
- Windows x64
- macOS (universal launcher/app path)

Each job uploads an archive artifact (`lastchaos-<platform>-distribution`).


## GitHub Releases publishing

`distribution-matrix.yml` now also publishes a rolling prerelease tagged `nightly` on pushes to `main`/`master`, attaching Linux, Windows, and macOS distribution archives so artifacts appear under GitHub Releases in addition to Actions artifacts.
