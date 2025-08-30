#include "Environment.hpp"


void environment::initialize(lua_State* L)
{
    closure_library::initialize(L); //Soo many errors
    script_library::initialize(L);
    //http_library::initialize(L);
    debug_library::initialize(L);
    //lz4_library::initialize(L);
    //filesystem_library::initialize(L);
    //crypt_library::initialize(L);
    //cache_library::initialize(L);
    metatable_library::initialize(L);
    //reflection_library::initialize(L);
    //websocket_library::initialize(L);
    //instance_library::initialize(L);
    //signal_library::initialize(L);

    misc_library::initialize(L);

	//hooks::initialize(L);

    //luaL_sandboxthread(L);

    //lua_newtable(L);
    //lua_setglobal(L, "_G");

    //lua_newtable(L);
    //lua_setglobal(L, "shared");
}