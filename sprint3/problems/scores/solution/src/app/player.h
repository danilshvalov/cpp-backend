#pragma once

#include "app/game_session.h"
#include "utils/tagged.h"

#include <string>
#include <random>
#include <iomanip>
#include <sstream>

namespace app {
class Player {
  public:
    using Id = utils::Tagged<size_t, class PlayerTag>;

    Player(std::string name) : id_(Id(free_id_++)), name_(std::move(name)) {}

    const Id& GetId() const {
        return id_;
    }

    const std::string& GetName() const {
        return name_;
    }

    const GameSessionHolder& GetSession() const {
        return session_;
    }

    GameSessionHolder& GetSession() {
        return session_;
    }

    void SetGameSession(GameSessionHolder session) {
        session_ = std::move(session);
    }

    const model::Dog* GetDog() const {
        return dog_;
    }

    model::Dog* GetDog() {
        return dog_;
    }

    void SetDog(model::Dog* dog) {
        dog_ = dog;
    }

  private:
    inline static size_t free_id_ = 0;

    Id id_;
    std::string name_;
    std::shared_ptr<GameSession> session_ = nullptr;
    model::Dog* dog_ = nullptr;
};

using PlayerHolder = std::shared_ptr<Player>;

using Token = utils::Tagged<std::string, class TokenTag>;

class PlayerTokens {
  public:
    PlayerHolder FindPlayerBy(const Token& token) const {
        if (const auto it = players_.find(token); it != players_.end()) {
            return it->second;
        }
        return nullptr;
    }

    Token AddPlayer(PlayerHolder player) {
        Token token = GenerateToken();
        players_[token] = std::move(player);
        return token;
    }

  private:
    using Distribution =
        std::uniform_int_distribution<std::mt19937_64::result_type>;

    Token GenerateToken() {
        constexpr size_t token_size = 16;
        std::stringstream ss;

        const auto add_hex_number = [&](auto x) {
            ss << std::setfill('0') << std::setw(token_size) << std::hex << x;
        };

        add_hex_number(generator1_());
        add_hex_number(generator2_());

        return Token {ss.str()};
    }

    std::mt19937_64 MakeGenerator() {
        Distribution dist;
        return std::mt19937_64(dist(random_device_));
    }

    std::unordered_map<Token, PlayerHolder, utils::TaggedHasher<Token>>
        players_;
    std::random_device random_device_;
    std::mt19937_64 generator1_ = MakeGenerator();
    std::mt19937_64 generator2_ = MakeGenerator();
};

}  // namespace app
