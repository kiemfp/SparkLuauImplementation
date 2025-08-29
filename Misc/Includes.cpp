#include "Includes.hpp"


inline char* luau_compileW(const char* source, size_t size, lua_CompileOptions* options, size_t* outsize, std::string* ErrMsg)
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



