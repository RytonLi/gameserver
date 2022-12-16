#include "ShopMgr.h"
#include "cJSON/cJSON.h"
#include "Utils/TimeStampConvert.h"

#define SHOP_LIST_PATH "./Config"

using namespace std;
using namespace gs;
using namespace TCCamp;

bool ShopMgr::Init() {
    m_shopItems.clear();
    _LoadShopList();
	return true;
}

bool ShopMgr::UnInit() {
	return true;
}

bool ShopMgr::Update() {
    return true;
}

bool ShopMgr::Quit() {
    
    return true;
}

void ShopMgr::GetShopList(std::function<void(TCCamp::PB_RESULT_CODE, const std::unordered_map<int, ShopItem>&)> callback) {
    callback(PB_RESULT_CODE::SUCCESS, m_shopItems);
    return;
}

void ShopMgr::BuyItem(int itemID, int buyCount, int money, int personalLimit, function<void(TCCamp::PB_RESULT_CODE, int, int)> callback) {
    auto it = m_shopItems.find(itemID);
    PB_RESULT_CODE res;
    if(it == m_shopItems.end()){
        res = SHOP_BUY_ITEM_NOT_EXIST;
        goto Exit0;
    }

    {
        ShopItem& item = it->second;
        time_t nowTime = time(NULL);
        
        if (item.personalLimitMax != -1) {
            if (personalLimit == -1) {
                personalLimit = item.personalLimitMax;
            }
            if (buyCount > personalLimit) {
                res = SHOP_BUY_ITEM_PERSONAL_SHORTAGE;
                goto Exit0;
            }
        }

        if (item.globalLimitMax != -1 &&
            buyCount > item.globalLimit) {
            res = SHOP_BUY_ITEM_GLOBAL_SHORTAGE;
            goto Exit0;
        }

        if ((item.startTime != 0 && nowTime < item.startTime) ||
            (item.endTime != 0 && nowTime > item.endTime)) {
            res = SHOP_BUY_TIME_INVALID;
            goto Exit0;
        }

        if (buyCount * item.price > money) {
            res = SHOP_BUY_MONEY_SHORTAGE;
            goto Exit0;
        }

        money -= buyCount * item.price;
        if (item.globalLimitMax != -1) item.globalLimit -= buyCount;
        if (personalLimit != -1) personalLimit -= buyCount;
        res = SUCCESS;
    }

Exit0:
    callback(res, money, personalLimit);
    return;
}

bool ShopMgr::_LoadShopList() {
    bool ret = false;
    char path[512] = { 0 };
    msm::FILE_IO_RESULT res;
    char* buf = nullptr;
    uint64_t bufSize;
    int r;

    sprintf(path, "%s/%s", SHOP_LIST_PATH, "ShopList.json");

    res = msm::ServiceMgr::Ins().Open(path, O_RDONLY);
    if (res != msm::emFileIOSuccess) {
        LOG_ERROR("on load shop list, open file fail");
        goto Exit0;
    }

    bufSize = msm::ServiceMgr::Ins().GetFileSize(path);
    buf = (char*)malloc(bufSize);
    if (!buf) {
        LOG_ERROR("on load shop list, malloc buf fail");
        goto Exit0;
    }

    r = msm::ServiceMgr::Ins().ReadSync(path, buf, bufSize, 0);
    if (r < bufSize) {
        LOG_ERROR("on load shop list, read result: %d, buf size: %d", r, bufSize);
        goto Exit0;
    }
    
    _ParseShopList(buf, bufSize);

Exit0:
    if (buf) {
        free(buf);
    }
    return true;
}

bool ShopMgr::_ParseShopList(const char* data, int len) {
    bool ret = false;
    int itemNum;

    const cJSON* itemArray     = nullptr;
    const cJSON* readItem      = nullptr;
    const cJSON* id            = nullptr;
    const cJSON* price         = nullptr;
    const cJSON* personalLimit = nullptr;
    const cJSON* globalLimit   = nullptr;
    const cJSON* startTime     = nullptr;
    const cJSON* endTime       = nullptr;

    cJSON* jsonData = cJSON_ParseWithLength(data, len);
    if (jsonData == NULL) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            LOG_ERROR("parse json error: %s\n", error_ptr);
        }
        goto Exit0;
    }

    itemArray = cJSON_GetObjectItemCaseSensitive(jsonData, "shopItems");
    if (!cJSON_IsArray(itemArray)) {
        LOG_ERROR("invalid data, roles field is not array\n");
        goto Exit0;
    }

    itemNum = cJSON_GetArraySize(itemArray);
    for (int i = 0; i < itemNum; i++) {
        readItem = cJSON_GetArrayItem(itemArray, i);
        ShopItem item;

        //获取商品ID
        id = cJSON_GetObjectItemCaseSensitive(readItem, "id");
        if (!cJSON_IsNumber(id)) {
            LOG_ERROR("invalid data, id field is not number, on index: %d\n", i);
            goto Exit0;
        }
        item.id = id->valueint;

        //获取商品价格
        price = cJSON_GetObjectItemCaseSensitive(readItem, "price");
        if (!cJSON_IsNumber(price)) {
            LOG_ERROR("invalid data, price field is not number, on index: %d\n", i);
            goto Exit0;
        }
        item.price = price->valueint;

        //获取个人限购量
        personalLimit = cJSON_GetObjectItemCaseSensitive(readItem, "personalLimit");
        if (!cJSON_IsNumber(personalLimit)) {
            LOG_ERROR("invalid data, personalLimit field is not number, on index: %d\n", i);
            goto Exit0;
        }
        item.personalLimitMax = personalLimit->valueint;

        //获取总限购量
        globalLimit = cJSON_GetObjectItemCaseSensitive(readItem, "globalLimit");
        if (!cJSON_IsNumber(globalLimit)) {
            LOG_ERROR("invalid data, globalLimit field is not number, on index: %d\n", i);
            goto Exit0;
        }
        item.globalLimit = globalLimit->valueint;
        item.globalLimitMax = globalLimit->valueint;

        //获取开始时间
        startTime = cJSON_GetObjectItemCaseSensitive(readItem, "startTime");
        if (!cJSON_IsString(startTime)) {
            LOG_ERROR("invalid data, startTime field is not string, on index: %d\n", i);
            goto Exit0;
        }
        item.startTime = ConvertStringToTimeStamp(startTime->valuestring);

        //获取结束时间
        endTime = cJSON_GetObjectItemCaseSensitive(readItem, "endTime");
        if (!cJSON_IsString(endTime)) {
            LOG_ERROR("invalid data, endTime field is not string, on index: %d\n", i);
            goto Exit0;
        }
        item.endTime = ConvertStringToTimeStamp(endTime->valuestring);

        m_shopItems.insert(pair<int, ShopItem>(item.id, move(item)));
    }

    ret = true;
Exit0:
    return ret;
}

