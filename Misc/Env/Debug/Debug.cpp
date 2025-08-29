#include "Debug.hpp"

#include "lmem.h"
#include <lmem.h>


static LuaTable* getcurrenvG(lua_State* L)
{
    if (L->ci == L->base_ci) // no enclosing function?
        return L->gt;        // use global table as environment
    else
        return curr_func(L)->env;
}

static LUAU_NOINLINE TValue* pseudo2addrF(lua_State* L, int idx)
{
    api_check(L, lua_ispseudo(idx));
    switch (idx)
    { // pseudo-indices
    case LUA_REGISTRYINDEX:
        return registry(L);
    case LUA_ENVIRONINDEX:
    {
        sethvalue(L, &L->global->pseudotemp, getcurrenvG(L));
        return &L->global->pseudotemp;
    }
    case LUA_GLOBALSINDEX:
    {
        sethvalue(L, &L->global->pseudotemp, L->gt);
        return &L->global->pseudotemp;
    }
    default:
    {
        Closure* func = curr_func(L);
        idx = LUA_GLOBALSINDEX - idx;
        return (idx <= func->nupvalues) ? &func->c.upvals[idx - 1] : cast_to(TValue*, luaO_nilobject);
    }
    }
}

LUAU_FORCEINLINE TValue* index2addrW(lua_State* L, int idx)
{
    if (idx > 0)
    {
        TValue* o = L->base + (idx - 1);
        api_check(L, idx <= L->ci->top - L->base);
        if (o >= L->top)
            return cast_to(TValue*, luaO_nilobject);
        else
            return o;
    }
    else if (idx > LUA_REGISTRYINDEX)
    {
        api_check(L, idx != 0 && -idx <= L->top - L->base);
        return L->top + idx;
    }
    else
    {
        return pseudo2addrF(L, idx);
    }
}




inline char* luau_compileF(const char* source, size_t size, lua_CompileOptions* options, size_t* outsize, std::string* ErrMsg)
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





struct lua_Page;
union GCObject;

struct lua_Page
{
    // list of pages with free blocks
    lua_Page* prev;
    lua_Page* next;

    // list of all pages
    lua_Page* listprev;
    lua_Page* listnext;

    int pageSize;  // page size in bytes, including page header
    int blockSize; // block size in bytes, including block header (for non-GCO)

    void* freeList; // next free block in this page; linked with metadata()/freegcolink()
    int freeNext;   // next free block offset in this page, in bytes; when negative, freeList is used instead
    int busyBlocks; // number of blocks allocated out of this page

    // provide additional padding based on current object size to provide 16 byte alignment of data
    // later static_assert checks that this requirement is held
    char padding[sizeof(void*) == 8 ? 8 : 12];

    char data[1];
};


namespace Debug {
    Closure* header_get_function(lua_State* L, bool allowCclosure = false, bool popcl = true)
    {
        luaL_checkany(L, 1);

        if (!(lua_isfunction(L, 1) || lua_isnumber(L, 1)))
        {
            luaL_argerror(L, 1, "function or number");
        }

        int level = 0;
        if (lua_isnumber(L, 1))
        {
            level = lua_tointeger(L, 1);

            if (level <= 0)
            {
                luaL_argerror(L, 1, "level out of range");
            }
        }
        else if (lua_isfunction(L, 1))
        {
            level = -lua_gettop(L);
        }

        lua_Debug ar;
        if (!lua_getinfo(L, level, "f", &ar))
        {
            luaL_argerror(L, 1, "invalid level");
        }

        if (!lua_isfunction(L, -1))
            luaL_argerror(L, 1, "level does not point to a function");
        if (lua_iscfunction(L, -1) && !allowCclosure)
            luaL_argerror(L, 1, "level points to c function");

        if (!allowCclosure && lua_iscfunction(L, -1))
        {
            luaL_argerror(L, 1, "luau function expected");
        }

        auto function = clvalue(luaA_toobject(L, -1));
        if (popcl) lua_pop(L, 1);

        return function;
    }

    inline const char* aux_upvalue_2(Closure* f, int n, TValue** val)
    {
        if (f->isC)
        {
            if (!(1 <= n && n <= f->nupvalues))
                return NULL;
            *val = &f->c.upvals[n - 1];
            return "";
        }
        else
        {
            Proto* p = f->l.p;
            if (!(1 <= n && n <= p->nups)) // not a valid upvalue
                return NULL;
            TValue* r = &f->l.uprefs[n - 1];
            *val = ttisupval(r) ? upvalue(r)->v : r;
            if (!(1 <= n && n <= p->sizeupvalues)) // don't have a name for this upvalue
                return "";
            return getstr(p->upvalues[n - 1]);
        }
    }

