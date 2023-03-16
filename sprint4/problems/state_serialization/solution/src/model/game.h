#pragma once

#include "model/map.h"
#include "model/loot_generator.h"
#include "model/game_session.h"
#include "utils/tagged.h"

#include <vector>

namespace model {

class Game {
  public:
    using Maps = std::vector<Map>;

    Game(LootGenerator loot_generator) :
        loot_generator_(std::move(loot_generator)) {}

    void AddMap(Map map);

    void AddSession(GameSessionHolder game_session) {
        // map_id_to_index_[game_session->GetMap().GetId()] = sessions_.size();
        sessions_.push_back(std::move(game_session));
    }

    const Maps& GetMaps() const noexcept {
        return maps_;
    }

    const Map* FindMap(const Map::Id& id) const noexcept {
        if (auto it = map_id_to_index_.find(id); it != map_id_to_index_.end()) {
            return &maps_.at(it->second);
        }
        return nullptr;
    }

    bool ContainsMap(const Map::Id& id) const noexcept {
        return FindMap(id) != nullptr;
    }

    const LootGenerator& GetLootGenerator() const {
        return loot_generator_;
    }

    LootGenerator& GetLootGenerator() {
        return loot_generator_;
    }

    GameSessionHolder FindGameSession(const Map::Id& map_id) {
        auto it = std::find_if(
            sessions_.begin(), sessions_.end(),
            [&](const auto& game_session) {
                return game_session->GetMap().GetId() == map_id;
            }
        );
        if (it != sessions_.end()) {
            return *it;
        }
        return nullptr;
    }

  private:
    using MapIdToIndex =
        std::unordered_map<Map::Id, size_t, utils::TaggedHasher<Map::Id>>;

    GameSessions sessions_;
    std::vector<Map> maps_;
    MapIdToIndex map_id_to_index_;
    LootGenerator loot_generator_;
};

} // namespace model
