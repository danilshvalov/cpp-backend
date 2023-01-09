#pragma once

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
    using Strand = net::strand<net::io_context::executor_type>;
    using Players = std::vector<Player*>;

    Application(net::io_context& io, model::Game game) :
        io_(io),
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
            game_session = MakeGameSession(*map);
        }

        SetPlayerGameSession(*player, *game_session);
        return JoinGameResult {
            .token = std::move(token),
            .id = player->GetId(),
        };
    }

  private:
    Player* MakePlayer(std::string name) {
        players_.push_back(std::make_unique<Player>(std::move(name)));
        return players_.back().get();
    }

    GameSession* FindGameSessionBy(const model::Map::Id& map_id) {
        if (auto it = map_to_session_.find(map_id);
            it != map_to_session_.end()) {
            return &sessions_.at(it->second);
        }
        return nullptr;
    }

    void SetPlayerGameSession(Player& player, GameSession& session) {
        session_players_[session.GetId()].push_back(&player);
        player.SetGameSession(&session);
        // player.CreateDog(player.GetName(), session.GetMap());
    }

    GameSession* MakeGameSession(const model::Map& map) {
        GameSession session(map);

        if (map_to_session_.count(map.GetId())) {
            // TODO: add exception message
            throw std::invalid_argument("");
        }

        map_to_session_[map.GetId()] = sessions_.size();
        sessions_.push_back(std::move(session));

        return &sessions_.back();
    }

    net::io_context& io_;
    Strand strand_;
    model::Game game_;
    std::vector<std::unique_ptr<Player>> players_;
    std::vector<GameSession> sessions_;
    PlayerTokens player_tokens_;
    std::unordered_map<
        GameSession::Id,
        std::vector<Player*>,
        GameSessionIdHasher>
        session_players_;
    std::unordered_map<model::Map::Id, size_t, MapIdHasher> map_to_session_;
};
}  // namespace app
