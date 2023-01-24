#pragma once

#include "model/map.h"
#include "utils/tagged.h"
#include "utils/random.h"

#include <optional>
#include <chrono>
#include <deque>

namespace app {

class GameSession {
  public:
    using Id = utils::Tagged<std::string, GameSession>;

    GameSession(const model::Map& map) : id_(*map.GetId()), map_(map) {}

    const Id& GetId() const {
        return id_;
    }

    const model::Map& GetMap() const {
        return map_;
    }

    model::Dog* CreateDog(bool randomize_spawn_points) {
        model::Point start_position = randomize_spawn_points
            ? MakeRandomPosition()
            : MakeDefaultPosition();

        return &dogs_.emplace_back(
            start_position,
            model::Speed(0, 0),
            model::Direction::NORTH
        );
    }

    void UpdateGameState(const std::chrono::milliseconds& time_delta) {
        for (auto& dog : dogs_) {
            const auto& position = dog.GetPosition();
            const auto& speed = dog.GetSpeed();

            model::Point new_position = {
                position.x + speed.x * time_delta.count() / 1000.0,
                position.y + speed.y * time_delta.count() / 1000.0,
            };

            auto suitable_point = FindSuitablePoint(position, new_position);

            if (suitable_point) {
                dog.SetPosition(*suitable_point);
                if (suitable_point->x != new_position.x ||
                    suitable_point->y != new_position.y) {
                    dog.SetSpeed(model::Speed(0, 0));
                }
            }
        }
    }

  private:
    model::Point MakeRandomPosition() const {
        const auto& roads = map_.GetRoads();
        size_t road_index =
            utils::GenerateRandomNumber<size_t>(0, roads.size() - 1);
        const auto& road = roads[road_index];

        if (road.IsHorizontal()) {
            return model::Point {
                .x = utils::GenerateRandomNumber<double>(
                    road.GetStart().x,
                    road.GetEnd().x
                ),
                .y = static_cast<double>(road.GetStart().y),
            };
        } else {
            return model::Point {
                .x = static_cast<double>(road.GetStart().x),
                .y = utils::GenerateRandomNumber<double>(
                    road.GetStart().y,
                    road.GetEnd().y
                ),
            };
        }
    }

    model::Point MakeDefaultPosition() const {
        const auto& road = map_.GetRoads().front();
        return road.GetStart();
    }

    std::optional<model::Point> FindSuitablePoint(
        const model::Point& start,
        const model::Point& end
    ) const {
        std::vector<model::Road> start_roads;
        for (const auto& road : map_.GetRoads()) {
            if (road.Contains(start)) {
                start_roads.push_back(road);
            }
        }

        if (start_roads.empty()) {
            return std::nullopt;
        }

        model::Point most_far = start_roads.front().Bound(end);

        if (start_roads.size() == 1) {
            return most_far;
        }

        model::Dimension max_distance = FindDistance(start, most_far);

        for (size_t i = 1; i < start_roads.size(); ++i) {
            model::Point pretender = start_roads[i].Bound(end);
            model::Dimension distance = FindDistance(start, pretender);
            if (distance > max_distance) {
                most_far = pretender;
                max_distance = distance;
            }
        }

        return most_far;
    }

    Id id_;
    std::deque<model::Dog> dogs_;
    const model::Map& map_;
};

}  // namespace app
