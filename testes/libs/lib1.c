#include "lua.h"
#include "lauxlib.h"
#include <string.h>

static int id (lua_State *L) {
  return lua_gettop(L);
}


static const struct luaL_Reg funcs[] = {
  {"id", id},
  {NULL, NULL}
};


/* function used by lib11.c */
LUAMOD_API int lib1_export (lua_State *L) {
  lua_pushstring(L, "exported");
  return 1;
}


LUAMOD_API int onefunction (lua_State *L) {
  luaL_checkversion(L);
  lua_settop(L, 2);
  lua_pushvalue(L, 1);
  return 2;
}


LUAMOD_API int anotherfunc (lua_State *L) {
  luaL_checkversion(L);
  lua_pushfstring(L, "%d%%%d\n", (int)lua_tointeger(L, 1),
                                 (int)lua_tointeger(L, 2));
  return 1;
}


static int check_upvalue (lua_State *L) {
  /* upvalue 1 is at -10003 */
  const char *s = lua_tostring(L, -10003);
  if (!s || strcmp(s, "upval1") != 0) return luaL_error(L, "upvalue -10003 failed");
  lua_pushboolean(L, 1);
  return 1;
}


static int try_replace_env (lua_State *L) {
  lua_pushnumber(L, 1);
  lua_replace(L, -10001);
  return 0;
}


static int try_replace_globals (lua_State *L) {
  lua_pushnumber(L, 1);
  lua_replace(L, -10002);
  return 0;
}


static int try_access_env (lua_State *L) {
  lua_pushvalue(L, -10001);
  return 1;
}


/* 5.1-style prototypes to test width conversion (signed 32-bit int) */
#undef lua_rawgeti
#undef lua_rawseti
#undef lua_tointeger
extern int (lua_rawgeti) (lua_State *L, int idx, int n);
extern void (lua_rawseti) (lua_State *L, int idx, int n);
extern ptrdiff_t (lua_tointeger) (lua_State *L, int idx);


