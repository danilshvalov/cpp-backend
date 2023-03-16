#pragma once

#include "app/controllers/player_controller.h"
#include "app/controllers/game_sessions_controller.h"
#include "app/player.h"
#include "datetime/ticker.h"
#include "model/game.h"
#include "model/game_session.h"
#include "model/map.h"
#include "utils/tagged.h"
#include "serde/archive.h"

#include <boost/asio/io_context.hpp>
#include <boost/asio/strand.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include <chrono>
#include <format>
#include <iomanip>
#include <random>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <fstream>

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

struct SaveStateConfig {
    std::string state_file;
    std::chrono::milliseconds save_period{0};
};

class Application {
  public:
    using Strand = net::strand<net::io_context::executor_type>;

    using Players = std::vector<PlayerHolder>;
    using GameSessions = std::vector<model::GameSessionHolder>;

    Application(
        net::io_context& io, model::Game game,
        const std::chrono::milliseconds& tick_period,
        bool randomize_spawn_points, SaveStateConfig save_state_config
    ) :
        io_(io),
        strand_(net::make_strand(io)),
        game_(std::move(game)),
        randomize_spawn_points_(randomize_spawn_points),
        game_sessions_(io_, strand_, game_, tick_period),
        save_state_config_(std::move(save_state_config)) {
        RestoreGameState();

        if (tick_period.count() != 0) {
            ticker_ = std::make_shared<datetime::Ticker>(
                strand_, tick_period,
                [&](std::chrono::milliseconds delta) {
                    UpdateGameState(delta);

                    time_without_save_ += delta;
                    if (time_without_save_ >= save_state_config_.save_period) {
                        time_without_save_ = std::chrono::milliseconds(0);
                        SaveGameState();
                    }
                }
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
        return players_.FindPlayerBy(token) != nullptr;
    }

    const Players& GetPlayers(const Token& token) {
        return players_.GetPlayers(token);
    }

    const model::GameSession::LostObjects& GetLostObjects(const Token& token) {
        return players_.GetLostObjects(token);
    }

    void UpdateGameState(const std::chrono::milliseconds& time_delta) {
        game_sessions_.UpdateGameState(time_delta);
    }

    JoinGameResult JoinGame(const JoinGameData& data) {
        const model::Map* map = game_.FindMap(data.map_id);

        if (!map) {
            throw std::invalid_argument(
                "Map with id " + *data.map_id + " doesn't exists"
            );
        }

        auto player = std::make_shared<Player>(std::move(data.name));
        Token token = players_.AddPlayer(player);
        auto game_session = game_sessions_.FindGameSessionBy(data.map_id);

        if (!game_session) {
            game_session = game_sessions_.AddGameSession(*map);
        }

        SetPlayerGameSession(player, game_session);
        return JoinGameResult{
            .token = std::move(token),
            .id = player->GetId(),
        };
    }

    void MovePlayer(const Token& token, model::Direction direction) {
        PlayerHolder player = players_.FindPlayerBy(token);
        double dog_speed = player->GetSession()->GetMap().GetDogSpeed();

        player->GetDog()->SetSpeed(model::Speed(dog_speed, direction));
        player->GetDog()->SetDirection(direction);
    }

    bool HasTickPeriod() const {
        return ticker_ != nullptr;
    }

  private:
    void SaveGameState() {
        using namespace serde::archive;
        if (save_state_config_.state_file.empty()) {
            return;
        }

        std::ofstream output(save_state_config_.state_file, std::ios_base::in);
        if (!output) {
            throw std::runtime_error("Cannot open state file");
        }

        boost::archive::text_oarchive oarchive{output};

        std::vector<GameSessionRepr> game_sessions;
        for (const auto& game_session : game_sessions_.GetSessions()) {
            game_sessions.push_back(GameSessionRepr(*game_session));
        }

        oarchive << game_sessions;

        std::vector<PlayerRepr> players;
        for (const auto& [token, player] : players_.GetPlayers()) {
            players.push_back(PlayerRepr(*player, token));
        }

        oarchive << players;
    }

    void RestoreGameState() {
        using namespace serde::archive;
        if (save_state_config_.state_file.empty()) {
            return;
        }

        std::ifstream input(save_state_config_.state_file, std::ios_base::in);
        if (!input) {
            throw std::runtime_error("Cannot open state file");
        }

        boost::archive::text_iarchive iarchive{input};

        std::vector<GameSessionRepr> game_sessions;
        iarchive >> game_sessions;
        std::vector<PlayerRepr> players;
        iarchive >> players;

        for (const auto& game_session_repr : game_sessions) {
            const model::Map* map =
                game_.FindMap(game_session_repr.RestoreMapId());

            if (!map) {
                throw std::runtime_error("Unknown map");
            }
            auto game_session = game_sessions_.AddGameSession(*map);
            game_session_repr.RestoreLostObjects(*game_session);
        }

        for (const auto& player_repr : players) {
            auto player = std::make_shared<Player>(player_repr.Restore());
            players_.AddPlayer(player, player_repr.RestoreToken());
            auto game_session =
                game_sessions_.FindGameSessionBy(player_repr.RestoreMapId());

            if (!game_session) {
                throw std::runtime_error("Cannot find game session");
            }

            auto dog = std::make_shared<model::Dog>(player_repr.RestoreDog());
            game_session->AddDog(dog);

            player->SetGameSession(game_session);
            player->SetDog(dog);
            players_.SetPlayerSession(player, game_session);
        }
    }

    void SetPlayerGameSession(
        const PlayerHolder& player, const model::GameSessionHolder& session
    ) {
        player->SetGameSession(session);
        player->SetDog(session->CreateDog(randomize_spawn_points_));
        players_.SetPlayerSession(player, session);
    }

    net::io_context& io_;
    Strand strand_;
    std::shared_ptr<datetime::Ticker> ticker_;
    model::Game game_;
    bool randomize_spawn_points_;
    GameSessionsController game_sessions_;
    PlayersController players_;
    SaveStateConfig save_state_config_;
    std::chrono::milliseconds time_without_save_{0};
};
} // namespace app
