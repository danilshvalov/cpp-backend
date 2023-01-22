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

Dimension FindDistance(const Point& lhs, const Point& rhs);

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

Direction GetOppositeDirection(Direction direction);

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

Direction GetDirectionFromSpeed(const Speed& speed);

}  // namespace model
