#include "Script.hpp"

int gettenv(lua_State* L) {
    luaL_trimstack(L, 1);
    luaL_checktype(L, 1, LUA_TTHREAD);
    lua_State* ls = (lua_State*)lua_topointer(L, 1);
    LuaTable* tab = hvalue(luaA_toobject(ls, LUA_GLOBALSINDEX));

    sethvalue(L, L->top, tab);
    L->top++;

    return 1;
};

int getgenv(lua_State* L) {
    luaL_trimstack(L, 0);
    lua_pushvalue(L, LUA_ENVIRONINDEX);
    return 1;
}
/*
int getrenv(lua_State* L) {
    luaL_trimstack(L, 0);
    lua_State* RobloxState = globals::Polaris_state;
    LuaTable* clone = luaH_clone(L, RobloxState->gt);

    lua_rawcheckstack(L, 1);
    luaC_threadbarrier(L);
    luaC_threadbarrier(RobloxState);

    L->top->value.p = clone;
    L->top->tt = LUA_TTABLE;
    L->top++;

    lua_rawgeti(L, LUA_REGISTRYINDEX, 2);
    lua_setfield(L, -2, "_G");
    lua_rawgeti(L, LUA_REGISTRYINDEX, 4);
    lua_setfield(L, -2, "shared");
    return 1;
}
*/
int getgc(lua_State* L) {
    luaL_trimstack(L, 1);
    const bool includeTables = luaL_optboolean(L, 1, false);

    lua_newtable(L);
    lua_newtable(L);

    lua_pushstring(L, "kvs");
    lua_setfield(L, -2, "__mode");

    lua_setmetatable(L, -2);

    typedef struct {
        lua_State* luaThread;
        bool includeTables;
        int itemsFound;
    } GCOContext;

    auto GCContext = GCOContext{ L, includeTables, 0 };

    const auto oldGCThreshold = L->global->GCthreshold;
    L->global->GCthreshold = SIZE_MAX;

    luaM_visitgco(L, &GCContext, [](void* ctx, lua_Page* page, GCObject* gcObj) -> bool {
        const auto context = static_cast<GCOContext*>(ctx);
        const auto luaThread = context->luaThread;

        if (isdead(luaThread->global, gcObj))
            return false;

        const auto gcObjectType = gcObj->gch.tt;
        if (gcObjectType == LUA_TFUNCTION || gcObjectType == LUA_TTHREAD || gcObjectType == LUA_TUSERDATA ||
            gcObjectType == LUA_TLIGHTUSERDATA ||
            gcObjectType == LUA_TBUFFER || gcObjectType == LUA_TTABLE && context->includeTables) {
            luaThread->top->value.gc = gcObj;
            luaThread->top->tt = gcObjectType;
            incr_top(luaThread);

            const auto newTableIndex = context->itemsFound++;
            lua_rawseti(luaThread, -2, newTableIndex);
        }

        return false;
        });

    L->global->GCthreshold = oldGCThreshold;

    return 1;
};

