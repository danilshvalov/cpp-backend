#pragma once

#include <utility>

namespace model {

struct Position {
    double x;
    double y;
};

struct Speed {
    double x;
    double y;
};

enum Direction { NORTH, SOUTH, WEST, EAST };

class Dog {
  public:
    Dog(Position position, Speed speed, Direction direction) :
        position_(std::move(position)),
        speed_(std::move(speed)),
        direction_(std::move(direction)) {}

    Position GetPosition() const {
        return position_;
    }

    Speed GetSpeed() const {
        return speed_;
    }

    Direction GetDirection() const {
        return direction_;
    }

  private:
    Position position_;
    Speed speed_;
    Direction direction_;
};

}  // namespace model
