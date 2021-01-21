
#ifndef GAME_LUA_ASSET_H
#define GAME_LUA_ASSET_H

#include <asset_manager/asset.h>
#include <asset_manager/AssetManager.h>
#include "luau.h"

template<typename type>
void addLuaAssetLoader(
        const std::string &luaName,
        const std::string &assetFileSuffix,
        std::function<type *(const std::string &path)> function
)
{
    AssetManager::addAssetLoader(assetFileSuffix, function);

    auto assetType = luau::getLuaState().new_usertype<asset<type>>(luaName, sol::call_constructor,
            sol::constructors<asset<type>(), asset<type>(const std::string &)>(),
            "set", &asset<type>::set,
            "isSet", &asset<type>::isSet,
            "unset", &asset<type>::unset
    );
}

#endif //GAME_LUA_ASSET_H
