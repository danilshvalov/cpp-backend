#pragma once

#include "app/app.h"
#include "model/map.h"
#include "model/game.h"

#include <boost/json/value.hpp>

#include <filesystem>

namespace serde::json {

namespace json = boost::json;

model::Game LoadGame(const std::filesystem::path& json_path);

model::Direction ParseDirection(const json::object& value);

json::value SerializeMapInfo(const model::Map& map);

json::value SerializeMapsList(const model::Game::Maps& maps);

json::value SerializePlayers(const app::Application::Players& players);

json::value SerializeGameState(const app::Application::Players& players);

}  // namespace serde::json
