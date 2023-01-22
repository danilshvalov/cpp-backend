#include "model/road_manager.h"

#include "utils/algorithm.h"

#include <algorithm>
#include <limits>

namespace model {

namespace consts {
    constexpr int64_t pixel_size = 10;
    constexpr double road_offset = 0.4;
}  // namespace consts

void RoadManager::AddRoad(const Road& road) {
    const int64_t offset = consts::road_offset * consts::pixel_size;
    const size_t index = roads_.size();
    roads_.emplace_back(road);

    int64_t start, end;
    if (road.IsHorizontal()) {
        std::tie(start, end) = std::minmax(road.GetStart().x, road.GetEnd().x);
    } else {
        std::tie(start, end) = std::minmax(road.GetStart().y, road.GetEnd().y);
    }

    start = start * consts::pixel_size - offset;
    end = end * consts::pixel_size + offset;

    if (road.IsHorizontal()) {
        int64_t y = road.GetStart().y * consts::pixel_size;
        for (int64_t x = start; x <= end; ++x) {
            for (int64_t i = -offset; i <= offset; ++i) {
                roads_pixels_[{x, y + i}].insert(index);
            }
        }
    } else {
        int64_t x = road.GetStart().x * consts::pixel_size;
        for (int64_t y = start; y <= end; ++y) {
            for (int64_t i = -offset; i <= offset; ++i) {
                roads_pixels_[{x + i, y}].insert(index);
            }
        }
    }
};

const RoadManager::Roads& RoadManager::GetRoads() const noexcept {
    return roads_;
};

MovementResult RoadManager::MakeMovement(
    const Position& position,
    const Speed& speed,
    const std::chrono::milliseconds& time_delta
) const {
    if (speed.x == 0 && speed.y == 0) {
        return {position, speed};
    }

    model::Position new_position = {
        position.x + speed.x * time_delta.count() / 1000.0,
        position.y + speed.y * time_delta.count() / 1000.0,
    };

    RoadPixel start = PositionToRoadPixel(position);
    RoadPixel end = PositionToRoadPixel(new_position);
    if (!roads_pixels_.contains(end) ||
        !IsCorrectPosition(roads_pixels_.at(end), new_position)) {
        end.x = std::signbit(speed.x)
            ? -consts::road_offset * consts::pixel_size - 1
            : std::numeric_limits<std::int64_t>::max();
        end.y = std::signbit(speed.y)
            ? -consts::road_offset * consts::pixel_size - 1
            : std::numeric_limits<std::int64_t>::max();
    }

    auto dest = FindNewPixel(start, end, speed);
    if (IsCorrectPosition(roads_pixels_.at(dest), new_position)) {
        return {new_position, speed};
    } else {
        return {
            FindCornerPosition(dest, new_position, speed),
            Speed(0, 0),
        };
    }
};

RoadManager::RoadPixel RoadManager::FindNewPixel(
    const RoadPixel& start,
    const RoadPixel& end,
    const Speed& speed
) const {
    RoadPixel pixel = start;
    if (speed.x != 0) {
        int64_t direction = std::signbit(speed.x) ? -1 : 1;
        for (int64_t x = start.x; x != end.x &&
             roads_pixels_.contains({x, start.y}) &&
             utils::IsIntersect(roads_pixels_.at(start),
                                roads_pixels_.at({x, start.y}));
             x += direction) {
            pixel = {x, start.y};
        }
        return pixel;
    } else {
        int64_t direction = std::signbit(speed.y) ? -1 : 1;
        for (int64_t y = start.y; y != end.y &&
             roads_pixels_.contains({start.x, y}) &&
             utils::IsIntersect(roads_pixels_.at(start),
                                roads_pixels_.at({start.x, y}));
             y += direction) {
            pixel = {start.x, y};
        }
        return pixel;
    }
};

RoadManager::RoadPixel RoadManager::PositionToRoadPixel(const Position& position
) const {
    return RoadPixel {
        static_cast<int64_t>(position.x * consts::pixel_size),
        static_cast<int64_t>(position.y * consts::pixel_size),
    };
};

Position RoadManager::FindCornerPosition(
    const RoadPixel& pixel,
    const Position& position,
    const Speed& speed
) const {
    const Direction direction = GetDirectionFromSpeed(speed);

    for (auto road_index : roads_pixels_.at(pixel)) {
        if (Position end = RoadPixelToPosition(pixel, position, direction);
            IsCorrectPosition(roads_[road_index], end)) {
            return end;
        }
    }

    return position;
};

Position RoadManager::RoadPixelToPosition(
    const RoadPixel& pixel,
    const Position& old_position,
    Direction direction
) const {
    switch (direction) {
        case Direction::NORTH:
            return Position {
                old_position.x,
                static_cast<double>(pixel.y) / consts::pixel_size,
            };
        case Direction::SOUTH:
            return Position {
                old_position.x,
                static_cast<double>(pixel.y) / consts::pixel_size,
            };
        case Direction::WEST:
            return Position {
                static_cast<double>(pixel.x) / consts::pixel_size,
                old_position.y,
            };
        case Direction::EAST:
            return Position {
                static_cast<double>(pixel.x) / consts::pixel_size,
                old_position.y,
            };
        case Direction::NONE:
            return Position {old_position.x, old_position.y};
    }
}

bool RoadManager::IsCorrectPosition(
    const std::unordered_set<size_t>& roads,
    const Position& position
) const {
    return std::any_of(roads.begin(), roads.end(), [&](size_t road_index) {
        return IsCorrectPosition(roads_[road_index], position);
    });
}

bool RoadManager::IsCorrectPosition(const Road& road, const Position& position)
    const {
    const auto& start_point = road.GetStart();
    const auto& end_point = road.GetEnd();

    Position start, end;

    if (road.IsHorizontal()) {
        std::tie(start.x, end.x) = std::minmax(start_point.x, end_point.x);
        start.x -= consts::road_offset;
        end.x += consts::road_offset;

        start.y = start_point.y - consts::road_offset;
        end.y = start_point.y + consts::road_offset;
    } else {
        std::tie(start.y, end.y) = std::minmax(start_point.y, end_point.y);
        start.y -= consts::road_offset;
        end.y += consts::road_offset;

        start.x = start_point.x - consts::road_offset;
        end.x = start_point.x + consts::road_offset;
    }

    return start.x <= position.x && start.y <= position.y &&
        position.x <= end.x && position.y <= end.y;
};

}  // namespace model
