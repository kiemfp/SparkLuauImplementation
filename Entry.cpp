#include <iostream>
#include <string>
#include <vector>

#include <cstring>
#include <cstdio>
#include <cstdlib>

#include <Misc/Environment.hpp>
//#include <Misc/JniBridge.hpp> //I've been trying for a whole week, won't try again in a while. 

#include <Luau/Compiler.h>
#include <Luau/BytecodeBuilder.h>
#include <Luau/BytecodeUtils.h>
#include <Luau/Bytecode.h>
#include <luacode.h>

#include <lapi.h>
#include <lstate.h>
#include <lualib.h>
#include <ldebug.h>
#include <ltable.h>
#include "ldebug.h"
//#include "lapi.h"
#include "lfunc.h"
#include "lmem.h"
#include "lgc.h"
#include "ldo.h"
#include "lbytecode.h"
#include "lua.h"


inline char* luau_compileG(const char* source, size_t size, lua_CompileOptions* options, size_t* outsize, std::string* ErrMsg)
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


static void replaceString(std::string& source, const std::string_view toReplace, const std::string_view replacement) {
    size_t pos = source.find(toReplace);

    if (pos != std::string::npos) {
        source.replace(pos, toReplace.length(), replacement);
    }
}

void* luau_alloc(void* ud, void* ptr, size_t osize, size_t nsize) {
    (void)ud;
    (void)osize;

    if (nsize == 0) {
        free(ptr);
        return NULL;
    } else {
        return realloc(ptr, nsize);
    }
}

int main() {
    lua_State* LS = lua_newstate(luau_alloc, NULL);

    if (!LS) {
        std::cerr << "Failed to create Luau state." << std::endl;
        return 1;
    }
    lua_State* L = lua_newthread(LS); //init script thread. 
    luaL_sandboxthread(L); //forgot what this does, but it sandbox the thread inherited by main state. 
    luaL_openlibs(LS); //open debug, math, os and etc libs. 

    environment::initialize(LS);// Our Funcs.

    size_t bytecodeSize; //
    std::string CompileErrorMessage; //Auto Explanatory.

    // here: read input
    std::string luauCode;
    std::string line;
    while (std::getline(std::cin, line)) { // "./Spark.out < InitScript.luau" gets the provided code. 
        luauCode += line + "\n"; // every line to newline, wow pro. 
    }

    if (luauCode.empty()) {
        std::cerr << "No Luau code provided via std input. Exiting." << std::endl;
        lua_close(L);
        return 1;
    }

    std::cout << "\n----COMPILING LUAU CODE FROM STDIN----" << std::endl;
    //std::cerr << "DEBUG: Luau code received (first 10 chars): " << luauCode.substr(0, std::min((size_t)10, luauCode.length())) << (luauCode.length() > 100 ? "..." : "") << std::endl;

    //luau_compileW is from Env. 
    char* bytecode = luau_compileG(luauCode.c_str(), luauCode.length(), NULL, &bytecodeSize, &CompileErrorMessage);

    if (!bytecode) {
        std::cerr << "Luau compilation of InitScript script failed." << std::endl;
        if (!CompileErrorMessage.empty()) {
            std::cerr << "Compiler error: " << CompileErrorMessage << std::endl;
        } else {
            // This block can be redundant if luau_compileW always set ErrMsg
            // but it's a really good fallback.
            if (lua_type(L, -1) == LUA_TSTRING) {
                std::cerr << "Error from Lua stack (possibly load/pcall): " << lua_tostring(L, -1) << std::endl;
                lua_pop(L, 1);
            } else {
                 std::cerr << "Unknown compilation error for main script (no specific message tho)." << std::endl;
            }
        }
        lua_close(L);
        return 1;
    }
    std::cout << "\n----Compiled Successfully! ----" << std::endl;
    std::cout << "\n----EXECUTING LUAU CODE----" << std::endl;
    int loadStatus = luau_load(L, "@InitScript", bytecode, bytecodeSize, 0);

    free(bytecode);

    if (loadStatus != LUA_OK) {
        std::cerr << "Error loading Luau code: " << lua_tostring(L, -1) << std::endl;
        lua_close(L);
        return 1;
    }

    int pcallStatus = lua_pcall(L, 0, LUA_MULTRET, 0); // :(

    if (pcallStatus != LUA_OK) {
        std::cerr << "Error executing Luau code: " << lua_tostring(L, -1) << std::endl;
    }

    lua_close(L);
    std::cout << "Luau state closed." << std::endl;

    return 0;
}
