
#ifndef GAME_LUA_CONVERTERS_H
#define GAME_LUA_CONVERTERS_H

#include "../luau.h"
#include <json.hpp>

void jsonFromLuaTable(const sol::table &table, json &jsonOut);

void jsonToLuaTable(sol::table &table, const json &json);

template <>
struct sol::usertype_container<json> : public container_detail::usertype_container_default<json>
{
    static int index_get(lua_State *lua)
    {
        json &j = *sol::stack::unqualified_check_get<json *>(lua, 1).value();

        json *jOut = NULL;

        const char *keyStr = sol::stack::unqualified_check_get<const char *>(lua, 2).value_or((const char *) NULL);
        if (keyStr)
            jOut = &j.at(keyStr);

        else
        {
            int keyInt = sol::stack::unqualified_check_get<int>(lua, 2).value();
            jOut = &j.at(keyInt);
        }
        if (jOut->is_structured())
            sol::stack::push(lua, jOut);
        else if (jOut->is_boolean())
            sol::stack::push(lua, bool(*jOut));
        else if (jOut->is_null())
            sol::stack::push(lua, sol::nil);
        else if (jOut->is_number_float())
            sol::stack::push(lua, float(*jOut));
        else if (jOut->is_number())
            sol::stack::push(lua, int(*jOut));
        else if (jOut->is_string())
            sol::stack::push(lua, std::string(*jOut));
        return 1;
    }

};

#endif //GAME_LUA_CONVERTERS_H