    int debug_getupvalues(lua_State* L)
    {
        luaL_checkany(L, 1);

        Closure* function = header_get_function(L, true, false);
        lua_newtable(L);


        for (int i = 0; i < function->nupvalues; i++) {
            TValue* upval;

            const char* upvalue_name = aux_upvalue_2(function, i + 1, &upval);

            if (upvalue_name) {
                if (iscollectable(upval))
                    luaC_threadbarrier(L);

                lua_rawcheckstack(L, 1);

                L->top->value = upval->value;
                L->top->tt = upval->tt;
                L->top++;

                lua_rawseti(L, -2, i + 1);
            }

        }

        return 1;
    }

    int debug_getupvalue(lua_State* L)
    {
        const auto Func = header_get_function(L, true, false);
        const auto idx = lua_tointeger(L, 2);

        int level = -lua_gettop(L);;

        if (Func->nupvalues <= 0)
        {
            luaL_argerror(L, 1, "function has no upvalues");
        }


        lua_Debug ar;
        if (!lua_getinfo(L, level, "f", &ar))
            luaL_error(L, "invalid level");

        const char* upvalue = lua_getupvalue(L, -1, idx);
        if (!upvalue) {
            lua_pushnil(L);
            return 1;
        }

        return 1;
    }

    int debug_setupvalue(lua_State* L)
    {
        const auto Func = header_get_function(L, false, false);
        const auto idx = lua_tointeger(L, 2);

        if (Func->nupvalues <= 0)
        {
            luaL_argerror(L, 1, "function has no upvalues");
        }

        if (!(idx >= 1 && idx <= Func->nupvalues))
        {
            luaL_argerror(L, 2, "index out of range");
        }

        lua_pushvalue(L, 3);
        lua_setupvalue(L, -2, idx);
        return 1;
    }

    int debug_getconstants(lua_State* L)
    {
        const auto Func = header_get_function(L);
        const auto p = (Proto*)Func->l.p;

        lua_createtable(L, p->sizek, 0);

        for (int i = 0; i < p->sizek; i++)
        {
            TValue k = p->k[i];

            if (k.tt == LUA_TNIL || k.tt == LUA_TFUNCTION || k.tt == LUA_TTABLE)
            {
                lua_pushnil(L);
            }
            else
            {
                luaC_threadbarrier(L)
                    luaA_pushobject(L, &k);
            }
            lua_rawseti(L, -2, i + 1);
        }

        return 1;
    }

    int debug_getconstant(lua_State* L)
    {
        const auto Func = header_get_function(L);
        const auto idx = luaL_checkinteger(L, 2);
        const auto p = (Proto*)Func->l.p;

        const auto level = -lua_gettop(L);

        if (p->sizek <= 0)
        {
            luaL_argerror(L, 1, "function has no constants");
        }

        lua_Debug ar;
        if (!lua_getinfo(L, level, "f", &ar))
            luaL_error(L, "invalid level");

        if (!(idx >= 1 && idx <= p->sizek))
        {
            luaL_argerror(L, 2, "index out of range");
        }


        const auto k = &p->k[idx - 1];

        if (k->tt == LUA_TNIL || k->tt == LUA_TTABLE || k->tt == LUA_TFUNCTION) {
            lua_pushnil(L);
            return 1;
        }

        luaA_pushobject(L, k);
        return 1;
    }

    int debug_setconstant(lua_State* L)
    {
        luaL_checkany(L, 3);

        const auto Func = header_get_function(L);
        const auto idx = luaL_checkinteger(L, 2);
        const auto p = (Proto*)Func->l.p;

        if (p->sizek <= 0)
        {
            luaL_argerror(L, 1, "function has no constants");
        }

        if (!(idx >= 1 && idx <= p->sizek))
        {
            luaL_argerror(L, 2, "index out of range");
        }

        TValue* k = &p->k[idx - 1];

        if (k->tt == LUA_TFUNCTION || k->tt == LUA_TTABLE)
        {
            return 0;
        }
        else
        {
            if (k->tt == luaA_toobject(L, 3)->tt)
            {
                setobj2s(L, k, luaA_toobject(L, 3));
            }
        }

        return 0;
    }

