#pragma once

#include "model/units.h"

namespace serde::archive {

class PointRepr {
  public:
    explicit PointRepr() = default;

    explicit PointRepr(const model::Point& point) : x_(point.x), y_(point.y) {}

    [[nodiscard]] model::Point Restore() const {
        return model::Point{x_, y_};
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
