# LastChaos Full Port Plan (macOS-first, cross-platform-safe)

Date: 2026-04-11

## Recent Progress (2026-04-11)

- Added full-package preflight `scripts/check_generated_headers.sh` and wired it into `scripts/build_full_macos_app.sh`.
- Full macOS packaging now fails early when placeholder generated headers are detected, with a documented escape hatch (`LASTCHAOS_ALLOW_PLACEHOLDER_HEADERS=1`) for compile-only bring-up.
- Added host-side validation helper `scripts/validate_porting_matrix.sh` to run Linux/macOS probe checks and print Windows baseline commands.
- Added `scripts/report_nksp_unresolved_symbols.sh` and hooked it into matrix validation to track unresolved-symbol closure for `nksp_probe`.
- Lowered `porting/CMakeLists.txt` minimum CMake version to `3.21` to match the repo root and unblock Linux/macOS probe validation on standard toolchains.
- Added strict `nksp_probe` link mode toggle (`LASTCHAOS_NKSP_STRICT_LINK`) so unresolved-symbol closure can be enforced in validation runs.
- Added deterministic `EntitiesMP` header generation script (`scripts/generate_entitiesmp_headers.sh`) and integrated it into full macOS build preflight.

## Goal

Ship a **fully runnable native macOS client** (x86_64 + arm64, ideally universal) from this repository while preserving Windows x64 and Linux x64 compatibility.

## Success Criteria (Definition of Done)

1. **Build**
   - Native macOS game executable links without unresolved symbols.
   - Universal build artifacts are produced (`x86_64` + `arm64`) or per-arch artifacts with reproducible lipo merge.
2. **Runtime**
   - Client launches from `.app` and reaches login flow using bundled config.
   - Login handshake succeeds against target server profile.
   - Basic scene render/input path works (no crash in first playable zone).
3. **Packaging**
   - `LastChaos.app` includes required runtime deps (Vulkan/MoltenVK where needed) and signs cleanly.
   - Optional `LastChaos.dmg` generation is reproducible.
4. **Cross-platform safety**
   - Windows x64 baseline build remains green.
   - Linux x64 portability/probe build remains green.

---

## Current State Snapshot

- Packaging and launcher script path exists.
- Porting/probe CMake targets are in place.
- Native `Nksp` runtime/link path is still incomplete.
- IPC/process-launch and generated header/assets paths still require completion for full playability.

---

## Workstreams

## WS1 — Build Graph + Link Closure (Critical Path)

### Objective
Make the native macOS game executable link and run without unresolved symbols.

### Tasks
1. Create/complete CMake target for native game executable (not only probes).
2. Enumerate unresolved symbols (`nm`, linker logs), starting with `__bClientApp` and dependent chains.
3. Wire engine/object libraries explicitly; remove reliance on permissive unresolved-link modes.
4. Gate optional subsystems behind feature flags where unfinished (temporary).

### Exit Criteria
- Clean link for native client target on macOS x64 and arm64.
- No `-undefined dynamic_lookup` dependence for core game binary.

---

## WS2 — Platform Adapter Completion (IPC/Process/Input/Filesystem)

### Objective
Replace temporary non-Windows fallbacks with stable adapters and platform-correct behavior.

### Tasks
1. IPC: migrate `SharedMemory` fallback from compile-only/in-process behavior to validated runtime semantics needed by client flows.
2. Process/launcher: isolate remaining Win32-only process, ToolHelp, and dialog paths behind adapter interfaces.
3. Filesystem/path handling: verify all path separators, case sensitivity issues, and asset-root derivation on macOS/Linux.
4. Input/windowing hooks: validate non-Windows event paths used by `Nksp` startup.

### Exit Criteria
- Runtime-critical non-Windows codepaths have explicit adapter implementations.
- No hard Win32 include dependency on macOS compile path.

---

## WS3 — Rendering + Vulkan/MoltenVK Runtime Hardening

### Objective
Ensure macOS render boot sequence is stable and runtime libraries are discoverable in bundle.

### Tasks
1. Validate Vulkan loader search order against bundled `Contents/Frameworks` libs.
2. Confirm MoltenVK compatibility on Intel + Apple Silicon.
3. Add startup diagnostics (render backend selected, dylib load path, surface creation result).
4. Keep fallback/disable path for experimental Vulkan abstraction if needed.