    int debug_getprotos(lua_State* L)
    {
        Closure* Func = header_get_function(L);
        bool active = !lua_isnoneornil(L, 2) ? (lua_toboolean(L, 2) != 0) : false;

        Proto* p = (Proto*)Func->l.p;
        lua_createtable(L, p->sizep, 0);
        if (!active)
        {
            // Return non-callable handles for inactive protos (lightuserdata)
            for (int i = 0; i < p->sizep; i++)
            {
                Proto* proto = p->p[i];
                lua_pushlightuserdata(L, (void*)proto);
                lua_rawseti(L, -2, i + 1);
            }
        }
        else
        {
            // Return active closures for each child proto
            for (int i = 0; i < p->sizep; i++)
            {
                Proto* proto = p->p[i];
                Closure* pcl = luaF_newLclosure(L, Func->nupvalues, Func->env, proto);
                luaC_threadbarrier(L); setclvalue(L, L->top, pcl); L->top++;
                lua_rawseti(L, -2, i + 1);
            }
        }
        return 1;
    }

    Proto* clone_proto(lua_State* L, Proto* proto) /* sUnc is pmo, wants the proto to not be callable */
    {
        Proto* clone = luaF_newproto(L);

        clone->sizek = proto->sizek;
        clone->k = luaM_newarray(L, proto->sizek, TValue, proto->memcat);
        for (int i = 0; i < proto->sizek; ++i)
            setobj2n(L, &clone->k[i], &proto->k[i]);

        clone->lineinfo = clone->lineinfo;
        clone->locvars = luaM_newarray(L, proto->sizelocvars, LocVar, proto->memcat);
        for (int i = 0; i < proto->sizelocvars; ++i)
        {
            const auto varname = getstr(proto->locvars[i].varname);
            const auto varname_size = strlen(varname);

            clone->locvars[i].varname = luaS_newlstr(L, varname, varname_size);
            clone->locvars[i].endpc = proto->locvars[i].endpc;
            clone->locvars[i].reg = proto->locvars[i].reg;
            clone->locvars[i].startpc = proto->locvars[i].startpc;
        }

        clone->nups = proto->nups;
        clone->sizeupvalues = proto->sizeupvalues;
        clone->sizelineinfo = proto->sizelineinfo;
        clone->linegaplog2 = proto->linegaplog2;
        clone->sizelocvars = proto->sizelocvars;
        clone->linedefined = proto->linedefined;

        if (proto->debugname)
        {
            const auto debugname = getstr(proto->debugname);
            const auto debugname_size = strlen(debugname);

            clone->debugname = luaS_newlstr(L, debugname, debugname_size);
        }

        if (proto->source)
        {
            const auto source = getstr(proto->source);
            const auto source_size = strlen(source);

            clone->source = luaS_newlstr(L, source, source_size);
        }

        clone->numparams = proto->numparams;
        clone->is_vararg = proto->is_vararg;
        clone->maxstacksize = proto->maxstacksize;
        clone->bytecodeid = proto->bytecodeid;


        size_t bytecodeSize;
        std::string CompileErrorMessage;


        auto bytecode = luau_compileF("return", strlen("return"), NULL, &bytecodeSize, &CompileErrorMessage);
        luau_load(L, "@cloneproto", bytecode, bytecodeSize, 0);

        Closure* cl = clvalue(index2addrW(L, -1));

        clone->sizecode = cl->l.p->sizecode;
        clone->code = luaM_newarray(L, clone->sizecode, Instruction, proto->memcat);
        for (size_t i = 0; i < cl->l.p->sizecode; i++) {
            clone->code[i] = cl->l.p->code[i];
        }
        lua_pop(L, 1);
        clone->codeentry = clone->code;
        clone->debuginsn = 0;

        clone->sizep = proto->sizep;
        clone->p = luaM_newarray(L, proto->sizep, Proto*, proto->memcat);
        for (int i = 0; i < proto->sizep; ++i)
        {
            clone->p[i] = clone_proto(L, proto->p[i]);
        }

        return clone;
    }

