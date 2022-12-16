#include "PlayerSaveData.h"

#define MAX_ROLE_NUM 1

using namespace gs;
using namespace std;

void PlayerSaveData::SetPlayerInfo(const string& playerID, const string& password) {
    m_playerData->playerID = playerID;
    m_playerData->password = password;
    m_needSave = true;
}

const std::string& PlayerSaveData::GetPassword() const {
    return m_playerData->password;
}

const std::unordered_map<int, RoleData>& PlayerSaveData::GetRoleDatas() const {
    return m_playerData->roleDatas;
}

const RoleData* PlayerSaveData::GetRoleDataByID(int id) const {
    RoleData* roleData = nullptr;
    auto it = m_playerData->roleDatas.find(id);
    if (it != m_playerData->roleDatas.end()) {
        roleData = &(it->second);
    }
    return roleData;
}

bool PlayerSaveData::SetRoleDataByID(int id, const RoleData& roleData) {
    bool ret = false;
    auto it = m_playerData->roleDatas.find(id);
    if (it != m_playerData->roleDatas.end()) {
        it->second = roleData;
        ret = true;
        m_needSave = true;
    }
    return ret;
}

bool PlayerSaveData::AddRoleDataByID(int id) {
    bool ret = false;
    auto it = m_playerData->roleDatas.find(id);
    if (it == m_playerData->roleDatas.end() &&
        m_playerData->roleDatas.size() < MAX_ROLE_NUM) {
        m_playerData->roleDatas.insert(pair<int, RoleData>(id, RoleData(id)));
        ret = true;
        m_needSave = true;
    }
    return ret;
}

uint64_t PlayerSaveData::GetExpireTime() const {
    return m_expireTime;
}

void PlayerSaveData::SetExpireTime(uint64_t timestamp) {
    m_expireTime = timestamp;
}

bool PlayerSaveData::GetNeedSave() const {
    return m_needSave;
}

void PlayerSaveData::SetNeedSave(bool flag) {
    m_needSave = flag;
}

bool PlayerSaveData::GetIsSaving() const {
    return m_isSaving;
}

void PlayerSaveData::SetIsSaving(bool flag) {
    m_isSaving = flag;
}

PlayerData* PlayerSaveData::Ptr() {
    return m_playerData;
}