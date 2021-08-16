#pragma once

#include <functional>

#include <sol/sol.hpp>

class LuaInterface {
public:
    LuaInterface() {
        lua.open_libraries(sol::lib::base);
    }

    void run_script_file(const char *script_file) {
        lua.script_file(script_file);
    }

    template<class... Args>
    void call_func(const char *func_name, Args... args) {
        sol::function func = lua[func_name];
        func(args...);
    }

    template<class Func>
    void set_func(const char *func_name, Func &&func) {
        lua.set_function(func_name, func);
    }

    template<class Var>
    void set_var(const char *var_name, Var var) {
        lua[var_name] = var;
    }
private:
    sol::state lua;
};
