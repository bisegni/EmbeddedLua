//
//  main.cpp
//  EmbedLua
//
//  Created by Bisegni Claudio on 26/02/12.
//  Copyright (c) 2012 INFN. All rights reserved.
//

#include <iostream>
#include <lua.hpp>
#include "luna.h"
#include "LuaExposedObject.h"

bool bAbortScript = false;

class AutoFoo : public LuaObjectFactoryRegister<AutoFoo>{
public:
    AutoFoo(lua_State *L) {
        printf("in constructor AutoFoo\n");
    }
    
    int foo(lua_State *state) {
        int argc = lua_gettop( state );
        std::cout << "[AutoFoo C-FUNCTION: Got " << argc << " arguments from caller" << std::endl;
        
        if( argc != 3 )
        {
            std::cout << "[AutoFoo C-FUNCTION]: We weren't called with two arguments ..." << std::endl;
            lua_pushnumber( state, 0 );
            return 1;
        }
        
        // Get argument 1 and 2 as int's
        int a = lua_tointeger( state, 2 );
        int b = lua_tointeger( state, 3 );
        
        // Calculate result and push it on the stack
        lua_pushnumber( state, a * b ); // Return argument 1 * argument 2
        return 1; // Number of return values
    }
    
    ~AutoFoo() {
        printf("in destructor AutoFoo\n");
    }
};

class Foo {
public:
    Foo(lua_State *L) {
        printf("in constructor\n");
    }
    
    int foo(lua_State *state) {
        int argc = lua_gettop( state );
        std::cout << "[C-FUNCTION: Got " << argc << " arguments from caller" << std::endl;
        
        if( argc != 3 )
        {
            std::cout << "[C-FUNCTION]: We weren't called with two arguments ..." << std::endl;
            lua_pushnumber( state, 0 );
            return 1;
        }
        
        // Get argument 1 and 2 as int's
        int a = lua_tointeger( state, 2 );
        int b = lua_tointeger( state, 3 );
        
        // Calculate result and push it on the stack
        lua_pushnumber( state, a * b ); // Return argument 1 * argument 2
        return 1; // Number of return values
    }
    
    ~Foo() {
        printf("in destructor\n");
    }
    
    static const char className[];
    static const Luna<Foo>::RegType Register[];
};

const char Foo::className[] = "Foo";
const Luna<Foo>::RegType Foo::Register[] = {
    { "foo", &Foo::foo },
    { 0 }
};

void lua_stacktrace(lua_State* L)
{
    lua_Debug entry;
    int depth = 0; 
    
    while (lua_getstack(L, depth, &entry))
        {
        //int status = lua_getinfo(L, "Sln", &entry);
        if(entry.short_src)std::cout << entry.short_src << std::endl;
        }
}
                
static void LuaHookCall (lua_State *lua)
{
    
    printf ("---- Call Stack ----\n");
        //   printf ("[Level] [Function] [# args] [@line] [src]\n");
    
    lua_Debug ar;
        // Look at call stack
    for (int iLevel = 0; lua_getstack (lua, iLevel, &ar) != 0; ++iLevel)
        {
        if (lua_getinfo (lua, "Snlu", &ar) != 0)
            {
            printf ("%d %s %s %d @%d %s\n", iLevel, ar.namewhat, ar.name, ar.nups, ar.currentline, ar.source);
            }
        }
}

static void LuaHookRet (lua_State *lua)
{
    
}

static void LuaHookLine (lua_State *lua)
{
    lua_Debug ar;
    lua_getstack (lua, 0, &ar);
    
    if (lua_getinfo (lua, "Sl", &ar) != 0)
        {
        printf ("exe %s on line %d\n", ar.short_src, ar.currentline);
        }
}

static void LuaHookCount (lua_State *lua)
{
    LuaHookLine (lua);
}

static void LuaHook (lua_State *lua, lua_Debug *ar)
{
        // Handover to the correct hook
    switch (ar->event)
    {
        case LUA_HOOKCALL:
        LuaHookCall (lua);
        break;
        case LUA_HOOKRET:
        LuaHookRet (lua);
        break;
        case LUA_HOOKLINE:
        LuaHookLine (lua);
        break;
        case LUA_HOOKCOUNT:
        LuaHookCount (lua);
        break;
    }
}


void report_errors(lua_State *L, int status)
{
    if ( status!=0 ) {
        std::cerr << "-- " << lua_tostring(L, -1) << std::endl;
        lua_pop(L, 1); // remove error message
    }
}

int main()
{
    int s=0;
    lua_State *L = luaL_newstate();
    
        // load the libs
    luaL_openlibs(L);
    
    LUA_REGISTER_CLASS(AutoFoo)
    LUA_REGISTER_METHOD(AutoFoo, foo)
    EXPOSE_TO_LUA(L, AutoFoo)
    
    Luna<Foo>::Register(L);
    
    luaL_loadfile(L, "/Coding/Sorgenti/gitrepos/EmbedLua/foo.lua");
    lua_sethook(L, LuaHook, LUA_MASKCOUNT, 1);
   
    if ( (s=lua_pcall(L, 0, LUA_MULTRET, 0)) != 0){
        report_errors(L, s);
    }
    lua_close(L);
    return s;
}
