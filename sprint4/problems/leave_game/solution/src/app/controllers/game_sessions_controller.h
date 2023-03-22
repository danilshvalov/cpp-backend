#pragma once

#include "model/game_session.h"
#include "model/game.h"

#include <chrono>

namespace app {

namespace net = boost::asio;

class GameSessionsController {
  public:
    using GameSessions = std::vector<model::GameSessionHolder>;
    using Strand = net::strand<net::io_context::executor_type>;

    GameSessionsController(
        net::io_context& io, Strand& strand, model::Game& game,
        std::chrono::milliseconds max_inactive_time
    ) :
        io_(io),
        game_(game) {}

    model::GameSessionHolder AddGameSession(const model::Map& map) {
        if (map_id_to_index_.contains(map.GetId())) {
            throw std::invalid_argument("Duplicate map");
        }

        map_id_to_index_[map.GetId()] = sessions_.size();
        sessions_.push_back(std::make_shared<model::GameSession>(
            io_, map, game_.GetLootGenerator(), game_.GetMaxInactiveTime()
        ));

        return sessions_.back();
    }

    model::GameSessionHolder FindGameSessionBy(const model::Map::Id& map_id) {
        if (auto it = map_id_to_index_.find(map_id);
            it != map_id_to_index_.end()) {
            return sessions_.at(it->second);
        }
        return nullptr;
    }

    void UpdateGameState(const std::chrono::milliseconds& time_delta) {
        for (const auto& session : sessions_) {
            session->UpdateGameState(time_delta);
        }
    }

    const GameSessions& GetSessions() const {
        return sessions_;
    }

    std::vector<model::DogHolder> ReleaseInactiveDogs() {
        std::vector<model::DogHolder> result;
        for (auto& session : sessions_) {
            auto dogs = session->ReleaseInactiveDogs();
            result.insert(
                result.end(), std::make_move_iterator(dogs.begin()),
                std::make_move_iterator(dogs.end())
            );
        }
        return result;
    }

  private:
    using MapIdToIndex = std::unordered_map<
        model::Map::Id, size_t, utils::TaggedHasher<model::Map::Id>>;

    net::io_context& io_;
    model::Game& game_;
    GameSessions sessions_;
    MapIdToIndex map_id_to_index_;
};
} // namespace app
