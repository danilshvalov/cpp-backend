#include "serde/json.h"

#include "datetime/consts.h"
#include "model/loot_generator.h"
#include "model/loot_type.h"

#include <boost/json/parse.hpp>
#include <boost/json/value.hpp>
#include <boost/json/value_to.hpp>

#include <fstream>
#include <iterator>

namespace serde::json {

namespace json = boost::json;

using namespace model;

namespace keys {
    namespace Direction {
        constexpr json::string_view move = "move";
        constexpr json::string_view west = "L";
        constexpr json::string_view east = "R";
        constexpr json::string_view north = "U";
        constexpr json::string_view south = "D";
        constexpr json::string_view none = "";
    }  // namespace Direction

    namespace Road {
        constexpr json::string_view start_x = "x0";
        constexpr json::string_view start_y = "y0";
        constexpr json::string_view end_x = "x1";
        constexpr json::string_view end_y = "y1";
    }  // namespace Road

    namespace Building {
        constexpr json::string_view x = "x";
        constexpr json::string_view y = "y";
        constexpr json::string_view width = "w";
        constexpr json::string_view height = "h";
    }  // namespace Building

    namespace Office {
        constexpr json::string_view id = "id";
        constexpr json::string_view x = "x";
        constexpr json::string_view y = "y";
        constexpr json::string_view offset_x = "offsetX";
        constexpr json::string_view offset_y = "offsetY";
    }  // namespace Office

    namespace Map {
        constexpr json::string_view id = "id";
        constexpr json::string_view name = "name";
        constexpr json::string_view roads = "roads";
        constexpr json::string_view buildings = "buildings";
        constexpr json::string_view offices = "offices";
        constexpr json::string_view loot_types = "lootTypes";
        constexpr json::string_view dog_speed = "dogSpeed";
    }  // namespace Map

    namespace MapsList {
        constexpr json::string_view id = "id";
        constexpr json::string_view name = "name";
    }  // namespace MapsList

    namespace LootGenerator {
        constexpr json::string_view period = "period";
        constexpr json::string_view probability = "probability";
    }  // namespace LootGenerator

    namespace Game {
        constexpr json::string_view maps = "maps";
        constexpr json::string_view default_dog_speed = "defaultDogSpeed";
        constexpr json::string_view loot_generator_config =
            "lootGeneratorConfig";
    }  // namespace Game

    namespace Player {
        constexpr json::string_view name = "name";
    }

    namespace GameState {
        constexpr json::string_view position = "pos";
        constexpr json::string_view speed = "speed";
        constexpr json::string_view direction = "dir";
        constexpr json::string_view players = "players";
        constexpr json::string_view lost_objects = "lostObjects";
        constexpr json::string_view type = "type";
    }  // namespace GameState

    namespace Tick {
        constexpr json::string_view time_delta = "timeDelta";
    }

