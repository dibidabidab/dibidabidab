
#ifndef GAME_BOWWEAPONSYSTEM_H
#define GAME_BOWWEAPONSYSTEM_H

#include "../EntitySystem.h"
#include "../../../room/Room.h"
#include "../../components/combat/Bow.h"
#include "../../components/physics/Physics.h"
#include "../../components/graphics/AsepriteView.h"
#include "../../components/combat/Aiming.h"
#include "../../components/PlatformerMovement.h"
#include "../../entity_templates/ArrowEntity.h"
#include "../../components/body_parts/Arm.h"

class BowWeaponSystem : public EntitySystem
{
    using EntitySystem::EntitySystem;

  protected:
    void update(double deltaTime, Room *room) override
    {
        room->entities.view<Bow, AABB, AsepriteView>().each([&](Bow &bow, AABB &aabb, AsepriteView &sprite) {

            AABB *archerAABB = room->entities.try_get<AABB>(bow.archer);
            Aiming *aim = room->entities.try_get<Aiming>(bow.archer);
            if (!archerAABB || !aim)
                return; // bow is not being held by an valid entity.

            ivec2 pivot = archerAABB->center + bow.rotatePivot;
            vec2 aimDirection = aim->target == pivot ? bow.prevAimDir : normalize(vec2(aim->target - pivot));
            bow.prevAimDir = aimDirection;

            // rotate bow around archer:
            aabb.center = pivot + ivec2(aimDirection * bow.distanceFromArcher);

            // choose frame based on angle: (frame 0 = not rotated, frame 1 = 45 deg rotated)
            float angle = atan2(aimDirection.y, aimDirection.x); // https://stackoverflow.com/questions/35271222/getting-the-angle-from-a-direction-vector
            float angleMod = mod(angle, mu::PI * .5f);
            sprite.rotate90Deg = false;
            if (angleMod > mu::PI * .25 * .5 && angleMod < mu::PI * .75 * .5)
            {
                if (sprite.playingTag != 1)
                {
                    sprite.playTag("angle1");
                    sprite.paused = true;
                    assert(sprite.playingTag == 1);
                }
                sprite.flipHorizontal = aabb.center.x < pivot.x;
                sprite.flipVertical = aabb.center.y < pivot.y;
            }
            else
            {
                if (sprite.playingTag != 0)
                {
                    sprite.playTag("angle0");
                    sprite.paused = true;
                    assert(sprite.playingTag == 0);
                }
                sprite.flipHorizontal = aabb.center.x < pivot.x;
                sprite.flipVertical = aabb.center.y < pivot.y;
                if (abs(aimDirection.y) > abs(aimDirection.x))
                {
                    sprite.rotate90Deg = true;
                    sprite.flipHorizontal = aimDirection.y < 0;
                }
            }

            PlatformerMovementInput *input = room->entities.try_get<PlatformerMovementInput>(bow.archer);

            bow.cooldown += deltaTime;

            if (input && input->attack && bow.cooldown >= bow.fireRate)
            {
                bow.cooldown = 0;
                auto arrowEntity = room->getTemplate<ArrowEntity>()->create();

                vec2 arrowSpawnPoint = aabb.center;
                room->entities.get<AABB>(arrowEntity).center = arrowSpawnPoint;
                room->entities.get<Physics>(arrowEntity).velocity = aimDirection * 1000.f;
                room->entities.get<Arrow>(arrowEntity).launchedBy = bow.archer;

                sprite.paused = false;
            }

            // make the archer's hands grab the bow:
            Arm *leftArm = room->entities.try_get<Arm>(bow.archerLeftArm);
            Arm *rightArm = room->entities.try_get<Arm>(bow.archerRightArm);

            if (leftArm && rightArm)
            {
                if (aabb.center.x < pivot.x)
                {
                    leftArm->grab = bow.handBowAnchor;
                    rightArm->grab = !sprite.paused ? bow.handStringAnchor : entt::null;
                }
                else
                {
                    rightArm->grab = bow.handBowAnchor;
                    leftArm->grab = !sprite.paused ? bow.handStringAnchor : entt::null;
                }
            }
        });
    }

};


#endif
