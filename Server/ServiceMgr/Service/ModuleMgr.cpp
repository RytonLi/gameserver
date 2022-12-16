#include "ModuleMgr.h"
#include "Log/Log.h"

using namespace std;
using namespace msm;

bool ModuleMgr::Init() {
    m_modules.clear();
    return true;
}

bool ModuleMgr::UnInit() {
    return true;
}

bool ModuleMgr::Update() {
    for (IModule* module : m_modules) {
        module->Update();
    }
    return true;
}

bool ModuleMgr::Quit() {
    return true;
}

bool ModuleMgr::AddModule(IModule* module) {
    if (module->Init()) {
        m_modules.push_back(module);
    }
    else {
        LOG_ERROR("module Init fail");
    }
    return true;
}