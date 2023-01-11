#pragma once

#include <utility>

namespace model {

enum Direction {
    NORTH,
    SOUTH,
    WEST,
    EAST,
    NONE,
};

struct Position {
    double x;
    double y;
};

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

class Dog {
  public:
    Dog(Position position, Speed speed, Direction direction) :
        position_(std::move(position)),
        speed_(std::move(speed)),
        direction_(std::move(direction)) {}

    Position GetPosition() const {
        return position_;
    }

    void SetPosition(Position position) {
        position_ = std::move(position);
    }

    Speed GetSpeed() const {
        return speed_;
    }

    void SetSpeed(Speed speed) {
        speed_ = std::move(speed);
    }

    Direction GetDirection() const {
        return direction_;
    }

    void SetDirection(Direction direction) {
        direction_ = std::move(direction);
    }

  private:
    Position position_;
    Speed speed_;
    Direction direction_;
};

}  // namespace model
