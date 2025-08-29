#pragma once
#include "iostream"

#include "Includes.hpp" 

#include "Env/Debug/Debug.hpp"
#include "Env/Metatable/Metatable.hpp"
//#include "Env/Closure/Closure.hpp"
#include "Env/Script/Script.hpp"
#include <lua.h>
class environment
{
public:
	static void initialize(lua_State* l);
};
