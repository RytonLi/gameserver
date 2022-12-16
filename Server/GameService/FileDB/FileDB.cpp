#include "FileDB.h"
#include "PlayerDataConvert.h"

#define SAVE_DATA_CHECK_RATE 60
#define SAVE_DATA_EXPIRE_TIME 30

#define SAVE_PATH "./data"
#define CUR_MAX_ROLEID_FILE "cur_max_id"

using namespace gs;
using namespace std;
using namespace msm;

bool FileDB::Init() {
    m_playerSaveDatas.clear();
    if (_LoadCurMaxRoleID()) {
        LOG_DEBUG("on load cur max role id, id: %d", m_curMaxRoleID);
    }

    ServiceMgr::Ins().AddTimer(
        SAVE_DATA_CHECK_RATE * 1000,
        true,
        [this]() {
            for (auto it = m_playerSaveDatas.begin(); it != m_playerSaveDatas.end();) {
                const string& playerID = it->first;
                PlayerSaveData* saveData = it->second;
                if (saveData->GetNeedSave()) {
                    if (!saveData->GetIsSaving()) {
                        saveData->SetNeedSave(false);
                        _SavePlayerSaveData(
                            playerID,
                            [](ERROR_CODE code) {}
                        );
                    }
                    it++;
                }
                else {
                    if (!saveData->GetIsSaving() && 
                        saveData->GetExpireTime() <= ServiceMgr::Ins().Now()) {
                        LOG_DEBUG("save data expire");
                        delete it->second;
                        it = m_playerSaveDatas.erase(it);
                    }
                    else {
                        it++;
                    }
                }

            }
        }
    );

	return true;
}

bool FileDB::UnInit() {
	return true;
}

bool FileDB::Update() {
    return true;
}

bool FileDB::Quit() {
    return true;
}

void FileDB::CreatePlayer(const string& playerID, const string& password, function<void(FILEDB_RESULT)> callback) {
    PlayerSaveData* saveData = nullptr;

    if (_CheckPlayerExist(playerID)) {
        callback(emFileDBPlayerExist);
        goto Exit0;
    }

    saveData = new PlayerSaveData();
    
    saveData->SetPlayerInfo(playerID, password);

    _AddPlayerSaveData(playerID, saveData);

    callback(emFileDBSuccess);

Exit0:
    return;
}

void FileDB::GetPasswordByPlayerID(const std::string& playerID, std::function<void(FILEDB_RESULT, const string&)> callback) {
    _GetPlayerSaveData(
        playerID,
        [callback](PlayerSaveData* saveData) {
            if (saveData) {
                callback(emFileDBSuccess, saveData->GetPassword());
            }
            else {
                callback(emFileDBPlayerNotFound, "");
            }
        }
    );
}

void FileDB::GetAllRoleDataByPlayerID(const std::string& playerID, std::function<void(FILEDB_RESULT, const vector<RoleData>&)> callback) {
    _GetPlayerSaveData(
        playerID,
        [callback](PlayerSaveData* saveData) {
            vector<RoleData> roleDatas;
            if (saveData) {
                for (auto pRole : saveData->GetRoleDatas()) {
                    const RoleData& roleData = pRole.second;
                    roleDatas.push_back(roleData);
                }
                callback(emFileDBSuccess, roleDatas);
            }
            else {
                callback(emFileDBDataNotFound, roleDatas);
            }
        }
    );
}

void FileDB::GetRoleDataByPlayerIDAndRoleID(const std::string& playerID, int roleID, std::function<void(FILEDB_RESULT, const RoleData&)> callback) {
    _GetPlayerSaveData(
        playerID,
        [roleID, callback](PlayerSaveData* saveData) {
            const RoleData* roleData = saveData->GetRoleDataByID(roleID);
            if (roleData) {
                callback(emFileDBSuccess, *roleData);
            }
            else {
                callback(emFileDBDataNotFound, RoleData());
            }
        }
    );
}

void FileDB::AddRoleByPlayerID(const std::string& playerID, std::function<void(FILEDB_RESULT)> callback) {
    _GetPlayerSaveData(
        playerID,
        [callback, this](PlayerSaveData* saveData) {
            if (saveData) {
                bool res = saveData->AddRoleDataByID(m_curMaxRoleID++);
                if (res) {
                    callback(emFileDBSuccess);
                    _SaveCurMaxRoleID();
                }
                else {
                    callback(emFileDBRoleCountMax);
                    m_curMaxRoleID--;
                }
            }
            else {
                callback(emFileDBDataNotFound);
            }
        }
    );
}

