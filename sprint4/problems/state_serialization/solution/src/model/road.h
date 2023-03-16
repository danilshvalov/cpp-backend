#pragma once

#include "model/units.h"

#include <algorithm>

namespace model {

class Road {
    struct HorizontalTag {
        explicit HorizontalTag() = default;
    };

    struct VerticalTag {
        explicit VerticalTag() = default;
    };

  public:
    constexpr static HorizontalTag HORIZONTAL {};
    constexpr static VerticalTag VERTICAL {};

    Road(
        HorizontalTag,
        Point start,
        Coord end_x,
        Dimension width = 0.4
    ) noexcept :
        Road(start, {end_x, start.y}, width) {}

    Road(VerticalTag, Point start, Coord end_y, Dimension width = 0.4) noexcept
        :
        Road(start, {start.x, end_y}, width) {}

    Road(Point start, Point end, Dimension width = 0.4) noexcept :
        start_(start),
        end_(end),
        width_(width) {}

    Point GetLeftBottomCorner() const noexcept {
        Dimension x = std::min(start_.x, end_.x) - width_;
        Dimension y = std::min(start_.y, end_.y) - width_;

        return Point {x, y};
    }

    Point GetRightTopCorner() const noexcept {
        Dimension x = std::max(start_.x, end_.x) + width_;
        Dimension y = std::max(start_.y, end_.y) + width_;
        return Point {x, y};
    }

    Point Bound(const Point& point) const noexcept {
        const Point left_bottom = GetLeftBottomCorner();
        const Point right_top = GetRightTopCorner();

        return Point {
            std::clamp(point.x, left_bottom.x, right_top.x),
            std::clamp(point.y, left_bottom.y, right_top.y),
        };
    }

    bool Contains(const Point& point) const noexcept {
        return GetLeftBottomCorner() <= point && point <= GetRightTopCorner();
    }

    bool IsHorizontal() const noexcept {
        return start_.y == end_.y;
    }

    bool IsVertical() const noexcept {
        return start_.x == end_.x;
    }

    Point GetStart() const noexcept {
        return start_;
    }

    Point GetEnd() const noexcept {
        return end_;
    }

    Dimension GetWidth() const noexcept {
        return width_;
    }

  private:
    Point start_;
    Point end_;
    Dimension width_;
};

}  // namespace model
