
#ifndef GAME_SPRITESYSTEM_H
#define GAME_SPRITESYSTEM_H

#include "../../../room/Room.h"
#include "../EntitySystem.h"
#include "../../components/graphics/SpriteAnchor.h"
#include "../../components/graphics/AsepriteView.h"
#include "../../components/physics/Physics.h"

/**
 * see SpriteAnchor documentation
 */
class SpriteSystem : public EntitySystem
{
    using EntitySystem::EntitySystem;

  protected:

    Room *room = NULL;

    void update(double deltaTime, Room *room) override;

    void updateAnchors();

    void updateFeetBobbing(double deltaTime);

    void updateAnimations(double deltaTime);
};

#endif
