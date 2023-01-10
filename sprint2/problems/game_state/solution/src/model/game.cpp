#include "model/game.h"

#include <stdexcept>

namespace model {

void Game::AddMap(Map map) {
    const size_t index = maps_.size();
    if (auto [it, inserted] = map_id_to_index_.emplace(map.GetId(), index);
        !inserted) {
        throw std::invalid_argument(
            "Map with id " + *map.GetId() + " already exists"
        );
    } else {
        try {
            maps_.emplace_back(std::move(map));
        } catch (...) {
            map_id_to_index_.erase(it);
            throw;
        }
    }
}

}  // namespace model
