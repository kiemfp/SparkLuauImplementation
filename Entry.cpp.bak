#include <iostream>
#include <string>
#include <vector>

#include <cstring>
#include <cstdio>
#include <cstdlib>

#include <Misc/Environment.hpp>

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
    luaL_sandboxthread(L);
    luaL_openlibs(LS);

    Env::registerFunctions(LS);//env

    size_t bytecodeSize;
    std::string mainCompileErrorMessage;

    // here: read input
    std::string luauCode;
    std::string line;
    while (std::getline(std::cin, line)) {
        luauCode += line + "\n"; // every line to newline, wow pro. 
    }

    if (luauCode.empty()) {
        std::cerr << "No Luau code provided via std input. Exiting." << std::endl;
        lua_close(L);
        return 1;
    }

    std::cout << "\n----COMPILING LUAU CODE FROM STDIN----" << std::endl;
    std::cerr << "DEBUG: Luau code received (first 10 chars): " << luauCode.substr(0, std::min((size_t)10, luauCode.length())) << (luauCode.length() > 100 ? "..." : "") << std::endl;


    char* bytecode = luau_compileW(luauCode.c_str(), luauCode.length(), NULL, &bytecodeSize, &mainCompileErrorMessage);

    if (!bytecode) {
        std::cerr << "Luau compilation of InitScript script failed." << std::endl;
        if (!mainCompileErrorMessage.empty()) {
            std::cerr << "Compiler error: " << mainCompileErrorMessage << std::endl;
        } else {
            // Este bloco pode ser redundante se luau_compileW sempre definir ErrMsg
            // mas é um bom fallback.
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

    std::cout << "\n----EXECUTING LUAU CODE----" << std::endl;
    int loadStatus = luau_load(L, "@InitScript", bytecode, bytecodeSize, 0);

    free(bytecode);

    if (loadStatus != LUA_OK) {
        std::cerr << "Error loading Luau code: " << lua_tostring(L, -1) << std::endl;
        lua_close(L);
        return 1;
    }

    int pcallStatus = lua_pcall(L, 0, LUA_MULTRET, 0);

    if (pcallStatus != LUA_OK) {
        std::cerr << "Error executing Luau code: " << lua_tostring(L, -1) << std::endl;
    }

    lua_close(L);
    std::cout << "Luau state closed." << std::endl;

    return 0;
}
