#pragma once

#include "serde/archive/game_session.h"
#include "app/player.h"

namespace serde::archive {

class PlayerRepr {
  public:
    explicit PlayerRepr() = default;

    explicit PlayerRepr(const app::Player& player, const app::Token& token) :
        id_(*player.GetId()),
        map_id_(*player.GetSession()->GetMap().GetId()),
        name_(player.GetName()),
        dog_(*player.GetDog()),
        token_(*token) {}

    [[nodiscard]] app::Player Restore() const {
        return app::Player(app::Player::Id(id_), name_);
    }

    [[nodiscard]] model::Map::Id RestoreMapId() const {
        return model::Map::Id(map_id_);
    }

    [[nodiscard]] model::Dog RestoreDog() const {
        return dog_.Restore();
    }

    [[nodiscard]] app::Token RestoreToken() const {
        return app::Token(token_);
    }

    template <typename Archive>
    void serialize(Archive& ar, [[maybe_unused]] const unsigned version) {
        ar& id_;
        ar& map_id_;
        ar& name_;
        ar& dog_;
        ar& token_;
    }

  private:
    size_t id_;
    std::string map_id_;
    std::string name_;
    DogRepr dog_;
    std::string token_;
};

} // namespace serde::archive
