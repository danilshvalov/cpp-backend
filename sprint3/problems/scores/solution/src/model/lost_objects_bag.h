#pragma once

#include "model/lost_object.h"

#include <vector>

namespace model {
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

    size_t Drop() {
        size_t score = 0;
        for (const auto& obj : lost_objects_) {
            score += obj.GetValue();
        }

        lost_objects_.clear();
        return score;
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
}  // namespace model
