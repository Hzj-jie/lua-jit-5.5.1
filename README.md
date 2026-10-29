# lua-jit-5.5.1

This repository is a fork of **Lua 5.5.1** modified to be **LuaJIT-compatible** — i.e., able to run code and prebuilt C modules written for LuaJIT directly (which is why it is named `lua-jit`). It is an interpreter without a JIT compiler, with built-in FFI preloaded (backed by embedded `cffi-lua` and static `libffi`).

## Changes for LuaJIT Compatibility

- **Lua 5.1 / LuaJIT C ABI:** Exports removed Lua 5.1 C API functions and `luaL_Buffer` routines, and translates Lua 5.1 pseudo-indices (`LUA_REGISTRYINDEX`, `LUA_GLOBALSINDEX`, upvalues).
- **Syntax:** Supports `LL` / `ULL` 64-bit integer suffixes (producing plain 64-bit integers, with `ULL` above `INT64_MAX` wrapping) and mutable `for`-loop control variables.
- **Runtime & Metamethods:** Supports cross-type userdata `__eq` comparisons (e.g., `cdata` with `nil`, numbers, or tables) and LuaJIT / Lua 5.1 number formatting (`tostring()` and `string.format`).
- **Standard Library:** Includes built-in `bit`, `jit`, `string.buffer` (LuaJIT-compatible binary `encode`/`decode` object serialization), `ffi` (LuaJIT-compatible C FFI backed by embedded `cffi-lua` and `libffi`), `unpack`, `loadstring`, `table.maxn`, and `setfenv` / `getfenv`, plus LuaJIT-compatible `os.execute` status codes, `os.date` format flags (`%-`, `%_`), and `coroutine.running()` on the main thread.

---

## About Lua

This is Lua 5.5.1.

Lua is a powerful, efficient, lightweight, embeddable scripting language developed by a [team](https://www.lua.org/authors.html) at [PUC-Rio](https://www.puc-rio.br/), the Pontifical Catholic University of Rio de Janeiro in Brazil. Lua is [free software](https://www.lua.org/license.html) used in [many products and projects](https://www.lua.org/uses.html) around the world.

Lua's [official website](https://www.lua.org/) provides complete information about Lua, including an [executive summary](https://www.lua.org/about.html), tips on [getting started](https://www.lua.org/start.html), and updated [documentation](https://www.lua.org/docs.html), especially the [reference manual](https://www.lua.org/manual/5.5/).

For installation instructions, license details, and further information about Lua, see https://www.lua.org/manual/5.5/readme.html.
