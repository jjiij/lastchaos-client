# LC2018 VS2019 DXVK Migration Notes

This workspace now uses the LC2018 client baseline from:

- `_references/LCSources-DXVK9/VSProjects/LC2018/Client/Sources.VS2019.DXVK`

and support assets from:

- `Tools.Win32`
- `Shaders`
- `Classes`
- `Option_Game.xml.BL.VK`
- `Option_Game.xml.BLess`
- `Readme.txt`

## Build Configuration

- Target: `LCRelease|x64`
- Preferred IDE/toolchain: Visual Studio 2019 style project settings (`v142` toolset in project files).
- GitHub workflow is Windows-only x64.
- Workflow Boost defaults to `1.78.0` (matching vendored LC2018 Boost headers in `Tools.Win32/Include/boost`).
- To switch Boost version in CI, update job env values in `.github/workflows/cross-platform-build.yml`:
  - `BOOST_VERSION`
  - `BOOST_UNDERSCORE`
  - `BOOST_LIB_TAG`

## Graphics API Setup

Runtime selector file:

- `Data/etc/GfxAPI.txt`

Values:

- `0` = OpenGL
- `1` = DirectX 9
- `2` = Vulkan

## Required Option File Copy (from upstream notes)

For the updated BL.VK video menu behavior, ensure:

- `Option_Game.xml.BL.VK` is copied to `Data/Interface/xml/Option_Game.xml`

The workflow does this during bundling.

## Known Upstream Notes

- Switching Vulkan -> DX9 may fail on some adapters/drivers.
- RTSS/RivaTuner can crash when changing resolution/API.
- Some display resolutions may not show the character model in selection menu.

## ECC

Project files still run `ecc` on `.es` script sources in `Engine` and `EntitiesMP` custom build steps.
A working `ecc.exe` remains required for full solution builds.
