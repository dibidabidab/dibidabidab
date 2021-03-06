
#include "SpawningSystem.h"
#include "../../generated/Spawning.hpp"

void SpawningSystem::update(double deltaTime, EntityEngine *room)
{
    this->room = room;
    room->entities.view<DespawnAfter>().each([&](auto e, DespawnAfter &despawnAfter) {
        despawnAfter.timer += deltaTime;
        if (despawnAfter.timer >= despawnAfter.time)
            room->entities.destroy(e);
    });

    room->entities.view<TemplateSpawner>().each([&](auto e, TemplateSpawner &spawner) {

        spawner.timer += deltaTime;
        if (spawner.timer < spawner.nextTime)
            return;

        spawner.timer = 0;
        spawner.nextTime = mu::random(spawner.minDelay, spawner.maxDelay);

        int q = round(mu::random(spawner.minQuantity, spawner.maxQuantity));
        for (int i = 0; i < q; i++)
            spawn(e, spawner);
    });
}

void SpawningSystem::spawn(entt::entity spawnerEntity, TemplateSpawner &spawner)
{
    try
    {
        auto spawned = room->entities.create();
        auto &spawnedBy = room->entities.assign<SpawnedBy>(spawned);
        spawnedBy.spawner = spawnerEntity;
        spawnedBy.customData = spawner.customData;
        spawnedBy.spawnerPos = room->getPosition(spawnerEntity);

        room->getTemplate(spawner.templateName).createComponents(spawned);
    }
    catch (_gu_err &err)
    {
        std::cerr << "TemplateSpawner#" << int(spawnerEntity) << " caused error:\n" << err.what() << std::endl;
    }
}
