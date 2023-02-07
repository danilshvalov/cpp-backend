#include "model/units.h"

namespace model {
Dimension FindDistance(const Point& lhs, const Point& rhs) {
    Dimension distance =
        std::pow(rhs.x - lhs.x, 2) + std::pow(rhs.y - lhs.y, 2);
    return std::sqrt(distance);
}

Direction GetDirectionFromSpeed(const Speed& speed) {
    if (speed.x == 0 && speed.y < 0) {
        return Direction::NORTH;
    } else if (speed.x == 0 && speed.y > 0) {
        return Direction::SOUTH;
    } else if (speed.x > 0 && speed.y == 0) {
        return Direction::EAST;
    } else if (speed.x < 0 && speed.y == 0) {
        return Direction::WEST;
    } else if (speed.x == 0 && speed.y == 0) {
        return Direction::NONE;
    } else {
        throw std::logic_error("Incorrect speed values");
    }
}

Direction GetOppositeDirection(Direction direction) {
    switch (direction) {
        case Direction::NORTH:
            return Direction::SOUTH;
        case Direction::SOUTH:
            return Direction::NORTH;
        case Direction::WEST:
            return Direction::EAST;
        case Direction::EAST:
            return Direction::WEST;
        case Direction::NONE:
            return Direction::NONE;
        default:
            throw std::runtime_error("Incorrect direction");
    }
}
}  // namespace model
