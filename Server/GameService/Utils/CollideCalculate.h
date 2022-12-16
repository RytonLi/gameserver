#pragma once

#include "DataStruct/PlayerData.h"

GAME_SERVICE_NS_BEGIN

Position CoordinateTransform(Position origin, Position target, double cos, double sin);

bool IsCircleIntersectRectangle(Position center, double radius, double length, double width);

bool IsCircleIntersectRectangle(Position center1, double radius1, Position center2, double radius2);

GAME_SERVICE_NS_END