void FileDB::UpdateRoleDataByPlayerIDAndRoleID(const std::string& playerID, int roleID, const RoleData& roleData) {
    _GetPlayerSaveData(
        playerID,
        [roleID, roleData](PlayerSaveData* saveData) {
            if (saveData) {
                saveData->SetRoleDataByID(roleID, roleData);
            }
            else {
                LOG_ERROR("on update role data, roleID: %d not found", roleID);
            }
        }
    );
}

void FileDB::_GetPlayerSaveData(const std::string& playerID, std::function<void(PlayerSaveData*)> callback) {
    if (!_CheckPlayerExist(playerID)) {
        callback(nullptr);
    }
    else {
        auto it = m_playerSaveDatas.find(playerID);
        if (it != m_playerSaveDatas.end()) {
            it->second->SetExpireTime(ServiceMgr::Ins().Now() + SAVE_DATA_EXPIRE_TIME * 1000);
            callback(it->second);
        }
        else {
            _LoadPlayerSaveData(
                playerID,
                [callback, playerID, this](ERROR_CODE code) {
                    if (code == FileDB::emSuccess) {
                        callback(m_playerSaveDatas[playerID]);
                    }
                    else {
                        LOG_ERROR("on load player save data error, code: %d", code);
                        callback(nullptr);
                    }
                }
            );
        }
    }

    return;
}

bool FileDB::_CheckPlayerExist(const std::string& playerID) {
    bool ret = false;
    char path[512] = { 0 };

    sprintf(path, "%s/%s", SAVE_PATH, playerID.c_str());

    if (m_playerSaveDatas.count(playerID) ||
        ServiceMgr::Ins().Access(path) == emFileIOSuccess) {
        ret = true;
    }

    return ret;
}

void FileDB::_AddPlayerSaveData(const std::string& playerID, PlayerSaveData* playerData) {
    m_playerSaveDatas.insert(pair<string, PlayerSaveData*>(playerID, playerData));
    playerData->SetExpireTime(ServiceMgr::Ins().Now() + SAVE_DATA_EXPIRE_TIME * 1000);
}

void FileDB::_RemovePlayerSaveData(const std::string& playerID) {
    delete m_playerSaveDatas[playerID];
    m_playerSaveDatas.erase(playerID);
}

void FileDB::_LoadPlayerSaveData(const std::string& playerID, FileAccessCallback callback) {
	char path[512] = { 0 };
	FILE_IO_RESULT res;
	char* buf = nullptr;
	uint64_t bufSize;

    sprintf(path, "%s/%s", SAVE_PATH, playerID.c_str());

    res = ServiceMgr::Ins().Open(path, O_RDONLY);
    if (res == emFileIONotFound) {
        LOG_DEBUG("on load player data, file not found, playerID: %s", playerID.c_str());
        callback(emFileNotFound);
        goto Exit0;
    }

	//申请加载文件内容buf
    bufSize = ServiceMgr::Ins().GetFileSize(path);
	buf = (char*)malloc(bufSize);
	if (!buf) {
		LOG_ERROR("on load player data, malloc fail, file size: %d, playerID: %s", bufSize, playerID.c_str());
		callback(emMallocFail);
		goto Exit0;
	}

	//读文件
	res = ServiceMgr::Ins().ReadAsync(path, buf, bufSize, 0,
		//读回调
		[bufSize, buf, playerID, callback, this](int res) {
			//TMP：没有读完
			if (res < bufSize) {
				LOG_ERROR("on load player data, read file fail, read res: %d, file size: %d, playerID: %s", 
					res, bufSize, playerID.c_str());
				callback(emReadFail);
			}
			else {
				//解析json存档
                PlayerSaveData* saveData = new PlayerSaveData;
				bool ret = PlayerDataConvert::ParseFromJsonData(buf, bufSize, saveData->Ptr());
				if (!ret) {
					LOG_ERROR("on load player data, parse fail, playerID: %s", playerID.c_str());
					callback(emParseFail);
					delete saveData;
				}
				else {
                    _AddPlayerSaveData(playerID, saveData);
					callback(emSuccess);
				}
			}
			free(buf);
		}
	);

	if (res != emFileIOSuccess) {
		callback(emReadFail);
		goto Exit0;
	}

	buf = nullptr;
Exit0:
	if (buf) {
		free(buf);
	}
    ServiceMgr::Ins().Close(path);
	return;
}