    int debug_getproto(lua_State* L)
    {
        luaL_checktype(L, 2, LUA_TNUMBER);

        // ReSharper disable once CppLocalVariableMayBeConst
        Closure* closure = nullptr;
        int index = lua_tointeger(L, 2);
        bool active = luaL_optboolean(L, 3, false);

        if (lua_isfunction(L, 1))
        {
            closure = clvalue(luaA_toobject(L, 1));
        }
        else if (lua_isnumber(L, 1))
        {
            lua_Debug dbg_info;

            int level = lua_tointeger(L, 1);
            int callstack_size = static_cast<int>(L->ci - L->base_ci);

            if (level <= 0 || callstack_size <= level)
                luaL_argerrorL(L, 1, ("level out of bounds"));
            if (!lua_getinfo(L, level, ("f"), &dbg_info))
                luaL_argerrorL(L, 1, ("level out of bounds"));

            if (!lua_isfunction(L, -1))
                luaL_argerrorL(L, 1, ("level does not point to a function"));
            if (lua_iscfunction(L, -1))
                luaL_argerrorL(L, 1, ("lua function expected"));;

            closure = clvalue(luaA_toobject(L, -1));
            lua_pop(L, 1);
        }


        // ReSharper disable once CppDFANullDereference
        Proto* p = closure->l.p;

        // ReSharper disable once CppDFANullDereference
        if (index <= 0 || index > p->sizep)
            luaL_argerrorL(L, 2, ("index out of bounds"));

        Proto* wanted_proto = p->p[index - 1];

        if (!active)
        {
            Proto* cloned_proto = clone_proto(L, wanted_proto);
            Closure* new_closure = luaF_newLclosure(L, closure->nupvalues, L->gt, cloned_proto);

            luaC_checkGC(L);
            luaC_threadbarrier(L);

            L->top->value.gc = reinterpret_cast<GCObject*>(new_closure);
            L->top->tt = LUA_TFUNCTION;
            luaD_checkstack(L, 1);
            L->top++;
        }
        else
        {
            const global_State* g = L->global;
            int object_count = 0;

            lua_newtable(L);

            lua_gc(L, LUA_GCSTOP, 0);
            for (lua_Page* current_gco_page = g->allgcopages; current_gco_page; )
            {
                lua_Page* next = current_gco_page->listnext; // block visit might destroy the page

                char* start;
                char* end;
                int busy_blocks;
                int block_size;

                luaM_getpagewalkinfo(current_gco_page, &start, &end, &busy_blocks, &block_size);

                for (char* pos = start; pos != end; pos += block_size)
                {
                    GCObject* gc_object = reinterpret_cast<GCObject*>(pos);

                    if (gc_object->gch.tt != LUA_TFUNCTION)
                        continue;

                    if (isdead(g, gc_object))
                        continue;

                    const Closure* gc_closure = reinterpret_cast<Closure*>(pos);

                    if (gc_closure->l.p != wanted_proto)
                        continue;

                    luaC_checkGC(L);
                    luaC_threadbarrier(L);

                    L->top->value.gc = gc_object;
                    L->top->tt = gc_object->gch.tt;
                    luaD_checkstack(L, 1);
                    L->top++;

                    lua_rawseti(L, -2, ++object_count);
                }

                current_gco_page = next;
            }
            lua_gc(L, LUA_GCRESTART, 0);
        }

        return 1;
    }

    int debug_getstack(lua_State* L)
    {
        luaL_checkany(L, 1);

        if (!(lua_isfunction(L, 1) || lua_isnumber(L, 1)))
        {
            luaL_argerror(L, 1, "function or number");
        }

        int level = 0;
        if (lua_isnumber(L, 1))
        {
            level = lua_tointeger(L, 1);

            if (level <= 0)
            {
                luaL_argerror(L, 1, "level out of range");
            }
        }
        else if (lua_isfunction(L, 1))
        {
            level = -lua_gettop(L);
        }

        lua_Debug ar;
        if (!lua_getinfo(L, level, "f", &ar))
        {
            luaL_argerror(L, 1, "invalid level");
        }

        if (!lua_isfunction(L, -1))
        {
            luaL_argerror(L, 1, "level does not point to a function");
        }

        if (lua_iscfunction(L, -1))
        {
            luaL_argerror(L, 1, "luau function expected");
        }

        lua_pop(L, 1);

        auto ci = L->ci[-level];

        if (lua_isnumber(L, 2))
        {
            const auto idx = lua_tointeger(L, 2) - 1;

            if (idx >= cast_int(ci.top - ci.base) || idx < 0)
            {
                luaL_argerror(L, 2, "index out of range");
            }

            auto val = ci.base + idx;
            luaC_threadbarrier(L) luaA_pushobject(L, val);
        }
        else
        {
            int idx = 0;
            lua_newtable(L);

            for (auto val = ci.base; val < ci.top; val++)
            {
                lua_pushinteger(L, idx++ + 1);

                luaC_threadbarrier(L) luaA_pushobject(L, val);

                lua_settable(L, -3);
            }
        }

        return 1;
    }

