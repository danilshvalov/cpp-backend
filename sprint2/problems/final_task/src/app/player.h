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

    const GameSession* GetSession() const {
        return session_;
    }

    GameSession* GetSession() {
        return session_;
    }

    void SetGameSession(GameSession* session) {
        session_ = session;
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
    GameSession* session_ = nullptr;
    model::Dog* dog_ = nullptr;
};

using Token = utils::Tagged<std::string, class TokenTag>;

class PlayerTokens {
  public:
    app::Player* FindPlayerBy(const Token& token) const {
        if (const auto it = players_.find(token); it != players_.end()) {
            return it->second;
        }
        return nullptr;
    }

    Token AddPlayer(app::Player* player) {
        Token token = GenerateToken();
        players_[token] = player;
        return token;
    }

  private:
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

    std::unordered_map<Token, app::Player*, utils::TaggedHasher<Token>>
        players_;
    std::random_device random_device_;
    std::mt19937_64 generator1_ {[this] {
        std::uniform_int_distribution<std::mt19937_64::result_type> dist;
        return dist(random_device_);
    }()};
    std::mt19937_64 generator2_ {[this] {
        std::uniform_int_distribution<std::mt19937_64::result_type> dist;
        return dist(random_device_);
    }()};
};

}  // namespace app
