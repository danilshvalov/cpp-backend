#pragma once

#include "app/game_session.h"
#include "app/player.h"

#include "model/map.h"
#include "model/game.h"
#include "utils/tagged.h"
#include "datetime/ticker.h"

#include <boost/asio/strand.hpp>
#include <boost/asio/io_context.hpp>

#include <random>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <iomanip>
#include <format>
#include <chrono>

namespace app {

namespace net = boost::asio;

struct JoinGameData {
    std::string name;
    model::Map::Id map_id;
};

struct JoinGameResult {
    Token token;
    Player::Id id;
};

class Application {
  public:
    using Strand = net::strand<net::io_context::executor_type>;

    using Players = std::vector<std::shared_ptr<Player>>;
    using Sessions = std::vector<std::shared_ptr<GameSession>>;

    Application(
        net::io_context& io,
        model::Game game,
        const std::chrono::milliseconds& tick_period,
        bool randomize_spawn_points
    ) :
        io_(io),
        strand_(net::make_strand(io)),
        game_(std::move(game)),
        randomize_spawn_points_(randomize_spawn_points) {
        if (tick_period.count() != 0) {
            ticker_ = std::make_shared<datetime::Ticker>(
                strand_,
                tick_period,
                [&](std::chrono::milliseconds delta) { UpdateGameState(delta); }
            );
            ticker_->Start();
        }
    }

    Strand& GetStrand() {
        return strand_;
    }

    const model::Game::Maps& ListMaps() const {
        return game_.GetMaps();
    }

    const model::Map* FindMap(const model::Map::Id& id) const {
        return game_.FindMap(id);
    }

    bool HasPlayer(const Token& token) const {
        return player_tokens_.FindPlayerBy(token) != nullptr;
    }

    const Players& GetPlayers(const Token& token) {
        static const Players empty_list = {};

        auto player = player_tokens_.FindPlayerBy(token);
        if (!player) {
            throw std::invalid_argument("Player doesn't exists");
        }

        GameSessionHolder session = player->GetSession();

        if (!session || !session_players_.contains(session->GetId())) {
            return empty_list;
        }

        return session_players_[session->GetId()];
    }

    const GameSession::LostObjects& GetLostObjects(const Token& token) {
        static const GameSession::LostObjects empty_list = {};

        auto player = player_tokens_.FindPlayerBy(token);
        if (!player) {
            throw std::invalid_argument("Player doesn't exists");
        }

        GameSessionHolder session = player->GetSession();

        if (!session || !session_players_.contains(session->GetId())) {
            return empty_list;
        }

        return session->GetLostObjects();
    }

    void UpdateGameState(const std::chrono::milliseconds& time_delta) {
        for (const auto& session : sessions_) {
            session->UpdateGameState(time_delta);
        }
    }

    JoinGameResult JoinGame(const JoinGameData& data) {
        const model::Map* map = game_.FindMap(data.map_id);

        if (!map) {
            throw std::invalid_argument(
                "Map with id " + *data.map_id + " doesn't exists"
            );
        }

        auto player = MakePlayer(std::move(data.name));
        Token token = player_tokens_.AddPlayer(player);
        std::shared_ptr<GameSession> game_session =
            FindGameSessionBy(data.map_id);

        if (!game_session) {
            game_session = AddGameSession(*map);
        }

        SetPlayerGameSession(player, game_session);
        return JoinGameResult {
            .token = std::move(token),
            .id = player->GetId(),
        };
    }

    void MovePlayer(const Token& token, model::Direction direction) {
        PlayerHolder player = player_tokens_.FindPlayerBy(token);
        double dog_speed = player->GetSession()->GetMap().GetDogSpeed();

        player->GetDog()->SetSpeed(model::Speed(dog_speed, direction));
        player->GetDog()->SetDirection(direction);
    }

    bool HasTickPeriod() const {
        return ticker_ != nullptr;
    }

  private:
    std::shared_ptr<Player> MakePlayer(std::string name) {
        players_.push_back(std::make_shared<Player>(std::move(name)));
        return players_.back();
    }

    std::shared_ptr<GameSession> FindGameSessionBy(const model::Map::Id& map_id
    ) {
        if (auto it = map_id_to_index.find(map_id);
            it != map_id_to_index.end()) {
            return sessions_.at(it->second);
        }
        return nullptr;
    }

    void SetPlayerGameSession(
        const PlayerHolder& player,
        const GameSessionHolder& session
    ) {
        player->SetGameSession(session);
        player->SetDog(session->CreateDog(randomize_spawn_points_));
        session_players_[session->GetId()].push_back(player);
    }

    std::shared_ptr<GameSession> AddGameSession(const model::Map& map) {
        if (map_id_to_index.contains(map.GetId())) {
            throw std::invalid_argument("Duplicate map");
        }

        map_id_to_index[map.GetId()] = sessions_.size();
        sessions_.push_back(
            std::make_shared<GameSession>(io_, map, game_.GetLootGenerator())
        );
        return sessions_.back();
    }

    using MapIdToIndex = std::unordered_map<
        model::Map::Id,
        size_t,
        utils::TaggedHasher<model::Map::Id>>;

    net::io_context& io_;
    Strand strand_;
    model::Game game_;
    bool randomize_spawn_points_;
    std::shared_ptr<datetime::Ticker> ticker_;
    Players players_;
    Sessions sessions_;
    PlayerTokens player_tokens_;
    std::unordered_map<
        GameSession::Id,
        Players,
        utils::TaggedHasher<GameSession::Id>>
        session_players_;
    MapIdToIndex map_id_to_index;
};
}  // namespace app
