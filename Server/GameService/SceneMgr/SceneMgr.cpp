#include "SceneMgr.h"
#include "cJSON/cJSON.h"
#include "SceneConfig.h"

#define  SCENE_CONFIG_PATH "./Config"

using namespace std;
using namespace gs;

bool SceneMgr::Init() {
    m_scenes.clear();

    if (_LoadSceneConfig()) {
        LOG_DEBUG("on load scene config, success");
    }

    return true;
}

bool SceneMgr::UnInit() {
    return true;
}

bool SceneMgr::Update() {
    for (auto it = m_scenes.begin(); it != m_scenes.end(); it++) {
        it->second->Update();
    }
    return true;
}

bool SceneMgr::Quit() {
    return true;
}

Scene* SceneMgr::GetScene(int sceneID) {
    Scene* scene = nullptr;

    auto it = m_scenes.find(sceneID);
    if (it != m_scenes.end()) {
        scene = it->second;
    }

    return scene;
}

bool SceneMgr::_LoadSceneConfig() {
    bool ret = false;
    char path[512] = { 0 };
    msm::FILE_IO_RESULT res;
    char* buf = nullptr;
    int bufSize;
    int r;

    sprintf(path, "%s/%s", SCENE_CONFIG_PATH, "SceneConfig.json");

    res = msm::ServiceMgr::Ins().Open(path, O_RDONLY);
    if (res != msm::emFileIOSuccess) {
        LOG_ERROR("on load scene config, open file fail");
        goto Exit0;
    }

    bufSize = msm::ServiceMgr::Ins().GetFileSize(path);
    buf = (char*)malloc(bufSize);
    if (!buf) {
        LOG_ERROR("on load scene config, malloc buf fail");
        goto Exit0;
    }

    r = msm::ServiceMgr::Ins().ReadSync(path, buf, bufSize, 0);
    if (r < bufSize) {
        LOG_ERROR("on load scene config, read result: %d, buf size: %d", r, bufSize);
        goto Exit0;
    }

    _ParseSceneConfig(buf, bufSize);

Exit0:
    if (buf) {
        free(buf);
    }
    return true;
}

bool _GetSceneBaseInfo(const cJSON* scene, SceneConfig* config) {
    bool ret = false;
    
    const cJSON* id = nullptr;
    const cJSON* length = nullptr;
    const cJSON* width = nullptr;

    id = cJSON_GetObjectItemCaseSensitive(scene, "id");
    if (!cJSON_IsNumber(id)) {
        LOG_ERROR("invalid data, id field is not number");
        goto Exit0;
    }
    config->id = id->valueint;

    length = cJSON_GetObjectItemCaseSensitive(scene, "length");
    if (!cJSON_IsNumber(length)) {
        LOG_ERROR("invalid data, length field is not number");
        goto Exit0;
    }
    config->length = length->valueint;

    width = cJSON_GetObjectItemCaseSensitive(scene, "width");
    if (!cJSON_IsNumber(width)) {
        LOG_ERROR("invalid data, width field is not number");
        goto Exit0;
    }
    config->width = width->valueint;

    ret = true;
Exit0:
    return ret;
}

bool _GetSceneRoleInitStatus(const cJSON* scene, SceneConfig* config) {
    bool ret = false;

    const cJSON* role  = nullptr;
    const cJSON* pos   = nullptr;
    const cJSON* rot   = nullptr;
    const cJSON* speed = nullptr;
    const cJSON* x     = nullptr;
    const cJSON* y     = nullptr;

    role = cJSON_GetObjectItemCaseSensitive(scene, "role");
    if (!cJSON_IsObject(role)) {
        LOG_ERROR("invalid data, role field is not obj");
        goto Exit0;
    }

    pos = cJSON_GetObjectItemCaseSensitive(role, "pos");
    if (!cJSON_IsObject(pos)) {
        LOG_ERROR("invalid data, pos field is not obj");
        goto Exit0;
    }
    x = cJSON_GetObjectItemCaseSensitive(pos, "x");
    y = cJSON_GetObjectItemCaseSensitive(pos, "y");
    if (!cJSON_IsNumber(x) ||
        !cJSON_IsNumber(y)) {
        LOG_ERROR("invalid data, x or y field is not number");
        goto Exit0;
    }
    config->roleInitStatus.trans.pos.x = x->valueint;
    config->roleInitStatus.trans.pos.y = y->valueint;

    rot = cJSON_GetObjectItemCaseSensitive(role, "rot");
    if (!cJSON_IsObject(rot)) {
        LOG_ERROR("invalid data, rot field is not obj");
        goto Exit0;
    }
    x = cJSON_GetObjectItemCaseSensitive(rot, "x");
    y = cJSON_GetObjectItemCaseSensitive(rot, "y");
    if (!cJSON_IsNumber(x) ||
        !cJSON_IsNumber(y)) {
        LOG_ERROR("invalid data, x or y field is not number");
        goto Exit0;
    }
    config->roleInitStatus.trans.rot.x = x->valueint;
    config->roleInitStatus.trans.rot.y = y->valueint;

    speed = cJSON_GetObjectItemCaseSensitive(role, "speed");
    if (!cJSON_IsNumber(speed)) {
        LOG_ERROR("invalid data, speed field is not number");
        goto Exit0;
    }
    config->roleInitStatus.speed = speed->valueint;

    ret = true;
Exit0:
    return ret;
}

