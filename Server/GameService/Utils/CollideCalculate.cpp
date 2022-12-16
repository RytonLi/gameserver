#include "CollideCalculate.h"
#include <cmath>

using namespace gs;

Position gs::CoordinateTransform(Position origin, Position target, double cos, double sin) {
    Position res;

    res.x = (cos * (origin.x - target.x) + sin * (origin.y - target.y)) / (2 * sin * cos);

    res.y = (origin.y - target.y - res.x * sin) / cos;

    return res;
}

bool gs::IsCircleIntersectRectangle(Position center, double radius, double length, double width) {
    if (center.x < 0) {
        center.x = -center.x;
    }
    if (center.y < 0) {
        center.y = -center.y;
    }

    double w1 = length / 2;
    double w2 = center.x;
    double h1 = width / 2;
    double h2 = center.y;

    if (w2 > w1 + radius) {
        return false;
    }
    if (h2 > h1 + radius) {
        return false;
    }
    if (w2 <= w1) {
        return true;
    }
    if (h2 <= h1) {
        return true;
    }
    
    return (w2 - w1) * (w2 - w1) + (h2 - h1) * (h2 - h1) <= radius * radius;
}

bool gs::IsCircleIntersectRectangle(Position center1, double radius1, Position center2, double radius2) {
    double d = sqrt((center2.x - center1.x) * (center2.x - center1.x) + (center2.y - center1.y) * (center2.y - center1.y));
    
    return radius1 + radius2 <= d;

}