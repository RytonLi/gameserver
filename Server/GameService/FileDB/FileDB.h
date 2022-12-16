#pragma once

#include <unordered_map>
#include <set>
#include <vector>
#include <functional>

#include <ServiceMgr.h>

#include "Utils/Define.h"
#include "PlayerSaveData.h"

GAME_SERVICE_NS_BEGIN

enum FILEDB_RESULT {
    emFileDBSuccess        = 0,
    emFileDBPlayerExist    = 1,
    emFileDBSaveError      = 2,
    emFileDBPlayerNotFound = 3,
    emFileDBInternalError  = 4,
    emFileDBDataNotFound   = 5,
    emFileDBRoleCountMax   = 6,
};

class FileDB : public IModule, public ISingleton<FileDB> {
public:
	bool Init() override;
	bool UnInit() override;
    bool Update() override;
    bool Quit() override;

    void CreatePlayer(const std::string& playerID, const std::string& password, std::function<void(FILEDB_RESULT)> callback);

    void GetPasswordByPlayerID(const std::string& playerID, std::function<void(FILEDB_RESULT, const std::string&)> callback);

    void GetAllRoleDataByPlayerID(const std::string& playerID, std::function<void(FILEDB_RESULT, const std::vector<RoleData>&)> callback);

    void GetRoleDataByPlayerIDAndRoleID(const std::string& playerID, int roleID, std::function<void(FILEDB_RESULT, const RoleData&)> callback);

    void AddRoleByPlayerID(const std::string& playerID, std::function<void(FILEDB_RESULT)> callback);

    void UpdateRoleDataByPlayerIDAndRoleID(const std::string& playerID, int roleID, const RoleData& roleData);

private:
	enum ERROR_CODE : uint8_t {
		emSuccess      = 0,
		emAccessing    = 1,
		emFileNotFound = 2,
		emMallocFail   = 3,
		emOpenFail     = 4,
		emReadFail     = 5,
		emParseFail    = 6,
		emDataNotFound = 7,
		emWriteFail    = 8,
	};
	using FileAccessCallback = std::function<void(ERROR_CODE)>;

    void _GetPlayerSaveData(const std::string& playerID, std::function<void(PlayerSaveData*)> callback);

    bool _CheckPlayerExist(const std::string& playerID);

    void _AddPlayerSaveData(const std::string& playerID, PlayerSaveData* saveData);
    void _RemovePlayerSaveData(const std::string& playerID);

	void _LoadPlayerSaveData(const std::string& playerID, FileAccessCallback callback);

	void _SavePlayerSaveData(const std::string& playerID, FileAccessCallback callback);

    bool _LoadCurMaxRoleID();
    bool _SaveCurMaxRoleID();

	std::unordered_map<std::string, PlayerSaveData*> m_playerSaveDatas;

    int m_curMaxRoleID;
};

GAME_SERVICE_NS_END