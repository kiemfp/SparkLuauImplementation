#include "luacode.h"
#include "luau/BytecodeBuilder.h"
#include "luau/BytecodeUtils.h"
#include "luau/Compiler.h"

#include "VM/src/lobject.h"
#include "VM/src/lstate.h"
#include "VM/src/lapi.h"

#include "lz4/include/lz4.h"
#include "zstd/include/zstd/xxhash.h"
#include "zstd/include/zstd/zstd.h"

#include "lualib.h"

#include <cstdint>
#include <lua.h>
#include <queue>
#include <psapi.h> //

#include "Compiler/include/luacode.h"



// from roblox based exploit... Idk how we would do that without relying on roblox. 


inline uintptr_t max_caps = 0xEFFFFFFFFFFFFFFF;

class bytecode_encoder : public Luau::BytecodeEncoder {
    inline void encode(uint32_t* data, size_t count) override {
        for (auto i = 0u; i < count;) {
            uint8_t op = LUAU_INSN_OP(data[i]);
            const auto opLength = Luau::getOpLength(static_cast<LuauOpcode>(op));
            const auto lookupTable = reinterpret_cast<BYTE*>(update::lua::opcode_lookup); //Roblox opcodes lookup table, I AIN'T getting that. 
            uint8_t newOp = op * 227;
            newOp = lookupTable[newOp];
            data[i] = (newOp) | (data[i] & ~0xff);
            i += opLength;
        }
    }
};

inline bytecode_encoder encoder;


inline std::string compile_script(const std::string& omegahacker) {
        static const char* mutable_globals[] = {
            "Game", "Workspace", "game", "plugin", "script", "shared", "workspace",
            "_G", "_ENV", nullptr
        };

        Luau::CompileOptions options;
        options.debugLevel = 1;
        options.optimizationLevel = 1;
        options.mutableGlobals = mutable_globals;
        options.vectorLib = "Vector3";
        options.vectorCtor = "new";
        options.vectorType = "Vector3";

        return Luau::compile(omegahacker, options, {}, &encoder);
    }

void execution::execute_script(lua_State* l, const std::string& script) {
    if (script.empty())
        return;

    int original_top = lua_gettop(l);
    lua_State* thread = lua_newthread(l);
    lua_pop(l, 1);
    luaL_sandboxthread(thread);

    auto bytecode = compile_script(script);
    if (luau_load(thread, "@RobloxV2", bytecode.c_str(), bytecode.size(), 0) != LUA_OK) {
        if (const char* err = lua_tostring(thread, -1))
            COUT<<err<<std::endl;
        lua_pop(thread, 1);
        return;
    }
/*
    if (auto closure = (Closure*)(lua_topointer(thread, -1)); closure && closure->l.p)
        context_manager::set_proto_capabilities(closure->l.p, &max_caps);
*/
    lua_getglobal(l, "task");
    lua_getfield(l, -1, "defer");
    lua_remove(l, -2);
    lua_xmove(thread, l, 1); //how do that without that? 

    if (lua_pcall(l, 1, 0, 0) != LUA_OK) {
        if (const char* err = lua_tostring(l, -1))
            COUT<< err << std::endl;
        lua_pop(l, 1);
    }

    lua_settop(thread, 0);
    lua_settop(l, original_top);
}