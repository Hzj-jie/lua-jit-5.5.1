/*
** $Id: linit.c $
** Initialization of libraries for lua.c and other clients
** See Copyright Notice in lua.h
*/


#define linit_c
#define LUA_LIB


#include "lprefix.h"


#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "lua.h"

#include "lualib.h"
#include "lauxlib.h"
#include "llimits.h"


/*
** LuaJIT-compatible 'bit' library
*/

typedef union {
  lua_Number n;
#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
  struct { uint32_t hi; uint32_t lo; } b;
#else
  struct { uint32_t lo; uint32_t hi; } b;
#endif
} BitNum;

static int32_t barg (lua_State *L, int idx) {
  BitNum bn;
  if (lua_isinteger(L, idx))
    return (int32_t)(uint32_t)lua_tointeger(L, idx);
  bn.n = luaL_checknumber(L, idx) + 6755399441055744.0;
  return (int32_t)bn.b.lo;
}

#define tobit(L, idx)  barg(L, idx)

static int bit_tobit (lua_State *L) {
  lua_pushinteger(L, (lua_Integer)tobit(L, 1));
  return 1;
}

static int bit_tohex (lua_State *L) {
  uint32_t b = (uint32_t)tobit(L, 1);
  lua_Integer n = luaL_optinteger(L, 2, 8);
  const char *hexdigits = "0123456789abcdef";
  char buf[128];
  int un;
  if (n < 0) {
    un = (int)(-n);
    hexdigits = "0123456789ABCDEF";
  }
  else {
    un = (int)n;
  }
  if (un > 99) un = 99;
  if (un == 0) {
    lua_pushliteral(L, "");
  }
  else if (un > 8) {
    int zeros = un - 8;
    int i;
    for (i = 0; i < zeros; i++) buf[i] = '0';
    for (i = 7; i >= 0; i--) buf[zeros + (7 - i)] = hexdigits[(b >> (i * 4)) & 0xf];
    buf[un] = '\0';
    lua_pushlstring(L, buf, (size_t)un);
  }
  else {
    int i;
    for (i = un - 1; i >= 0; i--) buf[(un - 1) - i] = hexdigits[(b >> (i * 4)) & 0xf];
    buf[un] = '\0';
    lua_pushlstring(L, buf, (size_t)un);
  }
  return 1;
}

static int bit_bnot (lua_State *L) {
  uint32_t b = (uint32_t)tobit(L, 1);
  lua_pushinteger(L, (lua_Integer)(int32_t)(~b));
  return 1;
}

static int bit_band (lua_State *L) {
  int i, n = lua_gettop(L);
  uint32_t b = 0xffffffffU;
  for (i = 1; i <= n; i++)
    b &= (uint32_t)tobit(L, i);
  lua_pushinteger(L, (lua_Integer)(int32_t)b);
  return 1;
}

static int bit_bor (lua_State *L) {
  int i, n = lua_gettop(L);
  uint32_t b = 0;
  for (i = 1; i <= n; i++)
    b |= (uint32_t)tobit(L, i);
  lua_pushinteger(L, (lua_Integer)(int32_t)b);
  return 1;
}

static int bit_bxor (lua_State *L) {
  int i, n = lua_gettop(L);
  uint32_t b = 0;
  for (i = 1; i <= n; i++)
    b ^= (uint32_t)tobit(L, i);
  lua_pushinteger(L, (lua_Integer)(int32_t)b);
  return 1;
}

static int bit_lshift (lua_State *L) {
  uint32_t b = (uint32_t)tobit(L, 1);
  uint32_t s = (uint32_t)tobit(L, 2) & 31;
  lua_pushinteger(L, (lua_Integer)(int32_t)(b << s));
  return 1;
}

static int bit_rshift (lua_State *L) {
  uint32_t b = (uint32_t)tobit(L, 1);
  uint32_t s = (uint32_t)tobit(L, 2) & 31;
  lua_pushinteger(L, (lua_Integer)(uint32_t)(b >> s));
  return 1;
}

static int bit_arshift (lua_State *L) {
  int32_t b = tobit(L, 1);
  uint32_t s = (uint32_t)tobit(L, 2) & 31;
  lua_pushinteger(L, (lua_Integer)(b >> s));
  return 1;
}

static int bit_rol (lua_State *L) {
  uint32_t b = (uint32_t)tobit(L, 1);
  uint32_t s = (uint32_t)tobit(L, 2) & 31;
  uint32_t res = s ? ((b << s) | (b >> (32 - s))) : b;
  lua_pushinteger(L, (lua_Integer)(int32_t)res);
  return 1;
}

static int bit_ror (lua_State *L) {
  uint32_t b = (uint32_t)tobit(L, 1);
  uint32_t s = (uint32_t)tobit(L, 2) & 31;
  uint32_t res = s ? ((b >> s) | (b << (32 - s))) : b;
  lua_pushinteger(L, (lua_Integer)(int32_t)res);
  return 1;
}

static int bit_bswap (lua_State *L) {
  uint32_t b = (uint32_t)tobit(L, 1);
  uint32_t res = ((b << 24) & 0xff000000U) |
                 ((b << 8)  & 0x00ff0000U) |
                 ((b >> 8)  & 0x0000ff00U) |
                 ((b >> 24) & 0x000000ffU);
  lua_pushinteger(L, (lua_Integer)(int32_t)res);
  return 1;
}

