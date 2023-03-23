#pragma once

#include "model/units.h"

namespace serde::archive {

class SpeedRepr {
  public:
    explicit SpeedRepr() = default;

    explicit SpeedRepr(const model::Speed& speed) : x_(speed.x), y_(speed.y) {}

    [[nodiscard]] model::Speed Restore() const {
        return model::Speed(x_, y_);
    }

    template <typename Archive>
    void serialize(Archive& ar, [[maybe_unused]] const unsigned version) {
        ar& x_;
        ar& y_;
    }

  private:
    double x_;
    double y_;
};

} // namespace serde::archive
