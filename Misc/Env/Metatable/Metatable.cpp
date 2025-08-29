#include "Metatable.hpp"

namespace Metatable {
    int getrawmetatable(lua_State* L) {
        luaL_trimstack(L, 1);
        luaL_checkany(L, 1);
        if (!lua_getmetatable(L, 1))
            lua_pushnil(L);
        return 1;
    }

    int setrawmetatable(lua_State* L) {
        luaL_trimstack(L, 2);
        luaL_checkany(L, 1);
        luaL_checktype(L, 2, LUA_TTABLE);
        lua_setmetatable(L, 1);
        lua_pushvalue(L, 1);
        return 1;
    }

    int setreadonly(lua_State* L) {
        luaL_trimstack(L, 2);
        luaL_checktype(L, 1, LUA_TTABLE);
        luaL_checktype(L, 2, LUA_TBOOLEAN);
        hvalue(luaA_toobject(L, 1))->readonly = lua_toboolean(L, 2);
        return 0;
    }

    int isreadonly(lua_State* L) {
        luaL_trimstack(L, 1);
        luaL_checktype(L, 1, LUA_TTABLE);
        lua_pushboolean(L, hvalue(luaA_toobject(L, 1))->readonly);
        return 1;
    }
}

void metatable_library::initialize(lua_State* L)
{
    NewFunction(L, "getrawmetatable", Metatable::getrawmetatable);
    NewFunction(L, "setrawmetatable", Metatable::setrawmetatable);
    NewFunction(L, "setreadonly", Metatable::setreadonly);
    NewFunction(L, "isreadonly", Metatable::isreadonly);
}