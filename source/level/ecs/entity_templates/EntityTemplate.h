
#ifndef GAME_ENTITYTEMPLATE_H
#define GAME_ENTITYTEMPLATE_H


#include "../../../../entt/src/entt/entity/registry.hpp"

class Room;
class Networked;

/**
 * Abstract class.
 *
 * Entity templates are used to construct one (or more) entities with a collection of components.
 *
 * .create() is used to create the entity as if it only exists client-side.
 *
 * .createNetworked() is used to create the entity as if it exists on all clients.
 */
class EntityTemplate
{
  private:
    friend class Room;
    int templateHash = -1;

  protected:
    Room *room = NULL;

  public:
    virtual entt::entity create() = 0;

    entt::entity createNetworked(int networkID=rand(), bool serverSide=true);

  protected:
    virtual void makeNetworkedServerSide(Networked &) {}

    virtual void makeNetworkedClientSide(Networked &) {}

    virtual ~EntityTemplate() = default;
};


#endif
