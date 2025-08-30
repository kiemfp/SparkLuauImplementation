#pragma once
#include "../../Includes.hpp"
struct lua_State;
class metatable_library
{
public:
	static void initialize(lua_State* L);
};