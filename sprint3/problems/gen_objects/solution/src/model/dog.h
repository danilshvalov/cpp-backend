#pragma once

#include "model/units.h"

#include <utility>

namespace model {

class Dog {
  public:
    Dog(Point position, Speed speed, Direction direction) :
        position_(std::move(position)),
        speed_(std::move(speed)),
        direction_(std::move(direction)) {}

    Point GetPosition() const {
        return position_;
    }

    void SetPosition(Point position) {
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
    Point position_;
    Speed speed_;
    Direction direction_;
};

}  // namespace model
