#pragma once
#include "../../Includes.hpp"

class closure_library
{
public:
	//static int loadstring(lua_State* L);
	static void initialize(lua_State* L);
};