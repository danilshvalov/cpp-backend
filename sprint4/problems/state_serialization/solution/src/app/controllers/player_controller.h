#pragma once

#include "app/player.h"

namespace app {

class PlayersController {
  public:
    using PlayerByToken =
        std::unordered_map<Token, PlayerHolder, utils::TaggedHasher<Token>>;

    PlayerHolder FindPlayerBy(const Token& token) const {
        if (const auto it = players_.find(token); it != players_.end()) {
            return it->second;
        }
        return nullptr;
    }

    bool HasPlayer(const Token& token) const {
        return FindPlayerBy(token) != nullptr;
    }

    Token AddPlayer(PlayerHolder player) {
        Token token = GenerateToken();
        players_[token] = std::move(player);
        return token;
    }

    void AddPlayer(PlayerHolder player, Token token) {
        players_[token] = std::move(player);
    }

    void SetPlayerSession(
        const PlayerHolder& player, const model::GameSessionHolder& session
    ) {
        session_players_[session->GetId()].push_back(player);
    }

    const Players& GetPlayers(const Token& token) {
        static const Players empty_list = {};

        auto player = FindPlayerBy(token);
        if (!player) {
            throw std::invalid_argument("Player doesn't exists");
        }

        model::GameSessionHolder session = player->GetSession();

        if (!session || !session_players_.contains(session->GetId())) {
            return empty_list;
        }

        return session_players_[session->GetId()];
    }

    const model::GameSession::LostObjects& GetLostObjects(const Token& token) {
        static const model::GameSession::LostObjects empty_list = {};

        auto player = FindPlayerBy(token);
        if (!player) {
            throw std::invalid_argument("Player doesn't exists");
        }

        const auto& session = player->GetSession();

        if (!session || !session_players_.contains(session->GetId())) {
            return empty_list;
        }

        return session->GetLostObjects();
    }

    const PlayerByToken& GetPlayers() const {
        return players_;
    }

    Player::Id GetFreePlayerId() const {
        return Player::Id(players_.size());
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

        return Token{ss.str()};
    }

    std::mt19937_64 MakeGenerator() {
        Distribution dist;
        return std::mt19937_64(dist(random_device_));
    }

    PlayerByToken players_;
    std::unordered_map<
        model::GameSession::Id, Players,
        utils::TaggedHasher<model::GameSession::Id>>
        session_players_;
    std::random_device random_device_;
    std::mt19937_64 generator1_ = MakeGenerator();
    std::mt19937_64 generator2_ = MakeGenerator();
};
} // namespace app