### Exit Criteria
- Client can initialize render path on macOS test machines (x64 + arm64).
- Bundle starts without requiring external Vulkan install when runtime is bundled.

---

## WS4 — Data/Generated Artifacts Pipeline

### Objective
Eliminate placeholder headers/stubs and ensure deterministic asset/config input requirements.

### Tasks
1. Replace temporary `EntitiesMP` stubs with generated headers from `.es` pipeline.
2. Document/automate header generation (`scripts/generate_es_header_stubs.sh` successor or final generator).
3. Define required asset root contract (`Data/`) and minimum validation checks in scripts.
4. Add preflight checks for missing critical tables/configs before launch.

### Exit Criteria
- Build uses real generated headers (or deterministic generation step).
- Packaging fails fast with actionable errors for missing artifacts.

---

## WS5 — Packaging, Signing, and Distribution Readiness

### Objective
Produce reproducible `.app` / `.dmg` outputs suitable for internal testing and distribution.

### Tasks
1. Keep universal arch controls (`LASTCHAOS_MACOS_ARCHS`) and binary arch validation (`lipo`) in CI/scripts.
2. Validate ad-hoc and developer signing flows; verify Gatekeeper behavior.
3. Add bundle verification command set (`codesign --verify`, `spctl` where available).
4. Add packaging manifest output (binary archs, included dylibs, sl.dta endpoint, asset root hash/size summary).

### Exit Criteria
- One-command reproducible package build.
- Signing/verification checklist passes on macOS test host(s).

---

## WS6 — Cross-Platform Non-Regression (Windows x64 / Linux x64)

### Objective
Prevent macOS work from breaking existing Windows/Linux flows.

### Tasks
1. Add CI matrix stages (or scripted equivalents) for:
   - Windows x64 baseline solution/project build.
   - Linux x64 probe/porting build.
   - macOS x64 + arm64 build/package checks.
2. Enforce platform guard conventions in changed files.
3. Add a portability checklist to PR template (if available in repo workflow).

### Exit Criteria
- Every porting change is exercised by at least one Windows and one Linux build check.

---

## WS7 — End-to-End Validation Milestones

### Milestone M1: Compile/Link
- Native macOS binary compiles+links on x64 and arm64.

### Milestone M2: Launch/Login
- App launches from Finder/Terminal wrapper, loads config, reaches login and completes handshake.

### Milestone M3: Enter World
- Character enters initial zone and remains stable for smoke duration.

### Milestone M4: Packaging GA-Internal
- Signed `.app`/`.dmg` generated reproducibly with documented inputs.

---

## Prioritized Execution Order (Recommended)

1. WS1 Build Graph + Link Closure
2. WS2 Platform Adapter Completion
3. WS4 Data/Generated Artifacts Pipeline
4. WS3 Rendering/Vulkan Hardening
5. WS5 Packaging/Signing
6. WS6 Cross-platform CI non-regression
7. WS7 End-to-end milestone completion

---

## Immediate Next 10 Tasks (actionable backlog)

1. Add native game target blueprint in CMake (inputs/libs/options documented even if partial).
2. Generate first unresolved-symbol report for macOS native target.
3. Classify unresolved symbols by subsystem (engine globals, rendering, IPC, launcher).
4. Replace one Win32 process-launch block in `Nksp` with adapter call + mac implementation stub.
5. Replace one remaining Win32-only IPC callsite with adapter-backed interface.
6. Add script check that confirms generated headers are real (non-placeholder) before full package.
7. Add startup logging for render backend + Vulkan dylib path.
8. Add `codesign --verify` + optional `spctl` summary in packaging script output.
9. Add Linux x64 probe build command to validation checklist.
10. Add Windows x64 baseline build command to validation checklist/documentation.

---

## Risk Register

1. **Link closure risk** — unresolved symbol graph could reveal missing object ownership assumptions from legacy solution.
2. **Runtime behavior risk** — compile-time fallbacks may not match required IPC/process semantics in production.
3. **Asset dependency risk** — missing real Data/generated artifacts can mask true runtime readiness.
4. **Platform drift risk** — macOS fixes can regress Windows unless guarded and tested continuously.

---

## Tracking Format (for each PR)

- Workstream: WS#
- Milestone impact: M#
- Platforms touched: macOS / Windows / Linux
- Risk level: Low/Med/High
- Validation run:
  - build commands
  - runtime smoke result
  - packaging verification result
