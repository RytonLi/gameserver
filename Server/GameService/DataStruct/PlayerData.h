#pragma once

#include <unordered_map>

#include "Utils/Define.h"

#define EPS 1e-6

GAME_SERVICE_NS_BEGIN

struct BagItem {
	int id;
	int num;
};

struct BuyLimit {
    int id;
    int num;
};

struct RoleData {
    int         id;
	std::string name;
	int         level;
	int         type;
	int         money;

	std::unordered_map<int, BagItem> bagItems;
    std::unordered_map<int, BuyLimit> buyLimits;

    RoleData() = default;

    RoleData(int id) : 
        id(id), 
        name(), 
        level(1), 
        type(0), 
        money(10000),
        bagItems(),
        buyLimits() {}
};

struct PlayerData {
	std::string playerID;
	std::string password;
	std::unordered_map<int, RoleData> roleDatas;

    PlayerData() = default;
};

struct Position {
    double x;
    double y;

    bool operator==(const Position& other) {
        if (x - other.x < EPS && 
            y - other.y < EPS) {
            return true;
        }
        return false;
    }

    bool operator!=(const Position& other) {
        if (x - other.x < EPS &&
            y - other.y < EPS) {
            return false;
        }
        return true;
    }
};

struct Rotation {
    double x;
    double y;

    bool operator==(const Rotation& other) {
        if (x - other.x < EPS &&
            y - other.y < EPS) {
            return true;
        }
        return false;
    }

    bool operator!=(const Rotation& other) {
        if (x - other.x < EPS &&
            y - other.y < EPS) {
            return false;
        }
        return true;
    }
};

struct Scale {
    double x;
    double y;

    bool operator==(const Scale& other) {
        if (x - other.x < EPS &&
            y - other.y < EPS) {
            return true;
        }
        return false;
    }

    bool operator!=(const Scale& other) {
        if (x - other.x < EPS &&
            y - other.y < EPS) {
            return false;
        }
        return true;
    }
};

struct Transform {
    Position pos;
    Rotation rot;
    Scale    scale;
};

GAME_SERVICE_NS_END