//#include "Yielder.hpp"


#include "Yielder.hpp"
#include <coroutine>
#include <functional>
#include <stdexcept>

// The promise type for the coroutine
struct LuaYieldPromise
{
    // A function pointer to the coroutine's final result.
    int result_count = 0;
    lua_State* state;

    // The coroutine's return type.
    struct CoroutineHandle
    {
        std::coroutine_handle<LuaYieldPromise> handle;
    };

    auto get_return_object()
    {
        return CoroutineHandle{std::coroutine_handle<LuaYieldPromise>::from_promise(*this)};
    }

    // This is where the magic happens. The coroutine immediately suspends on creation.
    std::suspend_always initial_suspend()
    {
        return {};
    }

    // The final result. The coroutine will not be resumed after this point.
    std::suspend_always final_suspend() noexcept
    {
        return {};
    }

    // Sets the final return value of the coroutine.
    void return_value(int value)
    {
        result_count = value;
    }

    // Required by the C++ coroutine standard.
    void unhandled_exception() {}
};

// Add this specialization to link your CoroutineHandle to the promise type.
template <typename... Args>
struct std::coroutine_traits<LuaYieldPromise::CoroutineHandle, Args...>
{
    using promise_type = LuaYieldPromise;
};

// The coroutine's body, which replaces the 'thread_worker' function.
LuaYieldPromise::CoroutineHandle
thread_worker(lua_State* L, const std::function<yielder::yield_return()>& generator)
{
    // Call the generator and return its result, which becomes the coroutine's return value.
    co_return generator()(L);
}

int yielder::yield_execution(lua_State* L, const std::function<yield_return()>& generator)
{
    // Create the coroutine and get its handle.
    auto coro_handle = thread_worker(L, generator);

    // The coroutine is suspended at its initial point.
    // We can now pass control back to Lua.
    L->base = L->top;
    L->status = LUA_YIELD;
    L->ci->flags |= 1;
    return -1;
}


/*
struct task_data
{
	lua_State* state;
	std::function<yielder::yield_return()> generator;
	PTP_WORK work;
};

void thread_worker(task_data* data)
{
	try
	{
		auto yield_result = data->generator();
		int result_count = yield_result(data->state);

		lua_State* thread_ctx = lua_newthread(data->state);

		lua_getglobal(thread_ctx, "task");
		lua_getfield(thread_ctx, -1, "defer");
		lua_pushthread(data->state);
		lua_xmove(data->state, thread_ctx, 1);
		lua_pop(data->state, 1);

		for (int i = result_count; i >= 1; --i)
		{
			lua_pushvalue(data->state, -i);
			lua_xmove(data->state, thread_ctx, 1);
		}

		lua_pcall(thread_ctx, result_count + 1, 0, 0);
		lua_settop(thread_ctx, 0);
	}
	catch (...)
	{
	}

	delete data;
}

int yielder::yield_execution(lua_State* L, const std::function<yield_return()>& generator)
{
	lua_pushthread(L);
	lua_ref(L, -1);
	lua_pop(L, 1);

	auto* task = new task_data{ L, generator, nullptr };

	std::thread(thread_worker, task).detach();

	L->base = L->top;
	L->status = LUA_YIELD;
	L->ci->flags |= 1;
	return -1;
}*/