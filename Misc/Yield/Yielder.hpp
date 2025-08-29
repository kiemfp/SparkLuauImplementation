#pragma once
#include "../Environment.hpp"
#include <thread>
#include "functional"
#include "lua.h"
class yielder
{
public:
	using yield_return = std::function<int(lua_State* L)>;

	static int yield_execution(lua_State* L, const std::function<yield_return()>& generator);
};