#include <ServiceMgr.h>

#include "NetPackDispatcher/NetPackDispatcher.h"
#include "NetMgr/NetMgr.h"
#include "LoginMgr/LoginMgr.h"
#include "FileDB/FileDB.h"
#include "RoleMgr/RoleMgr.h"
#include "ShopMgr/ShopMgr.h"
#include "SceneMgr/SceneMgr.h"

msm::Service* g_gameService = nullptr;

bool GameServiceInit() {
    g_gameService = msm::ServiceMgr::Ins().AddService(&(gs::NetMgr::Ins()), "0.0.0.0", 8086);

    gs::NetPackDispatcher::Ins().Init();

    g_gameService->AddModule(&(gs::NetMgr::Ins()));
    g_gameService->AddModule(&(gs::LoginMgr::Ins()));
    g_gameService->AddModule(&(gs::FileDB::Ins()));
    g_gameService->AddModule(&(gs::RoleMgr::Ins()));
    g_gameService->AddModule(&(gs::ShopMgr::Ins()));
    g_gameService->AddModule(&(gs::SceneMgr::Ins()));

    return true;
}

int main() {
    msm::ServiceMgr::Ins().Init();
    
    GameServiceInit();

    msm::ServiceMgr::Ins().Start();
}