static const luaL_Reg bit_funcs[] = {
  {"tobit", bit_tobit},
  {"tohex", bit_tohex},
  {"bnot", bit_bnot},
  {"band", bit_band},
  {"bor", bit_bor},
  {"bxor", bit_bxor},
  {"lshift", bit_lshift},
  {"rshift", bit_rshift},
  {"arshift", bit_arshift},
  {"rol", bit_rol},
  {"ror", bit_ror},
  {"bswap", bit_bswap},
  {NULL, NULL}
};

LUAMOD_API int luaopen_bit (lua_State *L) {
  luaL_newlib(L, bit_funcs);
  return 1;
}


/*
** LuaJIT-compatible 'jit' module stub
*/

#if defined(__linux__) || defined(__linux) || defined(linux)
#define JIT_OS "Linux"
#elif defined(__APPLE__) || defined(__MACH__)
#define JIT_OS "OSX"
#elif defined(_WIN32) || defined(_WIN64)
#define JIT_OS "Windows"
#elif defined(__FreeBSD__) || defined(__FreeBSD_kernel__)
#define JIT_OS "BSD"
#elif defined(__sun) || defined(sun)
#define JIT_OS "Solaris"
#else
#define JIT_OS "Other"
#endif

#if defined(__x86_64__) || defined(_M_X64)
#define JIT_ARCH "x64"
#elif defined(__i386__) || defined(_M_IX86)
#define JIT_ARCH "x86"
#elif defined(__aarch64__) || defined(_M_ARM64)
#define JIT_ARCH "arm64"
#elif defined(__arm__) || defined(_M_ARM)
#define JIT_ARCH "arm"
#elif defined(__mips64)
#define JIT_ARCH "mips64"
#elif defined(__mips__) || defined(__mips)
#define JIT_ARCH "mips"
#elif defined(__powerpc__) || defined(__ppc__)
#define JIT_ARCH "ppc"
#elif defined(__riscv) && (__riscv_xlen == 64)
#define JIT_ARCH "riscv64"
#else
#define JIT_ARCH "unknown"
#endif

static int jit_status (lua_State *L) {
  lua_pushboolean(L, 0);
  return 1;
}

static int jit_noop (lua_State *L) {
  (void)L;
  return 0;
}

LUAMOD_API int luaopen_jit (lua_State *L) {
  lua_newtable(L);
  lua_pushstring(L, JIT_OS);
  lua_setfield(L, -2, "os");
  lua_pushstring(L, JIT_ARCH);
  lua_setfield(L, -2, "arch");
  lua_pushstring(L, "LuaJIT 2.1.0");
  lua_setfield(L, -2, "version");
  lua_pushinteger(L, 20100);
  lua_setfield(L, -2, "version_num");
  lua_pushcfunction(L, jit_status);
  lua_setfield(L, -2, "status");
  lua_pushcfunction(L, jit_noop);
  lua_setfield(L, -2, "on");
  lua_pushcfunction(L, jit_noop);
  lua_setfield(L, -2, "off");
  lua_pushcfunction(L, jit_noop);
  lua_setfield(L, -2, "flush");

  lua_newtable(L);
  lua_pushcfunction(L, jit_noop);
  lua_setfield(L, -2, "start");
  lua_setfield(L, -2, "opt");

  return 1;
}


/*
** Standard Libraries. (Must be listed in the same ORDER of their
** respective constants LUA_<libname>K.)
*/
static const luaL_Reg stdlibs[] = {
  {LUA_GNAME, luaopen_base},
  {LUA_LOADLIBNAME, luaopen_package},
  {LUA_COLIBNAME, luaopen_coroutine},
  {LUA_DBLIBNAME, luaopen_debug},
  {LUA_IOLIBNAME, luaopen_io},
  {LUA_MATHLIBNAME, luaopen_math},
  {LUA_OSLIBNAME, luaopen_os},
  {LUA_STRLIBNAME, luaopen_string},
  {LUA_TABLIBNAME, luaopen_table},
  {LUA_UTF8LIBNAME, luaopen_utf8},
  {NULL, NULL}
};


/*
** require and preload selected standard libraries
*/
LUALIB_API void luaL_openselectedlibs (lua_State *L, int load, int preload) {
  int mask;
  const luaL_Reg *lib;
  luaL_getsubtable(L, LUA_REGISTRYINDEX, LUA_PRELOAD_TABLE);
  for (lib = stdlibs, mask = 1; lib->name != NULL; lib++, mask <<= 1) {
    if (load & mask) {  /* selected? */
      luaL_requiref(L, lib->name, lib->func, 1);  /* require library */
      lua_pop(L, 1);  /* remove result from the stack */
    }
    else if (preload & mask) {  /* selected? */
      lua_pushcfunction(L, lib->func);
      lua_setfield(L, -2, lib->name);  /* add library to PRELOAD table */
    }
  }
  lua_assert((mask >> 1) == LUA_UTF8LIBK);
  if (load) {
    luaL_requiref(L, "bit", luaopen_bit, 1);
    lua_pop(L, 1);
    luaL_requiref(L, "jit", luaopen_jit, 1);
    lua_pop(L, 1);
  }
  else if (preload) {
    lua_pushcfunction(L, luaopen_bit);
    lua_setfield(L, -2, "bit");
    lua_pushcfunction(L, luaopen_jit);
    lua_setfield(L, -2, "jit");
  }
  lua_pop(L, 1);  /* remove PRELOAD table */
}

