#pragma once

#include <memory>

#include "DataStruct/PlayerData.h"

GAME_SERVICE_NS_BEGIN

class PlayerSaveData {
public:
    PlayerSaveData() :
        m_playerData(new PlayerData()),
        m_needSave(false),
        m_isSaving(false),
        m_expireTime(0) {}

    ~PlayerSaveData() { delete m_playerData; }

    void SetPlayerInfo(const std::string& playerID, const std::string& password);
    const std::string& GetPassword() const;

    const std::unordered_map<int, RoleData>& GetRoleDatas() const;
    const RoleData* GetRoleDataByID(int id) const;
    bool SetRoleDataByID(int id, const RoleData& roleData);
    bool AddRoleDataByID(int id);

    uint64_t GetExpireTime() const;
    void SetExpireTime(uint64_t timestamp);

    bool GetNeedSave() const;
    void SetNeedSave(bool flag);

    bool GetIsSaving() const;
    void SetIsSaving(bool flag);

    PlayerData* Ptr();

private:
    PlayerData* m_playerData;
    
    bool      m_needSave;
    bool      m_isSaving;
    uint64_t  m_expireTime;
};

GAME_SERVICE_NS_END