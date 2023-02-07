#pragma once

#include "model/units.h"
#include "model/lost_object.h"
#include "model/lost_objects_bag.h"

#include <utility>
#include <vector>

namespace model {

class Dog {
  public:
    Dog(Point position, Speed speed, Direction direction, size_t bag_capacity) :
        position_(std::move(position)),
        prev_position_(position_),
        speed_(std::move(speed)),
        direction_(std::move(direction)),
        bag_(bag_capacity) {}

    Point GetPosition() const {
        return position_;
    }

    Point GetPrevPosition() const {
        return prev_position_;
    }

    void SetPosition(Point position) {
        prev_position_ = std::move(position_);
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

    const LostObjectsBag& GetBag() const {
        return bag_;
    }

    LostObjectsBag& GetBag() {
        return bag_;
    }

    size_t GetScore() const {
        return score_;
    }

    void SetScore(size_t score) {
        score_ = score;
    }

  private:
    Point position_;
    Point prev_position_;
    Speed speed_;
    Direction direction_;
    LostObjectsBag bag_;
    size_t score_ = 0;
};

}  // namespace model
