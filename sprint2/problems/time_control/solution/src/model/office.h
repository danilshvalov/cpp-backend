#pragma once

#include "model/units.h"
#include "utils/tagged.h"

#include <string>

namespace model {

class Office {
  public:
    using Id = utils::Tagged<std::string, Office>;

    Office(Id id, Point position, Offset offset) noexcept :
        id_ {std::move(id)},
        position_ {position},
        offset_ {offset} {}

    const Id& GetId() const noexcept {
        return id_;
    }

    Point GetPosition() const noexcept {
        return position_;
    }

    Offset GetOffset() const noexcept {
        return offset_;
    }

  private:
    Id id_;
    Point position_;
    Offset offset_;
};

}  // namespace model
