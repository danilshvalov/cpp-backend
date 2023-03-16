#pragma once

#include "serde/archive/dog.h"
#include "serde/archive/lost_object.h"
#include "model/game_session.h"

#include <boost/serialization/vector.hpp>

namespace serde::archive {

class GameSessionRepr {
  public:
    explicit GameSessionRepr() = default;

    explicit GameSessionRepr(const model::GameSession& session) :
        map_id_(*session.GetMap().GetId()) {
        const auto& dogs = session.GetDogs();
        dogs_.reserve(dogs.size());

        for (const auto& dog : dogs) {
            dogs_.push_back(DogRepr(*dog));
        }

        const auto& lost_objects = session.GetLostObjects();
        for (const auto& lost_object : lost_objects) {
            lost_objects_.push_back(LostObjectRepr(lost_object));
        }
    }

    [[nodiscard]] model::Map::Id RestoreMapId() const {
        return model::Map::Id(map_id_);
    }

    void RestoreDogs(model::GameSession& session) const {
        for (const auto& dog : dogs_) {
            session.AddDog(std::make_shared<model::Dog>(dog.Restore()));
        }
    }

    void RestoreLostObjects(model::GameSession& session) const {
        for (const auto& lost_object : lost_objects_) {
            session.AddLostObject(lost_object.Restore());
        }
    }

    template <typename Archive>
    void serialize(Archive& ar, [[maybe_unused]] const unsigned version) {
        ar& map_id_;
        ar& dogs_;
        ar& lost_objects_;
    }

  private:
    std::string map_id_;
    std::vector<DogRepr> dogs_;
    std::vector<LostObjectRepr> lost_objects_;
};

} // namespace serde::archive
