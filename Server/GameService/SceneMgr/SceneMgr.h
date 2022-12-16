#pragma once

#include <unordered_map>

#include <ServiceMgr.h>

#include "Utils/Define.h"
#include "Scene.h"

GAME_SERVICE_NS_BEGIN

class SceneMgr : public IModule, public ISingleton<SceneMgr> {
public:
    bool Init() override;
    bool UnInit() override;
    bool Update() override;
    bool Quit() override;

    Scene* GetScene(int sceneID);

private:
    bool _LoadSceneConfig();
    bool _ParseSceneConfig(const char* data, int len);

    std::unordered_map<int, Scene*> m_scenes;
};

GAME_SERVICE_NS_END