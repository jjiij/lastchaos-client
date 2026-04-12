# Client Bootstrap

Minimum local bootstrap:

1. Build `source/LCClient.sln` with `Template|Win32` or `Template|x64`.
2. Ensure `sl.dta` exists in runtime folder (`source/Bin/`).
3. Start client from `source/Bin/` output.

`sl.dta` helper tool:
- `tools/sl_dta_tool.go`
