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

namespace model {

namespace net = boost::asio;

class GameSession : public std::enable_shared_from_this<GameSession> {
  public:
    using Id = utils::Tagged<std::string, GameSession>;
    using LostObjects = std::vector<LostObject>;
    using Strand = net::strand<net::io_context::executor_type>;

    GameSession(
        net::io_context& io, const Map& map, LootGenerator& loot_generator,
        std::chrono::milliseconds max_inactive_time
    ) :
        strand_(net::make_strand(io)),
        id_(*map.GetId()),
        map_(map),
        loot_generator_(loot_generator),
        max_inactive_time_(std::move(max_inactive_time)) {
        loot_ticker_ = std::make_shared<datetime::Ticker>(
            strand_, loot_generator_.GetInterval(),
            [this](const auto& dt) {
                GenerateLoot(dt);
            }
        );
        loot_ticker_->Start();
    }

    const Id& GetId() const {
        return id_;
    }

    const Map& GetMap() const {
        return map_;
    }

    void AddDog(DogHolder dog) {
        dogs_.push_back(std::move(dog));

        net::dispatch(strand_, [self = shared_from_this()] {
            self->GenerateLoot(self->loot_generator_.GetInterval());
        });
    }

    DogHolder CreateDog(bool randomize_spawn_points) {
        Point start_position = randomize_spawn_points ? MakeRandomPosition()
                                                      : MakeDefaultPosition();

        auto dog = std::make_shared<Dog>(
            start_position, Speed(0, 0), Direction::NORTH,
            map_.GetConfig().bag_capacity
        );
        AddDog(dog);
        return dog;
    }

    void AddLostObject(LostObject lost_object) {
        lost_objects_.push_back(std::move(lost_object));
    }

    void UpdateGameState(const std::chrono::milliseconds& time_delta) {
        for (const auto& dog : dogs_) {
            const auto& position = dog->GetPosition();
            const auto& speed = dog->GetSpeed();

            Point new_position = {
                position.x + speed.x * time_delta.count() /
                                 datetime::milliseconds_in_second,
                position.y + speed.y * time_delta.count() /
                                 datetime::milliseconds_in_second,
            };

            auto suitable_point = FindSuitablePoint(position, new_position);

            if (suitable_point) {
                dog->SetPosition(*suitable_point);
                if (suitable_point->x != new_position.x ||
                    suitable_point->y != new_position.y) {
                    dog->SetSpeed(Speed(0, 0));
                }
            }

            dog->SetLiveTime(dog->GetLiveTime() + time_delta);

            if (speed.x == 0 && speed.y == 0) {
                dog->SetInactiveTime(dog->GetInactiveTime() + time_delta);
            } else {
                dog->SetInactiveTime(std::chrono::milliseconds(0));
            }

            if (dog->GetInactiveTime() >= max_inactive_time_) {
                continue;
            }
        }
        ProcessLoot();
    }

    const std::vector<DogHolder>& GetDogs() const {
        return dogs_;
    }

    const LostObjects& GetLostObjects() const {
        return lost_objects_;
    }

    void RemoveDog(const DogHolder& dog) {
        std::erase(dogs_, dog);
    }

  private:
    Point MakeRandomPosition() const {
        const auto& roads = map_.GetRoads();
        size_t road_index =
            utils::GenerateRandomNumber<size_t>(0, roads.size() - 1);
        const auto& road = roads[road_index];

        if (road.IsHorizontal()) {
            return Point{
                .x = utils::GenerateRandomNumber<double>(
                    road.GetStart().x, road.GetEnd().x
                ),
                .y = static_cast<double>(road.GetStart().y),
            };
        } else {
            return Point{
                .x = static_cast<double>(road.GetStart().x),
                .y = utils::GenerateRandomNumber<double>(
                    road.GetStart().y, road.GetEnd().y
                ),
            };
        }
    }

    Point MakeDefaultPosition() const {
        const auto& road = map_.GetRoads().front();
        return road.GetStart();
    }

    std::optional<Point>
    FindSuitablePoint(const Point& start, const Point& end) const {
        std::vector<Road> start_roads;
        for (const auto& road : map_.GetRoads()) {
            if (road.Contains(start)) {
                start_roads.push_back(road);
            }
        }

        if (start_roads.empty()) {
            return std::nullopt;
        }

        Point most_far = start_roads.front().Bound(end);

        if (start_roads.size() == 1) {
            return most_far;
        }

        Dimension max_distance = FindDistance(start, most_far);

        for (size_t i = 1; i < start_roads.size(); ++i) {
            Point pretender = start_roads[i].Bound(end);
            Dimension distance = FindDistance(start, pretender);
            if (distance > max_distance) {
                most_far = pretender;
                max_distance = distance;
            }
        }

        return most_far;
    }

    LostObject MakeLostObject() const {
        const auto& loot_types = map_.GetLootTypes();

        auto type =
            utils::GenerateRandomNumber<size_t>(0, loot_types.size() - 1);
        return LostObject{
            MakeRandomPosition(),
            type,
            loot_types[type].value,
        };
    }

    void GenerateLoot(const LootGenerator::TimeInterval& dt) {
        size_t generated_count =
            loot_generator_.Generate(dt, lost_objects_.size(), dogs_.size());
        for (size_t i = 0; i < generated_count; ++i) {
            lost_objects_.push_back(MakeLostObject());
        }
    }

    void ProcessLoot() {
        using namespace model;
        const auto& offices = map_.GetOffices();

        ItemDogProvider::Items items;
        items.reserve(lost_objects_.size() + offices.size());

        for (const auto& obj : lost_objects_) {
            items.emplace_back(obj.GetPosition(), obj.GetWidth());
        }

        size_t offices_start = items.size();
        for (const auto& office : offices) {
            items.emplace_back(office.GetPosition(), office.GetWidth());
        }

        ItemDogProvider::Gatherers gatherers;
        gatherers.reserve(dogs_.size());
        for (const auto& dog : dogs_) {
            gatherers.emplace_back(
                dog->GetPrevPosition(), dog->GetPosition(), dog->GetWidth()
            );
        }

        auto events = physics::FindGatherEvents(
            ItemDogProvider(std::move(items), std::move(gatherers))
        );

        if (events.empty()) {
            return;
        }

        for (const auto& event : events) {
            auto& dog = dogs_[event.gatherer_id];
            auto& bag = dog->GetBag();

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
                dog->SetScore(dog->GetScore() + bag.Drop());
            }
        }

        std::erase_if(lost_objects_, [](const auto& obj) {
            return obj.IsPickedUp();
        });
    }

    Strand strand_;
    Id id_;
    std::vector<DogHolder> dogs_;
    const Map& map_;
    LootGenerator& loot_generator_;
    std::shared_ptr<datetime::Ticker> loot_ticker_;
    LostObjects lost_objects_;
    std::chrono::milliseconds max_inactive_time_;
};

using GameSessionHolder = std::shared_ptr<GameSession>;
using GameSessions = std::vector<GameSessionHolder>;

} // namespace model
