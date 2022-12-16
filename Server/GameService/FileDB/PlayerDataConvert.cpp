#include "PlayerDataConvert.h"
#include <ServiceMgr.h>

using namespace gs;
using namespace std;

bool _GetRoleData(const cJSON* jsonData, PlayerData* playerData);
bool _GetBagItem(const cJSON* readRole, RoleData* role);
bool _GetBuyLimit(const cJSON* readRole, RoleData* role);

bool PlayerDataConvert::ParseFromJsonData(const char* data, int len, PlayerData* playerData) {
	bool ret = false;
	const cJSON* playerID = nullptr;
	const cJSON* password = nullptr;

	cJSON* jsonData = cJSON_ParseWithLength(data, len);
	if (jsonData == NULL) {
		const char *error_ptr = cJSON_GetErrorPtr();
		if (error_ptr != NULL) {
			LOG_ERROR("parse json error: %s", error_ptr);
		}
		goto Exit0;
	}

	playerID = cJSON_GetObjectItemCaseSensitive(jsonData, "playerID");
	if (!cJSON_IsString(playerID)) {
		LOG_ERROR("invalid data, playerID field is not string");
		goto Exit0;
	}
	playerData->playerID = playerID->valuestring;

	password = cJSON_GetObjectItemCaseSensitive(jsonData, "password");
	if (!cJSON_IsString(password)) {
		LOG_ERROR("invalid data, password field is not string");
		goto Exit0;
	}
	playerData->password = password->valuestring;

	ret = _GetRoleData(jsonData, playerData);
	if (!ret) {
		goto Exit0;
	}

	ret = true;
Exit0:
	cJSON_Delete(jsonData);
	return ret;
}

cJSON* PlayerDataConvert::ToJsonData(const PlayerData* playerData) {
	cJSON* jsonData = cJSON_CreateObject();

	cJSON_AddStringToObject(jsonData, "playerID", playerData->playerID.c_str());
	cJSON_AddStringToObject(jsonData, "password", playerData->password.c_str());
	
	cJSON* roleArray = cJSON_CreateArray();
	for (auto pRole : playerData->roleDatas) {
		const RoleData& role = pRole.second;
		cJSON* addRole = cJSON_CreateObject();

        cJSON_AddNumberToObject(addRole, "id", role.id);
        cJSON_AddStringToObject(addRole, "name", role.name.c_str());
		cJSON_AddNumberToObject(addRole, "level", role.level);
		cJSON_AddNumberToObject(addRole, "type", role.type);
		cJSON_AddNumberToObject(addRole, "money", role.money);

		cJSON* itemArray = cJSON_CreateArray();
		for (auto pBagItem : role.bagItems) {
			const BagItem& bagItem = pBagItem.second;

			cJSON* addItem = cJSON_CreateObject();
			cJSON_AddNumberToObject(addItem, "id", bagItem.id);
			cJSON_AddNumberToObject(addItem, "num", bagItem.num);
			cJSON_AddItemToArray(itemArray, addItem);
		}
		cJSON_AddItemToObject(addRole, "bagItems", itemArray);

        cJSON* limitArray = cJSON_CreateArray();
        for (auto pBuyLimit : role.buyLimits) {
            const BuyLimit& buyLimit = pBuyLimit.second;

            cJSON* addLimit = cJSON_CreateObject();
            cJSON_AddNumberToObject(addLimit, "id", buyLimit.id);
            cJSON_AddNumberToObject(addLimit, "num", buyLimit.num);
            cJSON_AddItemToArray(limitArray, addLimit);
        }
        cJSON_AddItemToObject(addRole, "buyLimits", limitArray);

		cJSON_AddItemToArray(roleArray, addRole);
	}

    cJSON_AddItemToObject(jsonData, "roles", roleArray);

	return jsonData;
}

