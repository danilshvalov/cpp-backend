#pragma once

#include "model/map.h"
#include "utils/tagged.h"
#include "utils/random.h"

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

    model::Dog* CreateDog() {
        return &dogs_.emplace_back(model::Dog(
            MakeRandomPosition(),
            model::Speed(0, 0),
            model::Direction::NORTH
        ));
    }

  private:
    model::Position MakeRandomPosition() {
        const auto& roads = map_.GetRoads();
        size_t road_index =
            utils::GenerateRandomNumber<size_t>(0, roads.size() - 1);
        const auto& road = roads[road_index];

        if (road.IsHorizontal()) {
            return model::Position {
                .x = utils::GenerateRandomNumber<double>(
                    road.GetStart().x,
                    road.GetEnd().x
                ),
                .y = static_cast<double>(road.GetStart().y),
            };
        } else {
            return model::Position {
                .x = static_cast<double>(road.GetStart().x),
                .y = utils::GenerateRandomNumber<double>(
                    road.GetStart().y,
                    road.GetEnd().y
                ),
            };
        }
    }

    Id id_;
    std::vector<model::Dog> dogs_;
    const model::Map& map_;
};

// TODO:
using GameSessionIdHasher = utils::TaggedHasher<GameSession::Id>;

}  // namespace app
