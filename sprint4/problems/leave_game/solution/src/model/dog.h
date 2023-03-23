#pragma once

#include "model/units.h"
#include "model/lost_object.h"
#include "model/lost_objects_bag.h"

#include <utility>
#include <vector>
#include <memory>
#include <chrono>

namespace model {

class Dog {
  public:
    Dog(Point position, size_t bag_capacity, double width = 0.6) :
        Dog(position, Speed(0, 0), Direction::NORTH, bag_capacity, width) {}

    Dog(Point position, Speed speed, Direction direction, size_t bag_capacity,
        double width = 0.6) :
        position_(std::move(position)),
        prev_position_(position_),
        speed_(std::move(speed)),
        direction_(std::move(direction)),
        bag_(bag_capacity),
        width_(width) {}

    Dog(Point position, Point prev_position, Speed speed, Direction direction,
        LostObjectsBag bag, double width = 0.6, size_t score = 0) :
        position_(std::move(position)),
        prev_position_(prev_position),
        speed_(std::move(speed)),
        direction_(std::move(direction)),
        bag_(std::move(bag)),
        width_(width),
        score_(score) {}

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

    double GetWidth() const {
        return width_;
    }

    const std::chrono::milliseconds& GetLiveTime() const {
        return live_time_;
    }

    void SetLiveTime(std::chrono::milliseconds live_time) {
        live_time_ = std::move(live_time);
    }

    const std::chrono::milliseconds& GetInactiveTime() const {
        return inactive_time_;
    }

    void SetInactiveTime(std::chrono::milliseconds inactive_time) {
        inactive_time_ = std::move(inactive_time);
    }

  private:
    Point position_;
    Point prev_position_;
    Speed speed_;
    Direction direction_;
    LostObjectsBag bag_;
    double width_;
    size_t score_ = 0;
    std::chrono::milliseconds live_time_{0};
    std::chrono::milliseconds inactive_time_{0};
};

using DogHolder = std::shared_ptr<Dog>;

} // namespace model
