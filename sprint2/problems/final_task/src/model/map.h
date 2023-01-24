#pragma once

#include "utils/tagged.h"
#include "model/road.h"
#include "model/building.h"
#include "model/office.h"
#include "model/dog.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace model {

class Map {
  public:
    using Id = utils::Tagged<std::string, Map>;
    using Roads = std::vector<Road>;
    using Buildings = std::vector<Building>;
    using Offices = std::vector<Office>;

    struct Config {
        double dog_speed = 1;
    };

    Map(Id id, std::string name, Config config) noexcept :
        id_(id),
        name_(std::move(name)),
        config_(std::move(config)) {}

    const Id& GetId() const noexcept {
        return id_;
    }

    const std::string& GetName() const noexcept {
        return name_;
    }

    const Buildings& GetBuildings() const noexcept {
        return buildings_;
    }

    const Roads& GetRoads() const noexcept {
        return roads_;
    }

    const Offices& GetOffices() const noexcept {
        return offices_;
    }

    double GetDogSpeed() const noexcept {
        return config_.dog_speed;
    }

    void AddRoad(const Road& road) {
        roads_.emplace_back(road);
    }

    void AddBuilding(const Building& building) {
        buildings_.emplace_back(building);
    }

    void AddOffice(Office office);

  private:
    using OfficeIdToIndex =
        std::unordered_map<Office::Id, size_t, utils::TaggedHasher<Office::Id>>;

    Id id_;
    std::string name_;
    Roads roads_;
    Buildings buildings_;
    OfficeIdToIndex warehouse_id_to_index_;
    Offices offices_;
    Config config_;
};

}  // namespace model
