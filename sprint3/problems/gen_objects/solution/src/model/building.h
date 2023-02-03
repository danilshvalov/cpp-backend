#pragma once

#include "model/units.h"

namespace model {

class Building {
  public:
    explicit Building(Rectangle bounds) noexcept : bounds_ {bounds} {}

    const Rectangle& GetBounds() const noexcept {
        return bounds_;
    }

  private:
    Rectangle bounds_;
};

}  // namespace model