    namespace LootType {
        constexpr json::string_view name = "name";
        constexpr json::string_view file = "file";
        constexpr json::string_view type = "type";
        constexpr json::string_view rotation = "rotation";
        constexpr json::string_view color = "color";
        constexpr json::string_view scale = "scale";
    }  // namespace LootType
}  // namespace keys

json::value LoadJson(const std::filesystem::path& json_path) {
    std::ifstream json_file(json_path);

    if (!json_file) {
        throw std::runtime_error(
            "Cannot open configuration file " + json_path.string()
        );
    }

    std::string json_string(
        std::istreambuf_iterator<char>(json_file.rdbuf()),
        std::istreambuf_iterator<char>()
    );

    return json::parse(json_string);
}

Direction ParseDirection(const json::object& object) {
    const auto& move = object.at(keys::Direction::move).as_string();

    if (move == keys::Direction::west) {
        return Direction::WEST;
    } else if (move == keys::Direction::east) {
        return Direction::EAST;
    } else if (move == keys::Direction::north) {
        return Direction::NORTH;
    } else if (move == keys::Direction::south) {
        return Direction::SOUTH;
    } else if (move == keys::Direction::none) {
        return Direction::NONE;
    } else {
        throw std::invalid_argument("Incorrect direction of movement");
    }
}

std::chrono::milliseconds ParseTick(const json::object& object) {
    return std::chrono::milliseconds(
        object.at(keys::Tick::time_delta).as_int64()
    );
}

app::JoinGameData ParseJoinGameData(const json::object& object) {
    return app::JoinGameData {
        .name = std::string(object.at("userName").as_string()),
        .map_id = Map::Id(std::string(object.at("mapId").as_string())),
    };
}

Road ParseRoad(const json::object& object) {
    Point start {
        Coord(object.at(keys::Road::start_x).as_int64()),
        Coord(object.at(keys::Road::start_y).as_int64()),
    };

    if (auto end_x = object.find(keys::Road::end_x); end_x != object.end()) {
        return Road(Road::HORIZONTAL, start, end_x->value().as_int64());
    } else if (auto end_y = object.find(keys::Road::end_y);
               end_y != object.end()) {
        return Road(Road::VERTICAL, start, end_y->value().as_int64());
    } else {
        throw std::runtime_error("End coordinate is missing");
    }
}

Building ParseBuilding(const json::object& object) {
    return Building {Rectangle {
        Point {
            Coord(object.at(keys::Building::x).as_int64()),
            Coord(object.at(keys::Building::y).as_int64()),
        },
        Size {
            Dimension(object.at(keys::Building::width).as_int64()),
            Dimension(object.at(keys::Building::height).as_int64()),
        },
    }};
}

Office ParseOffice(const json::object& object) {
    return Office {
        Office::Id(json::value_to<std::string>(object.at(keys::Office::id))),
        Point {
            Coord(object.at(keys::Office::x).as_int64()),
            Coord(object.at(keys::Office::y).as_int64()),
        },
        Offset {
            Dimension(object.at(keys::Office::offset_x).as_int64()),
            Dimension(object.at(keys::Office::offset_y).as_int64()),
        },
    };
}

LootType ParseLootType(const json::object& object) {
    LootType loot_type {
        .name = std::string(object.at(keys::LootType::name).as_string()),
        .file = std::string(object.at(keys::LootType::file).as_string()),
        .type = std::string(object.at(keys::LootType::type).as_string()),
        .scale = object.at(keys::LootType::scale).as_double(),
    };

    if (auto it = object.find(keys::LootType::rotation); it != object.end()) {
        loot_type.rotation = it->value().as_int64();
    }

    if (auto it = object.find(keys::LootType::color); it != object.end()) {
        loot_type.color = std::string(it->value().as_string());
    }

    return loot_type;
}

Map ParseMap(const json::object& object, Map::Config config) {
    if (auto it = object.find(keys::Map::dog_speed); it != object.end()) {
        config.dog_speed = it->value().as_double();
    }

    Map map(
        Map::Id(json::value_to<std::string>(object.at(keys::Map::id))),
        json::value_to<std::string>(object.at(keys::Map::name)),
        config
    );

    for (const auto& node : object.at(keys::Map::roads).as_array()) {
        map.AddRoad(ParseRoad(node.as_object()));
    }

    for (const auto& node : object.at(keys::Map::buildings).as_array()) {
        map.AddBuilding(ParseBuilding(node.as_object()));
    }

    for (const auto& node : object.at(keys::Map::offices).as_array()) {
        map.AddOffice(ParseOffice(node.as_object()));
    }

    for (const auto& node : object.at(keys::Map::loot_types).as_array()) {
        map.AddLootType(ParseLootType(node.as_object()));
    }

    return map;
}

LootGenerator::Config ParseLootGeneratorConfig(const json::object& object) {
    return LootGenerator::Config {
        .base_interval = LootGenerator::TimeInterval(static_cast<uint64_t>(
            object.at(keys::LootGenerator::period).as_double() *
            datetime::milliseconds_in_second
        )),
        .probability = object.at(keys::LootGenerator::probability).as_double(),
    };
}

Game LoadGame(const std::filesystem::path& json_path) {
    json::object document = LoadJson(json_path).as_object();

    LootGenerator::Config loot_generator_config = ParseLootGeneratorConfig(
        document.at(keys::Game::loot_generator_config).as_object()
    );

    Game game(LootGenerator {loot_generator_config});

    Map::Config map_config;

    if (auto it = document.find(keys::Game::default_dog_speed);
        it != document.end()) {
        map_config.dog_speed = it->value().as_double();
    }

    for (const auto& node : document.at(keys::Game::maps).as_array()) {
        game.AddMap(ParseMap(node.as_object(), map_config));
    }

    return game;
}

json::value SerializeRoads(const model::Map::Roads& roads) {
    json::array array;
    array.reserve(roads.size());

    for (const auto& road : roads) {
        const auto& start = road.GetStart();
        const auto& end = road.GetEnd();

        json::object object {
            {keys::Road::start_x, start.x},
            {keys::Road::start_y, start.y},
        };

        if (road.IsHorizontal()) {
            object[keys::Road::end_x] = end.x;
        } else {
            object[keys::Road::end_y] = end.y;
        }

        array.push_back(std::move(object));
    }

    return array;
}

json::value SerializeBuildings(const model::Map::Buildings& buildings) {
    json::array array;
    array.reserve(buildings.size());

    for (const auto& building : buildings) {
        const auto& [position, size] = building.GetBounds();

        array.push_back(json::value {
            {keys::Building::x, position.x},
            {keys::Building::y, position.y},
            {keys::Building::width, size.width},
            {keys::Building::height, size.height},
        });
    }

    return array;
}

json::value SerializeOffices(const model::Map::Offices& offices) {
    json::array array;
    array.reserve(offices.size());

    for (const auto& office : offices) {
        const auto& pos = office.GetPosition();
        const auto& offset = office.GetOffset();

        array.push_back(json::value {
            {keys::Office::id, *office.GetId()},
            {keys::Office::x, pos.x},
            {keys::Office::y, pos.y},
            {keys::Office::offset_x, offset.dx},
            {keys::Office::offset_y, offset.dy},
        });
    }

    return array;
}

json::value SerializeLootTypes(const model::Map::LootTypes& loot_types) {
    json::array array;
    array.reserve(loot_types.size());

    for (const auto& loot_type : loot_types) {
        json::object object {
            {keys::LootType::name, loot_type.name},
            {keys::LootType::file, loot_type.file.string()},
            {keys::LootType::type, loot_type.type},
            {keys::LootType::scale, loot_type.scale},
        };

        if (loot_type.rotation) {
            object[keys::LootType::rotation] = *loot_type.rotation;
        }

        if (loot_type.color) {
            object[keys::LootType::color] = *loot_type.color;
        }

        array.push_back(std::move(object));
    }

    return array;
}

json::value SerializeMapInfo(const model::Map& map) {
    return json::value {
        {keys::Map::id, *map.GetId()},
        {keys::Map::name, map.GetName()},
        {keys::Map::roads, SerializeRoads(map.GetRoads())},
        {keys::Map::buildings, SerializeBuildings(map.GetBuildings())},
        {keys::Map::offices, SerializeOffices(map.GetOffices())},
        {keys::Map::loot_types, SerializeLootTypes(map.GetLootTypes())},
    };
}

json::value SerializeMapsList(const model::Game::Maps& maps) {
    json::array array;
    array.reserve(maps.size());

    for (const auto& map : maps) {
        array.push_back(json::value {
            {keys::MapsList::id, *map.GetId()},
            {keys::MapsList::name, map.GetName()},
        });
    }

    return array;
}

json::value SerializePlayers(const app::Application::Players& players) {
    json::object object;
    object.reserve(players.size());

    for (size_t i = 0; i < players.size(); ++i) {
        const auto& player = players[i];
        object[std::to_string(*player->GetId())] = json::object {
            {keys::Player::name, player->GetName()},
        };
    }

    return object;
}

json::value SerializePosition(const model::Point& position) {
    return json::array {position.x, position.y};
}

json::value SerializeSpeed(const model::Speed& speed) {
    return json::array {speed.x, speed.y};
}

json::value SerializeDirection(model::Direction direction) {
    switch (direction) {
        case model::Direction::WEST:
            return keys::Direction::west;
        case model::Direction::EAST:
            return keys::Direction::east;
        case model::Direction::NORTH:
            return keys::Direction::north;
        case model::Direction::SOUTH:
            return keys::Direction::south;
        case model::Direction::NONE:
            return keys::Direction::none;
        default:
            throw std::runtime_error("Incorrect direction");
    }
}

json::value SerializeGameState(
    const app::Application::Players& players,
    const app::GameSession::LostObjects& lost_objects
) {
    json::object json_players;

    for (const auto& player : players) {
        const model::Dog* dog = player->GetDog();

        json_players[std::to_string(*player->GetId())] = json::object {
            {keys::GameState::position, SerializePosition(dog->GetPosition())},
            {keys::GameState::speed, SerializeSpeed(dog->GetSpeed())},
            {keys::GameState::direction,
             SerializeDirection(dog->GetDirection())},
        };
    }

    json::object json_lost_objects;
    for (const auto& lost_object : lost_objects) {
        json_lost_objects[std::to_string(*lost_object.GetId())] = json::object {
            {keys::GameState::type, lost_object.GetType()},
            {keys::GameState::position,
             SerializePosition(lost_object.GetPosition())},
        };
    }

    return json::object {
        {keys::GameState::players, std::move(json_players)},
        {keys::GameState::lost_objects, std::move(json_lost_objects)},
    };
}

}  // namespace serde::json
