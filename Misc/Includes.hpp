#pragma once 
#include "iostream"
#include "vector"
#include <string>
#include <vector>
#include <algorithm>
#include <map>      
#include <cstdlib>   
#include <stdexcept> 
#include <ctime>    
#include <cctype>    
#include <random>    
#include <sstream>
#include <unordered_set>

#include "Yield/Yielder.hpp"
//#include "Hook/Hook.hpp" //exploit anti-exploit shit :skull:

#include <cstring>
#include <cstdio>
#include <cstdlib>

//#include <Misc/Environment.hpp>//yeah copied from entry

#include <Luau/Compiler.h>
#include <Luau/BytecodeBuilder.h>
#include <Luau/BytecodeUtils.h>
#include <Luau/Bytecode.h>
#include <luacode.h>

#include <lapi.h>
#include <lstate.h>
#include <lualib.h>
#include <lualib.h>
#include <luaconf.h>
#include <ldebug.h>
#include <ltable.h>
#include "ldebug.h"
#include "lapi.h"
#include "lfunc.h"
#include "lmem.h"
#include <lmem.h>
#include "lgc.h"
#include "ldo.h"
#include "lbytecode.h"
#include "lua.h"

#include "lstate.h"
#include "lstring.h"
#include "ltable.h"
#include "lfunc.h"
#include "ludata.h"
#include "lvm.h"
#include "lnumutils.h"
#include "lbuffer.h"

#define lua_pushrawclosure(L, rawClosure) \
setclvalue(L, L->top, rawClosure); \
incr_top(L)

#define luaL_trimstack(L, n) if (lua_gettop(L) > n) lua_settop(L, n)
#define lua_normalisestack(L, mxs) { if (lua_gettop(L) > mxs) lua_settop(L, mxs); }
#define lua_toclosure(L, i) (Closure*)lua_topointer(L, i)



static void NewTableFunction(lua_State* L, const char* globalname, lua_CFunction function) {
    lua_pushcclosurek(L, function, nullptr, 0, nullptr);
    lua_setfield(L, -2, globalname);
    std::cout << globalname<<std::endl;
}

static void NewFunction(lua_State* L, const char* globalname, lua_CFunction function)
{
    lua_pushcclosurek(L, function, nullptr, 0, nullptr);
    lua_setglobal(L, globalname);
    std::cout<<globalname<<std::endl;
}

//#define luau_compileW luau_compile
inline char* luau_compileW(const char* source, size_t size, lua_CompileOptions* options, size_t* outsize, std::string* ErrMsg);