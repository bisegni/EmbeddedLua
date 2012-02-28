/*      
 *      LuaExposedObject.h
 *      EmbedLua
 *      Created by Claudio Bisegni on 28/02/12.
 *      
 *      Copyright 2012 INFN, National Institute of Nuclear Physics
 *
 *      Licensed under the Apache License, Version 2.0 (the "License");
 *      you may not use this file except in compliance with the License.
 *      You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *      Unless required by applicable law or agreed to in writing, software
 *      distributed under the License is distributed on an "AS IS" BASIS,
 *      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *      See the License for the specific language governing permissions and
 *      limitations under the License.
 */



#ifndef EmbedLua_LuaExposedObject_h
#define EmbedLua_LuaExposedObject_h

#include <lua.hpp>
#include <vector>
#include <string>

using namespace std;


#define LUA_REGISTER_CLASS(c)\
LuaObjectFactoryRegister<c>::registerLuaClassName(#c);

#define LUA_REGISTER_METHOD(c,m)\
LuaObjectFactoryRegister<c>::registerLuaMethodName(#m,&c::m);

#define EXPOSE_TO_LUA(l,c)\
LuaObjectFactoryRegister<c>::registerWithLua(l);



/*
 Template class for keep track of aliast+factory for the object that has been
 registert for the type T
 */
template <typename T>
class LuaObjectFactoryRegister {
    
public:
    typedef int(T::*LuaExportedMethod)(lua_State*);
    
    static string className;
    //object factories map
    static vector<string>  objectExposedMethodName;
    static vector<LuaExportedMethod>  objectExposedMethodPtr;

    static void registerLuaClassName(const char * name){
        className = name;
    }
    
    static void registerLuaMethodName(const char * name, LuaExportedMethod m){
        objectExposedMethodName.push_back(string(name));
        objectExposedMethodPtr.push_back(m);
    }
    
    static void registerWithLua(lua_State *L) {
        lua_pushcfunction(L, &LuaObjectFactoryRegister<T>::constructor);
        lua_setglobal(L, LuaObjectFactoryRegister<T>::className.c_str());
        
        luaL_newmetatable(L, LuaObjectFactoryRegister<T>::className.c_str());
        lua_pushstring(L, "__gc");
        lua_pushcfunction(L, &LuaObjectFactoryRegister<T>::gc_obj);
        lua_settable(L, -3);
    }
    
    static int constructor(lua_State *L) {
        T* obj = new T(L);
        
        lua_newtable(L);
        lua_pushnumber(L, 0);
        T** a = (T**)lua_newuserdata(L, sizeof(T*));
        *a = obj;
        luaL_getmetatable(L, LuaObjectFactoryRegister<T>::className.c_str());
        lua_setmetatable(L, -2);
        lua_settable(L, -3); // table[0] = obj;
        int idx = 0;
        for(vector<string>::iterator iter = objectExposedMethodName.begin();
            iter != objectExposedMethodName.end();
            iter++){
            lua_pushstring(L, iter->c_str());
            lua_pushnumber(L, idx++);
            lua_pushcclosure(L, &LuaObjectFactoryRegister<T>::thunk, 1);
            lua_settable(L, -3);
        }

        return 1;
    }
    
    static int thunk(lua_State *L) {
        int i = (int)lua_tonumber(L, lua_upvalueindex(1));
        lua_pushnumber(L, 0);
        lua_gettable(L, 1);
        
        T** obj = static_cast<T**>(luaL_checkudata(L, -1, LuaObjectFactoryRegister<T>::className.c_str()));
        lua_remove(L, -1);
        return ((*obj)->*(T::objectExposedMethodPtr[i]))(L);
    }
    
    static int gc_obj(lua_State *L) {
        T** obj = static_cast<T**>(luaL_checkudata(L, -1, LuaObjectFactoryRegister<T>::className.c_str()));
        delete (*obj);
        return 0;
    }
};

template <class T> string                                                               LuaObjectFactoryRegister<T>::className;
template <class T> vector<string>                                                       LuaObjectFactoryRegister<T>::objectExposedMethodName;
template <class T> vector<typename LuaObjectFactoryRegister<T>::LuaExportedMethod>      LuaObjectFactoryRegister<T>::objectExposedMethodPtr;
#endif