int getreg(lua_State* L) {
    luaL_trimstack(L, 0);

    lua_rawcheckstack(L, 1);
    luaC_threadbarrier(L);

    lua_pushvalue(L, LUA_REGISTRYINDEX);
    return 1;
};
/*

static std::string GetBytecode(lua_State* L)
{
    lua_normalisestack(L, 1);
    luaL_checktype(L, 1, LUA_TUSERDATA);

    std::string typeName = luaL_typename(L, 1);
    if (typeName != "Instance") {
        luaL_typeerror(L, 1, "Instance");
    }

    void* scriptPtr = lua_touserdata(L, 1);
    if (!scriptPtr) {
        luaL_error(L, "unable to get instance");
    }

    uintptr_t script = *reinterpret_cast<uintptr_t*>(scriptPtr);
    if (!script) {
        luaL_error(L, "unable to get script pointer");
    }

    lua_getfield(L, 1, "ClassName");
    std::string className = lua_tostring(L, -1);
    lua_pop(L, 1);

    if (className != "AuroraScript"
        && className != "ModuleScript"
        && className != "Script"
        && className != "LocalScript") {
        luaL_error(L, "expected AuroraScript, ModuleScript, Script or LocalScript");
    }

    uintptr_t protectedString = 0;
    if (className == "ModuleScript") {
        protectedString = *reinterpret_cast<uintptr_t*>(script + update::offsets::bytecode::ModuleScriptByteCode);
    }
    else {
        protectedString = *reinterpret_cast<uintptr_t*>(script + update::offsets::bytecode::LocalScriptByteCode);
    }

    return *reinterpret_cast<std::string*>(protectedString + 0x10);
}

int getscriptclosure(lua_State* L)
{
    std::string compressedBytecode = GetBytecode(L);
    if (compressedBytecode.empty()) {
        lua_pushnil(L);
        return 1;
    }

    lua_State* L2 = lua_newthread(L);
    lua_pop(L, 1);
    luaL_sandboxthread(L2);
    context_manager::set_thread_capabilities(globals::Polaris_state, 8, max_caps);

    lua_pushvalue(L, 1);
    lua_xmove(L, L2, 1);
    lua_setglobal(L2, "script");

    int result = roblox::LuaVM__Load(L2, &compressedBytecode, "_", lua_isnoneornil(L2, -1) ? 0 : -1);
    if (result != LUA_OK) {
        lua_pop(L2, 1);
        lua_pushnil(L);
        return 1;
    }

    Closure* closure = clvalue(luaA_toobject(L2, -1));
    if (closure) {
        Proto* p = closure->l.p;
        if (p) {
            context_manager::set_proto_capabilities(p, &max_caps);
        }
    }

    lua_pop(L2, lua_gettop(L2));
    lua_pop(L, lua_gettop(L));

    setclvalue(L, L->top, closure);
    incr_top(L);
    return 1;
}
*/
/*
std::string read_bytecode(uintptr_t addr) {
	auto str = addr + 0x10;
	auto len = *(size_t*)(str + 0x10);
	auto data = *(size_t*)(str + 0x18) > 0xf ? *(uintptr_t*) (str) : str;
	return std::string((char*)(data), len);
}

int getscriptbytecode(lua_State* L) {
    luaL_checktype(L, 1, LUA_TUSERDATA);

    void* userdata_block = lua_touserdata(L, 1);
    uintptr_t script_pointer = userdata_block ? *(uintptr_t*)userdata_block : 0;

    if (!script_pointer) {
        lua_pushnil(L);
        return 1;
    }

    lua_getfield(L, 1, "ClassName");
    std::string className = lua_tostring(L, -1);
    lua_pop(L, 1);

    uintptr_t protectedString = 0;
    if (className == "ModuleScript") {
        protectedString = *reinterpret_cast<uintptr_t*>(script_pointer + update::offsets::bytecode::ModuleScriptByteCode);
    }
    else {
        protectedString = *reinterpret_cast<uintptr_t*>(script_pointer + update::offsets::bytecode::LocalScriptByteCode);
    }

    if (!protectedString) {
        lua_pushnil(L);
        return 1;
    }

    // Decompress per sUNC: return nil if no bytecode
    auto decompressed = global_functions::decompress_bytecode(read_bytecode(protectedString));
    if (decompressed.empty()) { lua_pushnil(L); return 1; }
    lua_pushlstring(L, decompressed.data(), decompressed.size());
    return 1;
}*/


/*
int getscripts(lua_State* L) {
    struct instancecontext {
        lua_State* L;
        __int64 n;
    };

    instancecontext Context = { L, 0 };

    lua_createtable(L, 0, 0);

    const auto originalGCThreshold = L->global->GCthreshold;
    L->global->GCthreshold = SIZE_MAX;

    luaM_visitgco(L, &Context, [](void* ctx, lua_Page* page, GCObject* gco) -> bool {
        auto context = static_cast<instancecontext*>(ctx);
        lua_State* L = context->L;

        if (isdead(L->global, gco))
            return false;

        if (gco->gch.tt == LUA_TUSERDATA) {
            TValue* top = L->top;
            top->value.p = reinterpret_cast<void*>(gco);
            top->tt = LUA_TUSERDATA;
            L->top++;

            if (strcmp(luaL_typename(L, -1), "Instance") == 0) {
                lua_getfield(L, -1, "ClassName");
                const char* className = lua_tolstring(L, -1, 0);

                if (className && (
                    strcmp(className, "LocalScript") == 0 ||
                    strcmp(className, "ModuleScript") == 0 ||
                    strcmp(className, "Script") == 0))
                {
                    lua_pop(L, 1);

                    lua_getfield(L, -1, "Parent");
                    if (lua_isnil(L, -1)) {
                        lua_pop(L, 1);

                        context->n++;
                        lua_rawseti(L, -2, context->n);
                    }
                    else {
                        lua_pop(L, 2);
                    }
                }
                else {
                    lua_pop(L, 2);
                }
            }
            else {
                lua_pop(L, 1);
            }
        }

        return true;
        });

    L->global->GCthreshold = originalGCThreshold;

    return 1;
}*/

void script_library::initialize(lua_State* L) //commented out's are going to be developed another day. 
{
    //NewFunction(L, "getscriptclosure", getscriptclosure);

    //NewFunction(L, "getscriptfunction", getscriptclosure); //ALIAS
    
    NewFunction(L, "gettenv", gettenv);

    NewFunction(L, "getgenv", getgenv);

   // NewFunction(L, "getrenv", getrenv);

    NewFunction(L, "getgc", getgc);

    NewFunction(L, "getreg", getreg);

    //NewFunction(L, "getsenv", getsenv);

   // NewFunction(L, "getscriptbytecode", getscriptbytecode);

  //  NewFunction(L, "getscripthash", getscripthash);

  //  NewFunction(L, "getfunctionhash", getfunctionhash);

  //  NewFunction(L, "getscripts", getscripts);

   // NewFunction(L, "getrunningscripts", getrunningscripts);
}