#include "AOI.h"
#include <algorithm>
#include "Entity.h"
#include "Scene.h"

using namespace gs;
using namespace std;

bool AOI::Update(int desGridIndex) {
    vector<vector<int>> aoi;

    _GetChangeAOIs(desGridIndex, aoi);
    // 新增的grid
    m_entity->m_scene->ChangeEntityEnterSight(aoi[(int)AOI_TYPE::emAOIIncrease], m_entity);
    // 减少的grid
    m_entity->m_scene->ChangeEntityExitSight(aoi[(int)AOI_TYPE::emAOIDecrease], m_entity);

    m_entity->m_scene->BroadcastEnter(aoi[emAOIIncrease], m_entity);
    m_entity->m_scene->BroadcastExit(aoi[emAOIDecrease], m_entity);
    m_entity->m_scene->BroadcastMove(aoi[emAOIUnChange], m_entity);

    // 更新aoi
    m_centerGridIndex = desGridIndex;
    m_aoiGridIndexs.clear();
    _GetAOIByIndex(m_centerGridIndex, m_aoiGridIndexs);
    
    return true;
}

const std::vector<int>& AOI::GetAOI() {
    return m_aoiGridIndexs;
}

void AOI::_GetChangeAOIs(int desGridIndex, vector<vector<int>>& output) {
    output.resize(3);
    vector<int>& incrAOI = output[AOI_TYPE::emAOIIncrease];
    vector<int>& decrAOI = output[AOI_TYPE::emAOIDecrease];
    vector<int>& unchangeAOI = output[AOI_TYPE::emAOIUnChange];

    int oldGridIndex = m_centerGridIndex;
    vector<int> oldAOI;
    if (oldGridIndex >= 0) {
        _GetAOIByIndex(oldGridIndex, oldAOI);
    }

    vector<int> newAOI;
    if (desGridIndex >= 0) {
        _GetAOIByIndex(desGridIndex, newAOI);
    }

    sort(oldAOI.begin(), oldAOI.end());
    sort(newAOI.begin(), newAOI.end());

    int indexOld = 0;
    int indexNew = 0;
    while (indexOld < oldAOI.size() && indexNew < newAOI.size()) {
        // 两个AOI的交集, 即不变的AOI区域
        if (oldAOI[indexOld] == newAOI[indexNew]) {
            unchangeAOI.push_back(newAOI[indexNew]);
            ++indexOld;
            ++indexNew;
        }
        // newAOI独特的区域，即新增的AOI
        else if (oldAOI[indexOld] > newAOI[indexNew]) {
            incrAOI.push_back(newAOI[indexNew]);
            ++indexNew;
        }
        // oldAOI独特的区域，即减少的AOI
        else if (oldAOI[indexOld] < newAOI[indexNew]) {
            decrAOI.push_back(oldAOI[indexOld]);
            ++indexOld;
        }
    }
    // 将剩余部分加入对应数组
    while (indexNew < newAOI.size()) {
        incrAOI.push_back(newAOI[indexNew]);
        ++indexNew;
    }
    while (indexOld < oldAOI.size()) {
        decrAOI.push_back(oldAOI[indexOld]);
        ++indexOld;
    }
    return;
}

void AOI::_GetAOIByIndex(int gridIndex, vector<int>& output) {
    // 视野方向: 从右方逆时针旋转，共八个方向
    int col = m_entity->m_scene->GetCol();
    int row = m_entity->m_scene->GetRow();

    bool firstRow = gridIndex < col;
    bool lastRow = gridIndex + col >= col * row;
    bool firstCol = gridIndex % col == 0;
    bool lastCol = gridIndex % col == col - 1;

    output.clear();
    output.push_back(gridIndex);
    if (!firstRow && !firstCol) {
        output.push_back(gridIndex - col - 1);
    }
    if (!firstRow) {
        output.push_back(gridIndex - col);
    }
    if (!firstRow && !lastCol) {
        output.push_back(gridIndex - col + 1);
    }
    if (!firstCol) {
        output.push_back(gridIndex - 1);
    }
    if (!lastCol) {
        output.push_back(gridIndex + 1);
    }
    if (!lastRow && !firstCol) {
        output.push_back(gridIndex + col - 1);
    }
    if (!lastRow) {
        output.push_back(gridIndex + col);
    }
    if (!lastRow && !lastCol) {
        output.push_back(gridIndex + col + 1);
    }

    return;
    return;
}