bool _GetRoleData(const cJSON* jsonData, PlayerData* playerData) {
	bool ret = false;
	int roleNum;
    
	const cJSON* roleArray = nullptr;
	const cJSON* readRole  = nullptr;
    const cJSON* id        = nullptr;
    const cJSON* name      = nullptr;
	const cJSON* level     = nullptr;
	const cJSON* type      = nullptr;
	const cJSON* money     = nullptr;

	roleArray = cJSON_GetObjectItemCaseSensitive(jsonData, "roles");
	if (!cJSON_IsArray(roleArray)) {
		LOG_ERROR("invalid data, roles field is not array");
		goto Exit0;
	}

	roleNum = cJSON_GetArraySize(roleArray);
	for (int i = 0; i < roleNum; i++) {
		readRole = cJSON_GetArrayItem(roleArray, i);
		RoleData role;

        //��ȡ��ɫID
        id = cJSON_GetObjectItemCaseSensitive(readRole, "id");
        if (!cJSON_IsNumber(id)) {
            LOG_ERROR("invalid data, id field is not number, on index: %d", i);
            goto Exit0;
        }
        role.id = id->valueint;

		//��ȡ��ɫ��
		name = cJSON_GetObjectItemCaseSensitive(readRole, "name");
		if (!cJSON_IsString(name)) {
			LOG_ERROR("invalid data, name field is not string, on index: %d", i);
			goto Exit0;
		}
		role.name = name->valuestring;

		//��ȡ��ɫ�ȼ�
		level = cJSON_GetObjectItemCaseSensitive(readRole, "level");
		if (!cJSON_IsNumber(level)) {
			LOG_ERROR("invalid data, level field is not number, on index: %d", i);
			goto Exit0;
		}
		role.level = level->valueint;

		//��ȡ��ɫְҵ
		type = cJSON_GetObjectItemCaseSensitive(readRole, "type");
		if (!cJSON_IsNumber(type)) {
			LOG_ERROR("invalid data, type field is not number, on index: %d", i);
			goto Exit0;
		}
		role.type = level->valueint;

		//��ȡ��ɫ��Ǯ
		money = cJSON_GetObjectItemCaseSensitive(readRole, "money");
		if (!cJSON_IsNumber(money)) {
			LOG_ERROR("invalid data, money field is not number, on index: %d", i);
			goto Exit0;
		}
		role.money = money->valueint;

		//��ȡ������Ʒ
		ret = _GetBagItem(readRole, &role);
		if (!ret) {
			LOG_ERROR("invalid data, get bag item failed, on index: %d", i);
			goto Exit0;
		}

        //��ȡ��������
        ret = _GetBuyLimit(readRole, &role);
        if (!ret) {
            LOG_ERROR("invalid data, get buy limit failed, on index: %d", i);
            goto Exit0;
        }

		playerData->roleDatas.insert(pair<int, RoleData>(role.id, move(role)));
	}

	ret = true;
Exit0:
	return ret;
}

bool _GetBagItem(const cJSON* readRole, RoleData* role) {
	bool ret = false;
	int itemNum;
	const cJSON* itemArray = nullptr;
	const cJSON* readItem = nullptr;
	const cJSON* id = nullptr;
	const cJSON* num = nullptr;

	itemArray = cJSON_GetObjectItemCaseSensitive(readRole, "bagItems");
	if (!cJSON_IsArray(itemArray)) {
		LOG_ERROR("invalid data, bag items field is not array");
		goto Exit0;
	}

	itemNum = cJSON_GetArraySize(itemArray);
	for (int i = 0; i < itemNum; i++) {
		BagItem item;
		readItem = cJSON_GetArrayItem(itemArray, i);

		//��ȡ��Ʒid
		id = cJSON_GetObjectItemCaseSensitive(readItem, "id");
		if (!cJSON_IsNumber(id)) {
			LOG_ERROR("invalid data, id field is not number, on index: %d", i);
			goto Exit0;
		}
		item.id = id->valueint;

		//��ȡ��Ʒ����
		num = cJSON_GetObjectItemCaseSensitive(readItem, "num");
		if (!cJSON_IsNumber(num)) {
			LOG_ERROR("invalid data, num field is not number, on index: %d", i);
			goto Exit0;
		}
		item.num = num->valueint;

		role->bagItems.insert(pair<int, BagItem>(id->valueint, move(item)));
	}

	ret = true;
Exit0:
	return ret;
}

bool _GetBuyLimit(const cJSON* readRole, RoleData* role) {
    bool ret = false;
    int limitNum;
    const cJSON* limitArray = nullptr;
    const cJSON* readLimit = nullptr;
    const cJSON* id = nullptr;
    const cJSON* num = nullptr;

    limitArray = cJSON_GetObjectItemCaseSensitive(readRole, "buyLimits");
    if (!cJSON_IsArray(limitArray)) {
        LOG_ERROR("invalid data, buy limits field is not array");
        goto Exit0;
    }

    limitNum = cJSON_GetArraySize(limitArray);
    for (int i = 0; i < limitNum; i++) {
        BuyLimit limit;
        readLimit = cJSON_GetArrayItem(limitArray, i);

        //��ȡ��Ʒid
        id = cJSON_GetObjectItemCaseSensitive(readLimit, "id");
        if (!cJSON_IsNumber(id)) {
            LOG_ERROR("invalid data, id field is not number, on index: %d", i);
            goto Exit0;
        }
        limit.id = id->valueint;

        //��ȡ��Ʒ����
        num = cJSON_GetObjectItemCaseSensitive(readLimit, "num");
        if (!cJSON_IsNumber(num)) {
            LOG_ERROR("invalid data, num field is not number, on index: %d", i);
            goto Exit0;
        }
        limit.num = num->valueint;

        role->buyLimits.insert(pair<int, BuyLimit>(id->valueint, move(limit)));
    }

    ret = true;
Exit0:
    return ret;
}