LUAMOD_API int test_c_abi (lua_State *L) {
  void *ud;
  /* 1. lua_tointeger truncation */
  lua_pushnumber(L, 2.9);
  if (lua_tointeger(L, -1) != 2) return luaL_error(L, "lua_tointeger failed");
  lua_pop(L, 1);

  /* 2. lua_equal and lua_objlen */
  lua_pushstring(L, "hello");
  lua_pushstring(L, "hello");
  if (!lua_equal(L, -1, -2)) return luaL_error(L, "lua_equal failed");
  if (lua_objlen(L, -1) != 5) return luaL_error(L, "lua_objlen failed");
  lua_pop(L, 2);

  /* 3. lua_insert, lua_remove, lua_replace */
  lua_pushinteger(L, 10);
  lua_pushinteger(L, 20);
  lua_insert(L, -2); /* stack: 20, 10 */
  if (lua_tointeger(L, -2) != 20 || lua_tointeger(L, -1) != 10)
    return luaL_error(L, "lua_insert failed");
  lua_remove(L, -2); /* stack: 10 */
  if (lua_tointeger(L, -1) != 10) return luaL_error(L, "lua_remove failed");
  lua_pushinteger(L, 30);
  lua_replace(L, -2); /* stack: 30 */
  if (lua_tointeger(L, -1) != 30) return luaL_error(L, "lua_replace failed");
  lua_pop(L, 1);

  /* 4. pseudo-indices: -10000 (registry) and -10002 (globals) */
  lua_pushstring(L, "test_global_val");
  lua_setfield(L, -10002, "test_global_key");
  lua_getfield(L, -10002, "test_global_key");
  if (strcmp(lua_tostring(L, -1), "test_global_val") != 0)
    return luaL_error(L, "globals pseudo-index -10002 failed");
  lua_pop(L, 1);

  /* 5. upvalues via pseudo-index -10003 */
  lua_pushstring(L, "upval1");
  lua_pushcclosure(L, check_upvalue, 1);
  lua_call(L, 0, 1);
  if (!lua_toboolean(L, -1)) return luaL_error(L, "check_upvalue failed");
  lua_pop(L, 1);

  /* 6. lua_newuserdata and lua_getfenv / lua_setfenv */
  ud = lua_newuserdata(L, 16);
  if (!ud) return luaL_error(L, "lua_newuserdata failed");
  lua_getfenv(L, -1);
  /* default env for new userdata is globals table */
  lua_getfield(L, -1, "test_global_key");
  if (strcmp(lua_tostring(L, -1), "test_global_val") != 0)
    return luaL_error(L, "userdata default env failed");
  lua_pop(L, 2); /* pop field and env table */
  /* custom env table */
  lua_newtable(L);
  lua_pushstring(L, "custom_val");
  lua_setfield(L, -2, "custom_key");
  lua_setfenv(L, -2);
  lua_getfenv(L, -1);
  lua_getfield(L, -1, "custom_key");
  if (strcmp(lua_tostring(L, -1), "custom_val") != 0)
    return luaL_error(L, "lua_setfenv failed");
  lua_pop(L, 3); /* pop field, env, userdata */

  /* 7. luaL_register */
  luaL_register(L, "test_reg_lib", NULL);
  if (!lua_istable(L, -1)) return luaL_error(L, "luaL_register failed");
  lua_pop(L, 1);

  /* 8. lua_call and lua_pcall */
  lua_pushcfunction(L, id);
  lua_pushinteger(L, 42);
  lua_call(L, 1, 1);
  if (lua_tointeger(L, -1) != 42) return luaL_error(L, "lua_call failed");
  lua_pop(L, 1);

  lua_pushcfunction(L, id);
  lua_pushinteger(L, 99);
  if (lua_pcall(L, 1, 1, 0) != 0) return luaL_error(L, "lua_pcall failed");
  if (lua_tointeger(L, -1) != 99) return luaL_error(L, "lua_pcall res failed");
  lua_pop(L, 1);

  /* 9. negative key with 5.1 prototype (int n) */
  lua_newtable(L);
  lua_pushstring(L, "neg_val");
  lua_rawseti(L, -2, -1);
  lua_geti(L, -1, -1);
  if (lua_type(L, -1) != LUA_TSTRING || strcmp(lua_tostring(L, -1), "neg_val") != 0)
    return luaL_error(L, "lua_geti failed after 5.1 lua_rawseti with negative int");
  lua_pop(L, 1);
  lua_rawgeti(L, -1, -1);
  if (lua_type(L, -1) != LUA_TSTRING || strcmp(lua_tostring(L, -1), "neg_val") != 0)
    return luaL_error(L, "5.1 lua_rawgeti/rawseti with negative int failed");
  lua_pop(L, 2);

  /* 10. pcall checks for -10001 access and -10001/-10002 replacement */
  lua_pushcfunction(L, try_access_env);
  if (lua_pcall(L, 0, 1, 0) == 0)
    return luaL_error(L, "accessing -10001 should fail");
  lua_pop(L, 1);

  lua_pushcfunction(L, try_replace_env);
  if (lua_pcall(L, 0, 0, 0) == 0)
    return luaL_error(L, "replacing -10001 should fail");
  lua_pop(L, 1);

  lua_pushcfunction(L, try_replace_globals);
  if (lua_pcall(L, 0, 0, 0) == 0)
    return luaL_error(L, "replacing -10002 should fail");
  lua_pop(L, 1);

  lua_pushboolean(L, 1);
  return 1;
} 


LUAMOD_API int luaopen_lib1_sub (lua_State *L) {
  lua_setglobal(L, "y");  /* 2nd arg: extra value (file name) */
  lua_setglobal(L, "x");  /* 1st arg: module name */
  luaL_newlib(L, funcs);
  return 1;
}

