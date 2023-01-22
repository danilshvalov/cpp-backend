#pragma once

#include "model/units.h"
#include "model/road.h"

#include <boost/functional/hash.hpp>

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <chrono>

namespace model {

class RoadManager {
  public:
    using Roads = std::vector<Road>;

    void AddRoad(const Road& road);

    const Roads& GetRoads() const noexcept;

    MovementResult MakeMovement(
        const Position& position,
        const Speed& speed,
        const std::chrono::milliseconds& time_delta
    ) const;

  private:
    struct RoadPixel {
        int64_t x;
        int64_t y;

        auto operator<=>(const RoadPixel&) const = default;

        struct Hasher {
            size_t operator()(const RoadPixel& value) const noexcept {
                return boost::hash_value(std::tie(value.x, value.y));
            }
        };
    };

    using PixelMap = std::
        unordered_map<RoadPixel, std::unordered_set<size_t>, RoadPixel::Hasher>;
    PixelMap roads_pixels_;
    std::vector<Road> roads_;

    RoadPixel FindNewPixel(
        const RoadPixel& start,
        const RoadPixel& end,
        const Speed& speed
    ) const;

    RoadPixel PositionToRoadPixel(const Position& position) const;

    Position RoadPixelToPosition(
        const RoadPixel& pixel,
        const Position& position,
        Direction direction
    ) const;

    Position FindCornerPosition(
        const RoadPixel& pixel,
        const Position& position,
        const Speed& speed
    ) const;

    bool IsCorrectPosition(
        const std::unordered_set<size_t>& roads,
        const Position& position
    ) const;

    bool IsCorrectPosition(const Road& road, const Position& position) const;

    void CopyContent(const Roads& roads);
};

}  // namespace model