void FileDB::_SavePlayerSaveData(const std::string& playerID, FileAccessCallback callback) {
	PlayerSaveData* saveData = nullptr;
	char path[512] = { 0 };
	FILE_IO_RESULT res;
	cJSON* jsonData = nullptr;
	char* formData = nullptr;
	int dataSize;

    saveData = m_playerSaveDatas[playerID];

	if (saveData->GetIsSaving()) {
		LOG_DEBUG("on save player data, file is being accessed, playerID: %s", playerID.c_str());
		callback(emAccessing);
		goto Exit0;
	}

	sprintf(path, "%s/%s", SAVE_PATH, playerID.c_str());

	res = ServiceMgr::Ins().Open(path, O_WRONLY | O_TRUNC | O_CREAT);
	if (res != emFileIOSuccess) {
		LOG_ERROR("on save player data, open fail, playerID: %s", playerID.c_str());
		callback(emOpenFail);
		goto Exit0;
	}

	jsonData = PlayerDataConvert::ToJsonData(saveData->Ptr());
	formData = cJSON_Print(jsonData);
	dataSize = strlen(formData);

    saveData->SetIsSaving(true);

	res = ServiceMgr::Ins().WriteAsync(path, formData, dataSize, 0,
		[jsonData, formData, dataSize, playerID, callback, saveData](int res) {
			//TMP：没有写完
			if (res < dataSize) {
				LOG_ERROR("on save player data, write fail, write res: %d, data size: %d, playerID: %s",
					res, dataSize, playerID.c_str());
				callback(emWriteFail);
			}
			else {
				LOG_DEBUG("on save player data, write success, data size: %d, playerID: %s", res, playerID.c_str());
				callback(emSuccess);
			}
			
            saveData->SetIsSaving(false);
            cJSON_Delete(jsonData);
			free(formData);
		}
	);
	if (!res) {
		LOG_ERROR("on save player data, write fail, playerID: %s", playerID.c_str());
		callback(emWriteFail);
        saveData->SetIsSaving(false);
		goto Exit0;
	}

	jsonData = nullptr;
	formData = nullptr;
Exit0:
	if (jsonData) {
		cJSON_Delete(jsonData);
	}
	if (formData) {
		free(formData);
	}
    ServiceMgr::Ins().Close(path);
	return;
}

bool FileDB::_LoadCurMaxRoleID() {
    bool ret = false;
    char path[512] = { 0 };
    FILE_IO_RESULT res;
    int r;
    char* buf = nullptr;
    uint64_t bufSize;

    sprintf(path, "%s/%s", SAVE_PATH, CUR_MAX_ROLEID_FILE);

    res = ServiceMgr::Ins().Open(path, O_RDONLY);
    if (res == emFileIONotFound) {
        LOG_DEBUG("on load cur max role id, file not found");
        goto Exit0;
    }

    //申请加载文件内容buf
    bufSize = ServiceMgr::Ins().GetFileSize(path);
    buf = (char*)malloc(bufSize);
    if (!buf) {
        LOG_ERROR("on load cur max role id, malloc fail, file size: %d", bufSize);
        goto Exit0;
    }

    r = ServiceMgr::Ins().ReadSync(path, buf, bufSize, 0);

    if (r < bufSize) {
        LOG_ERROR("on load cur max role id, read file fail, read res: %d, file size: %d",
            r, bufSize);
        goto Exit0;
    }
    
    sscanf(buf, "%d", &m_curMaxRoleID);
    //LOG_DEBUG("on load cur max role id, id: %d", m_curMaxRoleID);

    ret = true;
Exit0:
    if (buf) {
        free(buf);
    }
    ServiceMgr::Ins().Close(path);
    return ret;
}

bool FileDB::_SaveCurMaxRoleID() {
    bool ret = false;
    char path[512] = { 0 };
    int res;
    char num[16];
    int dataSize;

    sprintf(path, "%s/%s", SAVE_PATH, CUR_MAX_ROLEID_FILE);

    res = ServiceMgr::Ins().Open(path, O_WRONLY | O_TRUNC | O_CREAT);
    if (res != emFileIOSuccess) {
        LOG_ERROR("on save cur max role id, open fail");
        goto Exit0;
    }

    sprintf(num, "%d", m_curMaxRoleID);
    dataSize = strlen(num);

    res = ServiceMgr::Ins().WriteSync(path, num, dataSize, 0);

    if (res < dataSize) {
        LOG_ERROR("on save cur max role id, write fail, write res: %d, data size: %d",
            res, dataSize);
        goto Exit0;
    }

    ret = true;
Exit0:
    ServiceMgr::Ins().Close(path);
    return ret;
}