bool _GetSceneCubeInfo(const cJSON* scene, SceneConfig* config) {
    bool ret = false;

    int cubeNum;
    const cJSON* cubeArray = nullptr;
    const cJSON* readCube = nullptr;
    const cJSON* id = nullptr;
    const cJSON* position = nullptr;
    const cJSON* rotation = nullptr;
    const cJSON* scale = nullptr;
    const cJSON* x = nullptr;
    const cJSON* y = nullptr;

    cubeArray = cJSON_GetObjectItemCaseSensitive(scene, "cubes");
    if (!cJSON_IsArray(cubeArray)) {
        goto Exit0;
    }

    cubeNum = cJSON_GetArraySize(cubeArray);
    for (int i = 0; i < cubeNum; i++) {
        readCube = cJSON_GetArrayItem(cubeArray, i);
        CubeInfo cube;

        //获取cubeID
        id = cJSON_GetObjectItemCaseSensitive(readCube, "id");
        if (!cJSON_IsNumber(id)) {
            LOG_ERROR("invalid data, id field is not number, on index: %d\n", i);
            goto Exit0;
        }
        cube.id = id->valueint;

        //获取位置
        position = cJSON_GetObjectItemCaseSensitive(readCube, "position");
        if (!position) {
            LOG_ERROR("invalid data, no position field, on index: %d\n", i);
            goto Exit0;
        }
        x = cJSON_GetObjectItemCaseSensitive(position, "x");
        y = cJSON_GetObjectItemCaseSensitive(position, "y");
        if (!cJSON_IsNumber(x) || !cJSON_IsNumber(y)) {
            LOG_ERROR("invalid data, x or y field is not number, on index: %d\n", i);
            goto Exit0;
        }
        cube.trans.pos = { x->valuedouble, y->valuedouble };

        //获取旋转
        rotation = cJSON_GetObjectItemCaseSensitive(readCube, "rotation");
        if (!rotation) {
            LOG_ERROR("invalid data, no rotation field, on index: %d\n", i);
            goto Exit0;
        }
        x = cJSON_GetObjectItemCaseSensitive(rotation, "x");
        y = cJSON_GetObjectItemCaseSensitive(rotation, "y");
        if (!cJSON_IsNumber(x) || !cJSON_IsNumber(y)) {
            LOG_ERROR("invalid data, x or y field is not number, on index: %d\n", i);
            goto Exit0;
        }
        cube.trans.rot = { x->valuedouble, y->valuedouble };

        //获取长宽
        scale = cJSON_GetObjectItemCaseSensitive(readCube, "scale");
        if (!scale) {
            LOG_ERROR("invalid data, no scale field, on index: %d\n", i);
            goto Exit0;
        }
        x = cJSON_GetObjectItemCaseSensitive(scale, "x");
        y = cJSON_GetObjectItemCaseSensitive(scale, "y");
        if (!cJSON_IsNumber(x) || !cJSON_IsNumber(y)) {
            LOG_ERROR("invalid data, x or y field is not number, on index: %d\n", i);
            goto Exit0;
        }
        cube.trans.scale = { x->valuedouble, y->valuedouble };

        config->cubes.push_back(move(cube));
    }

    ret = true;
Exit0:
    return ret;
}

bool SceneMgr::_ParseSceneConfig(const char* data, int len) {
    bool ret = false;
    
    int sceneNum;
    const cJSON* sceneArray = nullptr;

    cJSON* jsonData = cJSON_ParseWithLength(data, len);
    if (jsonData == NULL) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            LOG_ERROR("parse json error: %s", error_ptr);
        }
        goto Exit0;
    }

    sceneArray = cJSON_GetObjectItemCaseSensitive(jsonData, "scenes");
    if (!cJSON_IsArray(sceneArray)) {
        LOG_ERROR("invalid data, scenes field is not array");
        goto Exit0;
    }
    sceneNum = cJSON_GetArraySize(sceneArray);
    for (int i = 0; i < sceneNum; i++) {
        SceneConfig config;
        const cJSON* addScene = nullptr;
        Scene* scene = nullptr;

        addScene = cJSON_GetArrayItem(sceneArray, i);

        if (!_GetSceneBaseInfo(addScene, &config)) {
            LOG_ERROR("get scene base info fail");
            goto Exit0;
        }

        if (!_GetSceneRoleInitStatus(addScene, &config)) {
            LOG_ERROR("get scene role init status fail");
            goto Exit0;
        }

        if (!_GetSceneCubeInfo(addScene, &config)) {
            LOG_ERROR("get scene cube info fail");
            goto Exit0;
        }

        scene = new Scene;
        scene->Init(&config);
        m_scenes.insert(pair<int, Scene*>(scene->m_id, scene));
    }

    ret = true;
Exit0:
    return ret;
}
