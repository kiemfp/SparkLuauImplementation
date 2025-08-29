#pragma once
#include "../../Includes.hpp"
#include "lapi.h"
struct lua_State;
class debug_library
{
public:
	static void initialize(lua_State* L);
};