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
** LuaJIT-compatible 'string.buffer' module (encode/decode)
*/

typedef struct {
  int boxidx;
  size_t size;
  size_t capacity;
} EncBuffer;

static void enc_write (lua_State *L, EncBuffer *b, const void *p, size_t n) {
  if (b->size + n > b->capacity) {
    size_t newcap = b->capacity == 0 ? 128 : b->capacity * 2;
    char *olddata;
    void *newdata;
    while (newcap < b->size + n) newcap *= 2;
    newdata = lua_newuserdatauv(L, newcap, 0);
    olddata = (char *)lua_touserdata(L, b->boxidx);
    if (b->size > 0 && olddata != NULL)
      memcpy(newdata, olddata, b->size);
    lua_replace(L, b->boxidx);
    b->capacity = newcap;
  }
  memcpy((char *)lua_touserdata(L, b->boxidx) + b->size, p, n);
  b->size += n;
}

static void enc_byte (lua_State *L, EncBuffer *b, uint8_t byte) {
  enc_write(L, b, &byte, 1);
}

static void enc_val (lua_State *L, EncBuffer *b, int idx, int depth) {
  int t;
  luaL_checkstack(L, 3, "buffer overflow");
  if (depth > 256)
    luaL_error(L, "nesting too deep in string.buffer.encode");
  idx = lua_absindex(L, idx);
  t = lua_type(L, idx);
  switch (t) {
    case LUA_TNIL:
      enc_byte(L, b, 0);
      break;
    case LUA_TBOOLEAN:
      enc_byte(L, b, (uint8_t)(lua_toboolean(L, idx) ? 2 : 1));
      break;
    case LUA_TNUMBER: {
      if (lua_isinteger(L, idx)) {
        int64_t v = (int64_t)lua_tointeger(L, idx);
        enc_byte(L, b, 3);
        enc_write(L, b, &v, sizeof(v));
      }
      else {
        double v = (double)lua_tonumber(L, idx);
        enc_byte(L, b, 4);
        enc_write(L, b, &v, sizeof(v));
      }
      break;
    }
    case LUA_TSTRING: {
      size_t len;
      const char *s = lua_tolstring(L, idx, &len);
      uint32_t ulen = (uint32_t)len;
      enc_byte(L, b, 5);
      enc_write(L, b, &ulen, sizeof(ulen));
      enc_write(L, b, s, len);
      break;
    }
    case LUA_TTABLE: {
      uint32_t count = 0;
      lua_pushnil(L);
      while (lua_next(L, idx)) {
        count++;
        lua_pop(L, 1);
      }
      enc_byte(L, b, 6);
      enc_write(L, b, &count, sizeof(count));
      lua_pushnil(L);
      while (lua_next(L, idx)) {
        enc_val(L, b, -2, depth + 1);
        enc_val(L, b, -1, depth + 1);
        lua_pop(L, 1);
      }
      break;
    }
    default:
      luaL_error(L, "cannot encode %s", lua_typename(L, t));
      break;
  }
}

typedef struct {
  const uint8_t *p;
  const uint8_t *end;
} DecBuffer;

static void dec_read (lua_State *L, DecBuffer *d, void *dest, size_t n) {
  if ((size_t)(d->end - d->p) < n)
    luaL_error(L, "truncated buffer in string.buffer.decode");
  memcpy(dest, d->p, n);
  d->p += n;
}

static uint8_t dec_byte (lua_State *L, DecBuffer *d) {
  uint8_t b;
  dec_read(L, d, &b, 1);
  return b;
}

static void dec_val (lua_State *L, DecBuffer *d, int depth) {
  uint8_t tag;
  luaL_checkstack(L, 3, "buffer overflow");
  if (depth > 256)
    luaL_error(L, "nesting too deep in string.buffer.decode");
  tag = dec_byte(L, d);
  switch (tag) {
    case 0:
      lua_pushnil(L);
      break;
    case 1:
      lua_pushboolean(L, 0);
      break;
    case 2:
      lua_pushboolean(L, 1);
      break;
    case 3: {
      int64_t v;
      dec_read(L, d, &v, sizeof(v));
      lua_pushinteger(L, (lua_Integer)v);
      break;
    }
    case 4: {
      double v;
      dec_read(L, d, &v, sizeof(v));
      lua_pushnumber(L, (lua_Number)v);
      break;
    }
    case 5: {
      uint32_t len;
      dec_read(L, d, &len, sizeof(len));
      if ((size_t)(d->end - d->p) < (size_t)len)
        luaL_error(L, "truncated string in string.buffer.decode");
      lua_pushlstring(L, (const char *)d->p, (size_t)len);
      d->p += len;
      break;
    }
    case 6: {
      uint32_t count, i;
      dec_read(L, d, &count, sizeof(count));
      lua_createtable(L, 0, (int)count);
      for (i = 0; i < count; i++) {
        dec_val(L, d, depth + 1); /* key */
        dec_val(L, d, depth + 1); /* value */
        lua_settable(L, -3);
      }
      break;
    }
    default:
      luaL_error(L, "invalid tag %d in string.buffer.decode", (int)tag);
      break;
  }
}

static int buf_encode (lua_State *L) {
  EncBuffer b;
  luaL_checkany(L, 1);
  lua_newuserdatauv(L, 128, 0);
  b.boxidx = lua_gettop(L);
  b.size = 0;
  b.capacity = 128;
  enc_val(L, &b, 1, 0);
  lua_pushlstring(L, (const char *)lua_touserdata(L, b.boxidx), b.size);
  return 1;
}

static int buf_decode (lua_State *L) {
  size_t len;
  const char *s = luaL_checklstring(L, 1, &len);
  DecBuffer d;
  d.p = (const uint8_t *)s;
  d.end = (const uint8_t *)s + len;
  dec_val(L, &d, 0);
  return 1;
}

static const luaL_Reg strbuf_funcs[] = {
  {"encode", buf_encode},
  {"decode", buf_decode},
  {NULL, NULL}
};

LUAMOD_API int luaopen_string_buffer (lua_State *L) {
  luaL_newlib(L, strbuf_funcs);
  lua_getglobal(L, "string");
  if (lua_istable(L, -1)) {
    lua_pushvalue(L, -2);
    lua_setfield(L, -2, "buffer");
  }
  lua_pop(L, 1);
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
  lua_pushcfunction(L, luaopen_string_buffer);
  lua_setfield(L, -2, "string.buffer");
  lua_pop(L, 1);  /* remove PRELOAD table */
}

