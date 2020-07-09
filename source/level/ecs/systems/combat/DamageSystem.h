
#ifndef GAME_DAMAGESYSTEM_H
#define GAME_DAMAGESYSTEM_H

#include "../EntitySystem.h"
#include <asset_manager/asset.h>

struct DamageType;

class DamageSystem : public EntitySystem
{

    asset<json> damageTypesJson;

    std::map<std::string, DamageType> damageTypes;

  protected:

    void loadDamageTypes();

    void update(double deltaTime, Room *room) override;

  public:
    DamageSystem();

};


#endif
