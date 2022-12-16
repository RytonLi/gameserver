#pragma once

#include <vector>

#include "../Utils/Define.h"
#include "../Interface/IModule.h"

namespace msm {

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

}