    int debug_setstack(lua_State* L)
    {
        luaL_checkany(L, 1);

        if (!(lua_isfunction(L, 1) || lua_isnumber(L, 1)))
        {
            luaL_argerror(L, 1, "function or number");
        }

        int level = 0;
        if (lua_isnumber(L, 1))
        {
            level = lua_tointeger(L, 1);

            if (level <= 0)
            {
                luaL_argerror(L, 1, "level out of range");
            }
        }
        else if (lua_isfunction(L, 1))
        {
            level = -lua_gettop(L);
        }

        lua_Debug ar;
        if (!lua_getinfo(L, level, "f", &ar))
        {
            luaL_argerror(L, 1, "invalid level");
        }

        if (!lua_isfunction(L, -1))
        {
            luaL_argerror(L, 1, "level does not point to a function");
        }

        if (lua_iscfunction(L, -1))
        {
            luaL_argerror(L, 1, "luau function expected");
        }

        lua_pop(L, 1);

        luaL_checkany(L, 3);

        auto ci = L->ci[-level];

        const auto idx = luaL_checkinteger(L, 2) - 1;
        if (idx >= cast_int(ci.top - ci.base) || idx < 0)
        {
            luaL_argerror(L, 2, "index out of range");
        }

        if ((ci.base + idx)->tt != luaA_toobject(L, 3)->tt)
        {
            luaL_argerror(L, 3, "new value type does not match previous value type");
        }

        setobj2s(L, (ci.base + idx), luaA_toobject(L, 3))
            return 0;
    }

    int debug_getinfo(lua_State* L)
    {

        luaL_checkany(L, 1);

        if (!(lua_isfunction(L, 1) || lua_isnumber(L, 1)))
        {
            luaL_argerror(L, 1, "function or number");
        }

        int level;
        if (lua_isnumber(L, 1))
        {
            level = lua_tointeger(L, 1);
        }
        else
        {
            level = -lua_gettop(L);
        }

        auto desc = "sluanf";

        lua_Debug ar;
        if (!lua_getinfo(L, level, desc, &ar))
        {
            luaL_argerror(L, 1, "invalid level");
        }

        if (!lua_isfunction(L, -1))
        {
            luaL_argerror(L, 1, "level does not point to a function.");
        }

        lua_newtable(L);
        {
            if (std::strchr(desc, 's'))
            {
                lua_pushstring(L, ar.source);
                lua_setfield(L, -2, "source");

                lua_pushstring(L, ar.short_src);
                lua_setfield(L, -2, "short_src");

                lua_pushstring(L, ar.what);
                lua_setfield(L, -2, "what");

                lua_pushinteger(L, ar.linedefined);
                lua_setfield(L, -2, "linedefined");
            }

            if (std::strchr(desc, 'l'))
            {
                lua_pushinteger(L, ar.currentline);
                lua_setfield(L, -2, "currentline");
            }

            if (std::strchr(desc, 'u'))
            {
                lua_pushinteger(L, ar.nupvals);
                lua_setfield(L, -2, "nups");
            }

            if (std::strchr(desc, 'a'))
            {
                lua_pushinteger(L, ar.isvararg);
                lua_setfield(L, -2, "is_vararg");

                lua_pushinteger(L, ar.nparams);
                lua_setfield(L, -2, "numparams");
            }

            if (std::strchr(desc, 'n'))
            {
                lua_pushstring(L, ar.name);
                lua_setfield(L, -2, "name");
            }

            if (std::strchr(desc, 'f'))
            {
                lua_pushvalue(L, -2);
                lua_remove(L, -3);
                lua_setfield(L, -2, "func");
            }
        }

        return 1;
    }
};

void debug_library::initialize(lua_State* L)
{
    lua_getglobal(L, "debug");
    if (!lua_istable(L, -1))
    {
        lua_pop(L, 1);
        lua_newtable(L);
    }

    lua_setreadonly(L, -1, false);
    NewTableFunction(L, "getproto", Debug::debug_getproto);
    NewTableFunction(L, "getprotos", Debug::debug_getprotos);
    NewTableFunction(L, "getstack", Debug::debug_getstack);
    NewTableFunction(L, "getinfo", Debug::debug_getinfo);
    NewTableFunction(L, "getupvalue", Debug::debug_getupvalue);
    NewTableFunction(L, "getupvalues", Debug::debug_getupvalues);
    NewTableFunction(L, "getconstant", Debug::debug_getconstant);
    NewTableFunction(L, "getconstants", Debug::debug_getconstants);
    NewTableFunction(L, "setconstant", Debug::debug_setconstant);
    NewTableFunction(L, "setstack", Debug::debug_setstack);
    NewTableFunction(L, "setupvalue", Debug::debug_setupvalue);

    lua_setreadonly(L, -1, true);
    lua_setglobal(L, "debug");
}