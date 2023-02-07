#pragma once

#include "model/loot_generator.h"
#include "model/map.h"
#include "model/lost_object.h"
#include "model/item_dog_provider.h"
#include "datetime/ticker.h"
#include "datetime/consts.h"
#include "utils/tagged.h"
#include "utils/random.h"

#include <boost/asio/strand.hpp>
#include <boost/asio/io_context.hpp>

#include <optional>
#include <chrono>
#include <deque>
#include <memory>

namespace app {

namespace net = boost::asio;

class GameSession : public std::enable_shared_from_this<GameSession> {
  public:
    using Id = utils::Tagged<std::string, GameSession>;
    using LostObjects = std::vector<model::LostObject>;
    using Strand = net::strand<net::io_context::executor_type>;

    GameSession(
        net::io_context& io,
        const model::Map& map,
        model::LootGenerator& loot_generator
    ) :
        strand_(net::make_strand(io)),
        id_(*map.GetId()),
        map_(map),
        loot_generator_(loot_generator) {
        loot_ticker_ = std::make_shared<datetime::Ticker>(
            strand_,
            loot_generator_.GetInterval(),
            [this](const auto& dt) { GenerateLoot(dt); }
        );
        loot_ticker_->Start();
    }

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

        auto& dog = dogs_.emplace_back(
            start_position,
            model::Speed(0, 0),
            model::Direction::NORTH,
            map_.GetConfig().bag_capacity
        );

        net::dispatch(strand_, [self = shared_from_this()] {
            self->GenerateLoot(self->loot_generator_.GetInterval());
        });

        return &dog;
    }

    void UpdateGameState(const std::chrono::milliseconds& time_delta) {
        for (auto& dog : dogs_) {
            const auto& position = dog.GetPosition();
            const auto& speed = dog.GetSpeed();

            model::Point new_position = {
                position.x +
                    speed.x * time_delta.count() /
                        datetime::milliseconds_in_second,
                position.y +
                    speed.y * time_delta.count() /
                        datetime::milliseconds_in_second,
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

    const LostObjects& GetLostObjects() const {
        return lost_objects_;
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

    model::LostObject MakeLostObject() const {
        return model::LostObject {
            MakeRandomPosition(),
            utils::GenerateRandomNumber<size_t>(
                0,
                map_.GetLootTypes().size() - 1
            )};
    }

    void GenerateLoot(const model::LootGenerator::TimeInterval& dt) {
        size_t generated_count =
            loot_generator_.Generate(dt, lost_objects_.size(), dogs_.size());
        for (size_t i = 0; i < generated_count; ++i) {
            lost_objects_.push_back(MakeLostObject());
        }
    }

    void ProcessLoot() {
        using namespace model;
        // TODO: move out
        constexpr double lost_objects_width = 0.0;
        constexpr double dog_width = 0.6;
        constexpr double office_width = 0.6;

        const auto& offices = map_.GetOffices();

        ItemDogProvider::Items items;
        items.reserve(lost_objects_.size() + offices.size());

        for (const auto& obj : lost_objects_) {
            items.emplace_back(obj.GetPosition(), lost_objects_width);
        }

        size_t offices_start = items.size();
        for (const auto& office : offices) {
            items.emplace_back(office.GetPosition(), office_width);
        }

        ItemDogProvider::Gatherers gatherers;
        gatherers.reserve(dogs_.size());
        for (const auto& dog : dogs_) {
            gatherers.emplace_back(
                dog.GetPrevPosition(),
                dog.GetPosition(),
                dog_width
            );
        }

        auto events = physics::FindGatherEvents(
            ItemDogProvider(std::move(items), std::move(gatherers))
        );

        if (events.empty()) {
            return;
        }

        for (const auto& event : events) {
            auto& bag = dogs_[event.gatherer_id].GetBag();

            if (event.item_id < offices_start) {
                auto& obj = lost_objects_[event.item_id];
                if (bag.IsFull() || obj.IsPickedUp()) {
                    continue;
                }

                obj.SetPickedUp();
                bag.Add(obj);
            } else {
                if (bag.IsEmpty()) {
                    continue;
                }
                bag.Drop();
            }
        }

        std::erase_if(lost_objects_, [](const auto& obj) {
            return obj.IsPickedUp();
        });
    }

    Strand strand_;
    Id id_;
    std::deque<model::Dog> dogs_;
    const model::Map& map_;
    model::LootGenerator& loot_generator_;
    std::shared_ptr<datetime::Ticker> loot_ticker_;
    LostObjects lost_objects_;
};

using GameSessionHolder = std::shared_ptr<GameSession>;

}  // namespace app
