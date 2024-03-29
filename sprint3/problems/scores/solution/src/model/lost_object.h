#pragma once

#include "model/units.h"
#include "utils/tagged.h"

namespace model {

class LostObject {
  public:
    using Id = utils::Tagged<size_t, LostObject>;

    LostObject(
        Point position,
        size_t type,
        size_t value,
        double width = 0.0
    ) noexcept :
        id_ {Id(free_id_++)},
        position_ {position},
        type_(type),
        value_(value) {}

    const Id& GetId() const noexcept {
        return id_;
    }

    Point GetPosition() const noexcept {
        return position_;
    }

    size_t GetType() const noexcept {
        return type_;
    }

    bool IsPickedUp() const {
        return picked_up_;
    }

    void SetPickedUp() {
        picked_up_ = true;
    }

    size_t GetValue() const {
        return value_;
    }

    double GetWidth() const {
        return width_;
    }

  private:
    inline static size_t free_id_ = 0;

    Id id_;
    Point position_;
    size_t type_;
    size_t value_;
    double width_;
    bool picked_up_ = false;
};

}  // namespace model
