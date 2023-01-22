#pragma once

#include <stdexcept>
#include <cmath>

namespace model {

using Dimension = double;
using Coord = Dimension;

struct Point {
    Coord x;
    Coord y;

    bool operator<=(const Point& other) const {
        return x <= other.x && y <= other.y;
    }

    bool operator<(const Point& other) const {
        return x < other.x && y < other.y;
    }

    bool operator>=(const Point& other) const {
        return x >= other.x && y >= other.y;
    }

    bool operator>(const Point& other) const {
        return x > other.x && y > other.y;
    }

    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }

    bool operator!=(const Point& other) const {
        return x != other.x || y != other.y;
    }
};

// TODO: remove inline
inline Dimension FindDistance(const Point& lhs, const Point& rhs) {
    Dimension distance =
        std::pow(rhs.x - lhs.x, 2) + std::pow(rhs.y - lhs.y, 2);
    return std::sqrt(distance);
}

struct Size {
    Dimension width;
    Dimension height;
};

struct Rectangle {
    Point position;
    Size size;
};

struct Offset {
    Dimension dx;
    Dimension dy;
};

enum Direction {
    NORTH,
    SOUTH,
    WEST,
    EAST,
    NONE,
};

// TODO: remove inline
inline Direction GetOppositeDirection(Direction direction) {
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
    }
}

// TODO: remove
using Position = Point;

// struct Position {
//     double x;
//     double y;
// };

struct Speed {
    Speed(double x, double y) : x(x), y(y) {};

    Speed(double speed, Direction direction) {
        switch (direction) {
            case Direction::WEST: {
                x = -speed;
                y = 0;
                break;
            }
            case Direction::EAST: {
                x = speed;
                y = 0;
                break;
            }
            case Direction::NORTH: {
                x = 0;
                y = -speed;
                break;
            }
            case Direction::SOUTH: {
                x = 0;
                y = speed;
                break;
            }
            case Direction::NONE: {
                x = 0;
                y = 0;
                break;
            }
        }
    }

    double x;
    double y;
};

// TODO: remove inline
inline Direction GetDirectionFromSpeed(const Speed& speed) {
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
        // TODO: change exception message
        throw std::logic_error("Incorrect speed values");
    }
}

// TODO: remove
struct MovementResult {
    Position position;
    Speed speed;
};

}  // namespace model
