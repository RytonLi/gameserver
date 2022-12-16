#pragma once

#include <unordered_map>
#include <google/protobuf/message.h>

#include "Proto/player.pb.h"
#include "Entity.h"

GAME_SERVICE_NS_BEGIN

class SceneRole : public Entity {
public:
    SceneRole(uint64_t clientID, int roleID) :
        m_clientID(clientID),
        m_entities(),
        Entity(emEntityRole, roleID, Transform()) {}
    virtual ~SceneRole() = default;

    //进入场景
    bool EnterScene(int sceneID);
    //离开场景
    bool ExitScene();

    bool OnMoveSync(Position pos, Rotation rot, int speed);

    bool Update(uint64_t diff) override;
    bool LateUpdate() override;

    bool OnEntityEnter(Entity* entity) override;
    bool OnEntityExit(Entity* entity) override;
    bool OnEntityMove(Entity* entity) override;

private:
    bool _SendToClient(int cmd, google::protobuf::Message* msg);

    uint64_t m_clientID;

    std::unordered_map<int, Entity*> m_entities;

    TCCamp::SceneSyncAnnounce m_syncAnnounce;
    TCCamp::ExitSceneAnnounce m_exitAnnounce;
};

GAME_SERVICE_NS_END