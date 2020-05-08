
#ifndef GAME_ASEPRITEVIEW_H
#define GAME_ASEPRITEVIEW_H

#include "../../../macro_magic/component.h"
#include <utils/aseprite/Aseprite.h>
#include <asset_manager/asset.h>

COMPONENT(
    AsepriteView,

    HASH(sprite.getLoadedAsset().shortPath, playingTag, paused, zIndex, aabbAlign.x, aabbAlign.y, originAlign.x, originAlign.y),

    FIELD(asset<aseprite::Sprite>, sprite),
    FIELD_DEF_VAL(int, frame, 0),
    FIELD_DEF_VAL(int, playingTag, -1),
    FIELD_DEF_VAL(bool, paused, false),
    FIELD_DEF_VAL(float, zIndex, 0),

    FIELD_DEF_VAL(vec2, aabbAlign, vec2(.5)), // by default the sprite's origin is positioned at the center of the AABB of the entity
    FIELD_DEF_VAL(vec2, originAlign, vec2(.5)) // by default the sprite's origin is the center of the sprite.
)

    float frameTimer = 0;
    bool pong = false;

    void playTag(const char *tagName, bool unpause=false)
    {
        int i =  0;
        for (auto &tag : sprite->tags)
        {
            if (tag.name == tagName)
            {
                frame = tag.from;
                frameTimer = 0;
                pong = false;
                playingTag = i;
                paused = unpause ? false : paused;
                break;
            }
            i++;
        }
    }

END_COMPONENT(AsepriteView)

#endif //GAME_ASEPRITEVIEW_H
