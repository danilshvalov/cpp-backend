#pragma once

#include "model/game_session.h"
#include "utils/tagged.h"

#include <iomanip>
#include <random>
#include <sstream>
#include <string>

namespace app {
class Player {
  public:
    using Id = utils::Tagged<size_t, class PlayerTag>;

    Player(Id id, std::string name) : id_(id), name_(std::move(name)) {}

    const Id& GetId() const {
        return id_;
    }

    const std::string& GetName() const {
        return name_;
    }

    const model::GameSessionHolder& GetSession() const {
        return session_;
    }

    model::GameSessionHolder& GetSession() {
        return session_;
    }

    void SetGameSession(model::GameSessionHolder session) {
        session_ = std::move(session);
    }

    const model::DogHolder& GetDog() const {
        return dog_;
    }

    void SetDog(model::DogHolder dog) {
        dog_ = std::move(dog);
    }

  private:
    inline static size_t free_id_ = 0;

    Id id_;
    std::string name_;
    model::GameSessionHolder session_ = nullptr;
    model::DogHolder dog_ = nullptr;
};

using PlayerHolder = std::shared_ptr<Player>;
using Players = std::vector<PlayerHolder>;

using Token = utils::Tagged<std::string, class TokenTag>;

} // namespace app
