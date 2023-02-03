#pragma once

#include "model/map.h"
#include "model/loot_generator.h"
#include "utils/tagged.h"

#include <vector>

namespace model {

class Game {
  public:
    using Maps = std::vector<Map>;

    Game(LootGenerator loot_generator) :
        loot_generator_(std::move(loot_generator)) {}

    void AddMap(Map map);

    const Maps& GetMaps() const noexcept {
        return maps_;
    }

    const Map* FindMap(const Map::Id& id) const noexcept {
        if (auto it = map_id_to_index_.find(id); it != map_id_to_index_.end()) {
            return &maps_.at(it->second);
        }
        return nullptr;
    }

    const LootGenerator& GetLootGenerator() const {
        return loot_generator_;
    }

    LootGenerator& GetLootGenerator() {
        return loot_generator_;
    }

  private:
    using MapIdHasher = utils::TaggedHasher<Map::Id>;
    using MapIdToIndex = std::unordered_map<Map::Id, size_t, MapIdHasher>;

    std::vector<Map> maps_;
    MapIdToIndex map_id_to_index_;
    LootGenerator loot_generator_;
};

}  // namespace model
