#pragma once

#include "model/units.h"
#include "model/physics/collision.h"
#include "utils/tagged.h"

#include <string>

namespace model {

class Office {
  public:
    using Id = utils::Tagged<std::string, Office>;

    Office(Id id, Point position, Offset offset, double width = 0.5) noexcept :
        id_(id),
        position_(position),
        offset_(offset),
        width_(width) {}

    const Id& GetId() const noexcept {
        return id_;
    }

    Point GetPosition() const noexcept {
        return position_;
    }

    Offset GetOffset() const noexcept {
        return offset_;
    }

    double GetWidth() const noexcept {
        return width_;
    }

  private:
    Id id_;
    Point position_;
    Offset offset_;
    double width_;
};

}  // namespace model
