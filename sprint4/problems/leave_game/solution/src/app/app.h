#pragma once

#include "app/controllers/player_controller.h"
#include "app/controllers/game_sessions_controller.h"
#include "app/player.h"
#include "app/use_cases_impl.h"
#include "datetime/ticker.h"
#include "model/game.h"
#include "model/game_session.h"
#include "model/map.h"
#include "utils/tagged.h"
#include "serde/archive.h"
#include "postgres/database.h"

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
#include <filesystem>

#include <iostream>

namespace app {

namespace net = boost::asio;
namespace fs = std::filesystem;

struct PlayerRecordsData {
    size_t start = 0;
    size_t max_items = 100;
};

struct JoinGameData {
    std::string name;
    model::Map::Id map_id;
};

struct JoinGameResult {
    Token token;
    Player::Id id;
};

struct LootConfig {
    std::chrono::milliseconds tick_period;
    bool randomize_spawn_points;
};

struct SaveStateConfig {
    std::string state_file;
    std::chrono::milliseconds save_period{0};
};

struct ApplicationConfig {
    LootConfig loot;
    SaveStateConfig save_state;
    postgres::DatabaseConfig database;
};

class Application {
  public:
    using Strand = net::strand<net::io_context::executor_type>;

    using Players = std::vector<PlayerHolder>;
    using GameSessions = std::vector<model::GameSessionHolder>;

    Application(
        net::io_context& io, model::Game game, const ApplicationConfig& config
    ) :
        io_(io),
        strand_(net::make_strand(io)),
        game_(std::move(game)),
        game_sessions_(io_, strand_, game_, config.loot.tick_period),
        config_(config),
        db_(config_.database) {
        RestoreGameState();

        const auto tick_period = config.loot.tick_period;
        if (tick_period.count() != 0) {
            ticker_ = std::make_shared<datetime::Ticker>(
                strand_, tick_period,
                [&](std::chrono::milliseconds delta) {
                    UpdateGameState(delta);
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
        ProcessInactiveDogs();

        time_without_save_ += time_delta;
        if (time_without_save_ >= config_.save_state.save_period) {
            time_without_save_ = std::chrono::milliseconds(0);
            SaveGameState();
        }
    }

    JoinGameResult JoinGame(const JoinGameData& data) {
        const model::Map* map = game_.FindMap(data.map_id);

        if (!map) {
            throw std::invalid_argument(
                "Map with id " + *data.map_id + " doesn't exists"
            );
        }

        auto player = std::make_shared<Player>(
            players_.GetFreePlayerId(), std::move(data.name)
        );
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
        auto& dog = player->GetDog();

        dog->SetSpeed(model::Speed(dog_speed, direction));
        dog->SetDirection(direction);

        if (direction != model::Direction::NONE) {
            dog->SetInactiveTime(std::chrono::milliseconds(0));
        }
    }

    bool HasTickPeriod() const {
        return ticker_ != nullptr;
    }

    void SaveGameState() {
        using namespace serde::archive;
        const auto& state_file = config_.save_state.state_file;
        if (state_file.empty()) {
            return;
        }

        fs::create_directory(fs::path(state_file).parent_path());

        const std::string output_filename = state_file + ".tmp";

        std::ofstream output(output_filename);
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
        output.close();
        fs::rename(output_filename, state_file);
    }

    std::vector<PlayerRecord> GetPlayerRecords(const PlayerRecordsData& data) {
        return use_cases_.GetPlayerRecords(data.start, data.max_items);
    }

  private:
    void RestoreGameState() {
        using namespace serde::archive;
        const auto& state_file = config_.save_state.state_file;
        if (state_file.empty()) {
            return;
        }

        std::ifstream input(state_file);
        if (!input) {
            return;
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
        player->SetDog(session->CreateDog(config_.loot.randomize_spawn_points));
        players_.SetPlayerSession(player, session);
    }

    void ProcessInactiveDogs() {
        std::vector<PlayerRecord> records;

        for (const auto& player :
             players_.RemoveInactivePlayers(game_.GetMaxInactiveTime())) {
            const auto& dog = player->GetDog();
            records.push_back(PlayerRecord{
                player->GetName(),
                dog->GetScore(),
                dog->GetLiveTime(),
            });
            player->GetSession()->RemoveDog(dog);
        }

        use_cases_.SavePlayerRecords(records);
    }

    net::io_context& io_;
    Strand strand_;
    std::shared_ptr<datetime::Ticker> ticker_;
    model::Game game_;
    GameSessionsController game_sessions_;
    PlayersController players_;
    ApplicationConfig config_;
    std::chrono::milliseconds time_without_save_{0};
    postgres::Database db_;
    app::UseCasesImpl use_cases_{db_.GetUnitOfWorkFactory()};
};
} // namespace app
