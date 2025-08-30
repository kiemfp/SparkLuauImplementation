#include "Misc.hpp"

inline char* luau_compileY(const char* source, size_t size, lua_CompileOptions* options, size_t* outsize, std::string* ErrMsg)
{
LUAU_ASSERT(outsize);

    Luau::CompileOptions opts;

    if (options)
    {
        static_assert(sizeof(lua_CompileOptions) == sizeof(Luau::CompileOptions), "C and C++ interface must match");
        memcpy(static_cast<void*>(&opts), options, sizeof(opts));
    }

    std::string result = Luau::compile(std::string(source, size), opts);

    if (result.empty())
    {
        if (ErrMsg) {
            *ErrMsg = "Luau compilation failed: invalid syntax or other compilation error (result was empty).";
        }
        *outsize = 0;

        if (ErrMsg) {
            std::cerr << "DEBUG (luau_compileW): Error: '" << *ErrMsg << "'" << std::endl;
        } else {
            std::cerr << "DEBUG (luau_compileW): Error Message pointer is null." << std::endl;
        }

        return nullptr;
    }

    char* copy = static_cast<char*>(malloc(result.size()));
    if (!copy) {
        if (ErrMsg) {
            *ErrMsg = "Memory allocation failed for bytecode.";
        }
        *outsize = 0;
        return nullptr;
    }

    memcpy(copy, result.data(), result.size());
    *outsize = result.size();
    return copy;
}

int loadstring(lua_State* LS) {//TODO: custom chunk support.
    luaL_checktype(LS, 1, LUA_TSTRING);

    size_t sourceLen;
    const char* Source = lua_tolstring(LS, 1, &sourceLen);
    const char* ChunkName = luaL_optstring(LS, 2, "@Spark" ); // definitely Roblox2 

    size_t bytecodeSize;
    std::string compileErrorMessage;
    char* Bytecode = luau_compileY(Source, sourceLen, NULL, &bytecodeSize, &compileErrorMessage);

    if (!Bytecode) {
        lua_pushnil(LS); // return nil
        if (!compileErrorMessage.empty()) {
            lua_pushstring(LS, compileErrorMessage.c_str());
        } else {
            lua_pushstring(LS, "Failed to compile Luau code.");
        }
        return 2; // (nil, errmsg)
    }

    int loadStatus = luau_load(LS, ChunkName, Bytecode, bytecodeSize, LUA_GLOBALSINDEX);

    free(Bytecode);

    if (loadStatus != LUA_OK) {
        // err msg should be stack top
        lua_pushboolean(LS, 0); // changed to return false
        lua_insert(LS, -2); // sets err msg to second r_arg
        return 2; // (false, err msg) 
    }

    // ret only f arg which is load' ret
    return 1;
}

void misc_library::initialize(lua_State* L)
{
    NewFunction(L, "loadstring", loadstring);
}