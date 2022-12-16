#pragma once

#include <vector>

#include "Utils/Define.h"
#include "Interface/IModule.h"

MY_SERVICE_MGR_NS_BEGIN

class ModuleMgr {
public:
    bool Init();
    bool UnInit();
    bool Update();
    bool Quit();

    bool AddModule(IModule* module);

private:
    std::vector<IModule*> m_modules;
};

MY_SERVICE_MGR_NS_END