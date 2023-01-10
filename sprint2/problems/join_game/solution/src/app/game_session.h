#pragma once

#include "model/map.h"
#include "utils/tagged.h"

namespace app {

class GameSession {
  public:
    using Id = utils::Tagged<std::string, GameSession>;

    GameSession(const model::Map& map) : id_(*map.GetId()), map_(map) {}

    const Id& GetId() const {
        return id_;
    }

    const model::Map& GetMap() const {
        return map_;
    }

  private:
    Id id_;
    std::vector<model::Dog> dogs_;
    const model::Map& map_;
};

// TODO:
using GameSessionIdHasher = utils::TaggedHasher<GameSession::Id>;

}  // namespace app
