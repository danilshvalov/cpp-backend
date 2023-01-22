#pragma once

#include "app/core.h"
#include "app/game_session.h"
#include "app/player.h"

#include "model/map.h"
#include "model/game.h"
#include "utils/tagged.h"

#include <boost/asio/strand.hpp>
#include <boost/asio/io_context.hpp>

#include <random>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <iomanip>
#include <format>
#include <chrono>

// TODO: remove
#include <iostream>

namespace app {

namespace net = boost::asio;

// TODO:
using MapIdHasher = utils::TaggedHasher<model::Map::Id>;

struct JoinGameResult {
    Token token;
    Player::Id id;
};

class Application {
  public:
    using Players = std::vector<Player*>;

    Application(net::io_context& io, model::Game game) :
        strand_(net::make_strand(io)),
        game_(std::move(game)) {}

    Strand& GetStrand() {
        return strand_;
    }

    const model::Game::Maps& ListMaps() const {
        return game_.GetMaps();
    }

    const model::Map* FindMap(const model::Map::Id& id) const {
        return game_.FindMap(id);
    }

    // TODO: add const
    [[nodiscard]] bool HasPlayer(const Token& token) {
        return player_tokens_.FindPlayerBy(token) != nullptr;
    }

    const std::vector<Player*>& GetPlayers(const Token& token) {
        static const std::vector<Player*> empty_list = {};

        Player* player = player_tokens_.FindPlayerBy(token);
        if (!player) {
            // TODO: change exception message
            throw std::invalid_argument("Player doesn't exists");
        }

        GameSession* session = player->GetSession();

        if (!session || !session_players_.contains(session->GetId())) {
            return empty_list;
        }

        return session_players_[session->GetId()];
    }

    void UpdateGameState(const std::chrono::milliseconds& time_delta) {
        for (const auto& session : sessions_) {
            session->UpdateGameState(time_delta);
        }
    }

    JoinGameResult JoinGame(const model::Map::Id& map_id, std::string name) {
        const model::Map* map = game_.FindMap(map_id);

        if (!map) {
            throw std::invalid_argument(
                "Map with id " + *map_id + " doesn't exists"
            );
        }

        Player* player = MakePlayer(std::move(name));
        Token token = player_tokens_.AddPlayer(player);
        GameSession* game_session = FindGameSessionBy(map_id);

        if (!game_session) {
            game_session = AddGameSession(*map);
        }

        SetPlayerGameSession(*player, *game_session);
        return JoinGameResult {
            .token = std::move(token),
            .id = player->GetId(),
        };
    }

    void MovePlayer(const Token& token, model::Direction direction) {
        Player* player = player_tokens_.FindPlayerBy(token);
        double dog_speed = player->GetSession()->GetMap().GetDogSpeed();

        // TODO: rework
        player->GetDog()->SetSpeed(model::Speed(dog_speed, direction));
        player->GetDog()->SetDirection(direction);
    }

  private:
    Player* MakePlayer(std::string name) {
        players_.push_back(std::make_unique<Player>(std::move(name)));
        return players_.back().get();
    }

    GameSession* FindGameSessionBy(const model::Map::Id& map_id) {
        if (auto it = map_id_to_index.find(map_id);
            it != map_id_to_index.end()) {
            return sessions_.at(it->second).get();
        }
        return nullptr;
    }

    void SetPlayerGameSession(Player& player, GameSession& session) {
        session_players_[session.GetId()].push_back(&player);
        player.SetGameSession(&session);
        player.SetDog(session.CreateDog());
    }

    GameSession* AddGameSession(const model::Map& map) {
        if (map_id_to_index.contains(map.GetId())) {
            throw std::invalid_argument("Duplicate map");
        }

        const size_t index = sessions_.size();
        GameSession& session =
            *sessions_.emplace_back(std::make_unique<GameSession>(map));
        try {
            map_id_to_index.emplace(map.GetId(), index);
        } catch (...) {
            sessions_.pop_back();
            throw;
        }

        return &session;
    }

    using MapIdToIndex = std::unordered_map<
        model::Map::Id,
        size_t,
        utils::TaggedHasher<model::Map::Id>>;

    Strand strand_;
    model::Game game_;
    std::vector<std::unique_ptr<Player>> players_;
    std::vector<std::unique_ptr<GameSession>> sessions_;
    PlayerTokens player_tokens_;
    std::unordered_map<
        GameSession::Id,
        std::vector<Player*>,
        GameSessionIdHasher>
        session_players_;
    MapIdToIndex map_id_to_index;
};
}  // namespace app
