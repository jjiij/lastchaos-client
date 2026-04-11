# Client Bootstrap

This is the first client-side baseline for the extracted source tree:

- Source root: `client-source/source`
- Main solution: `LCClient.sln`
- Primary game project: `GameMP/GameMP.vcxproj`

## What The Client Expects

The login endpoint is read from `sl.dta` at runtime.  
Code path: `Engine/Network/CommunicationInterface.cpp` (`ReadInfo()` in the current tree).

`sl.dta` must exist in the executable directory (`_fnmApplicationPath.FileDir()`).

## Local Server Wiring (First Target)

Goal: point client login to your local server using `127.0.0.1`.

1. Build client in Visual Studio (recommended first pass: `Template|x64`).
2. Identify runtime output folder for your chosen configuration (for example `Bin/` or configuration-specific output).
3. Generate `sl.dta` with local login endpoint.
4. Place generated `sl.dta` next to the client executable.

## `sl.dta` Tooling

Helper tool:

- `client-source/tools/sl_dta_tool.go`

Decode existing file:

```bash
go run client-source/tools/sl_dta_tool.go decode -in /path/to/sl.dta
```

Create local file for login at `127.0.0.1:4001`:

```bash
go run client-source/tools/sl_dta_tool.go encode \
  -out /path/to/client/bin/sl.dta \
  -entry "Local 127.0.0.1 4001 1000 300"
```

Entry format is exactly:

`"name address port full_users busy_users"`

## Immediate Next Milestones

1. Confirm correct login port from your running server setup.
2. Build and launch client with generated `sl.dta`.
3. Capture first connection attempt logs (success/fail path) to validate protocol compatibility.
