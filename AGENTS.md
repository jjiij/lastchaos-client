# AGENTS.md

## Project Porting Priorities (living note)

When working in this repository, prioritize platform work in this order:

1. **Primary focus: macOS universal support (x64 + arm64)**
   - Drive toward a fully working native macOS client build.
   - Keep packaging/build scripts aligned with universal binaries where possible.
   - Resolve compile, link, runtime, and packaging blockers with this as the top priority.

2. **Maintain compatibility while porting**
   - Do not regress existing **Windows x64** support.
   - Do not regress existing **Linux x64** support.
   - Prefer cross-platform abstractions and conditional compilation patterns that preserve all three major desktop platforms.

3. **Secondary wins (opportunistic)**
   - Native **Windows arm64** and **Linux arm64** support are welcome improvements.
   - Treat these as stretch goals unless they conflict with macOS readiness work.

## Implementation guidance

- Prefer portable layers/adapters over platform-specific code spread.
- Keep CMake and script changes structured so multi-platform CI/matrix work can be added incrementally.
- For macOS-specific fixes, isolate them cleanly (feature options, platform guards, target properties) so Windows/Linux paths stay stable.
- If a trade-off is required, choose the option that keeps macOS universal build progress unblocked while minimizing risk to Windows x64 and Linux x64.

## Definition of “macOS ready” (target)

A change set should trend toward this state:

- Native macOS client builds for **both x64 and arm64** (preferably universal artifacts).
- Required runtime dependencies are bundled or documented.
- Packaging outputs are launchable and testable with expected assets/configuration.
- No known regressions introduced for Windows x64 or Linux x64 baseline builds.

## Current execution focus

- Keep macOS build and packaging scripts aligned with universal-arch workflows (x64 + arm64).
- Add fast validation checks that fail early when required macOS architectures are missing from provided binaries.
- Prefer improvements that are additive and non-breaking for existing Windows/Linux build paths.
- Track implementation phases and milestone status in `MACOS_FULL_PORT_PLAN.md`.
