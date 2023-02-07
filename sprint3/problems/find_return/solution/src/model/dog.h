#pragma once

#include "model/units.h"
#include "model/lost_object.h"

#include <utility>
#include <vector>

namespace model {

// TODO: move out
class LostObjectsBag {
  public:
    using LostObjects = std::vector<LostObject>;

    LostObjectsBag(size_t capacity) : capacity_(capacity) {
        lost_objects_.reserve(capacity_);
    }

    bool IsFull() const {
        return lost_objects_.size() == capacity_;
    }

    bool IsEmpty() const {
        return lost_objects_.empty();
    }

    void Add(LostObject lost_object) {
        if (IsFull()) {
            throw std::runtime_error("Adding an object to an overflowing bag");
        }

        lost_objects_.push_back(std::move(lost_object));
    }

    void Drop() {
        lost_objects_.clear();
    }

    size_t Capacity() const {
        return capacity_;
    }

    size_t Size() const {
        return lost_objects_.size();
    }

    using iterator = typename LostObjects::iterator;
    using const_iterator = typename LostObjects::const_iterator;

    iterator begin() {
        return lost_objects_.begin();
    }

    iterator end() {
        return lost_objects_.end();
    }

    const_iterator begin() const {
        return lost_objects_.begin();
    }

    const_iterator end() const {
        return lost_objects_.end();
    }

  private:
    LostObjects lost_objects_;
    size_t capacity_;
};

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

  private:
    Point position_;
    Point prev_position_;
    Speed speed_;
    Direction direction_;
    LostObjectsBag bag_;
};

}  // namespace model
