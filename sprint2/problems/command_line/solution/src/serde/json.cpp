#include "serde/json.h"

#include <boost/json/parse.hpp>
#include <boost/json/value.hpp>
#include <boost/json/value_to.hpp>

#include <fstream>
#include <iterator>

namespace serde::json {

namespace json = boost::json;

using namespace model;

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
    const json::string_view move_key = "move";

    const auto& move = object.at(move_key).as_string();

    if (move == "L") {
        return Direction::WEST;
    } else if (move == "R") {
        return Direction::EAST;
    } else if (move == "U") {
        return Direction::NORTH;
    } else if (move == "D") {
        return Direction::SOUTH;
    } else if (move.empty()) {
        return Direction::NONE;
    } else {
        throw std::invalid_argument("Incorrect direction of movement");
    }
}

std::chrono::milliseconds ParseTick(const json::object& object) {
    const json::string_view time_delta_key = "timeDelta";
    return std::chrono::milliseconds(object.at(time_delta_key).as_int64());
}

Road ParseRoad(const json::object& object) {
    const json::string_view start_x_key = "x0";
    const json::string_view start_y_key = "y0";
    const json::string_view end_x_key = "x1";
    const json::string_view end_y_key = "y1";

    Point start {
        Coord(object.at(start_x_key).as_int64()),
        Coord(object.at(start_y_key).as_int64()),
    };

    if (object.contains(end_x_key)) {
        Coord end_x = object.at(end_x_key).as_int64();
        return Road(Road::HORIZONTAL, start, end_x);
    } else {
        Coord end_y = object.at(end_y_key).as_int64();
        return Road(Road::VERTICAL, start, end_y);
    }
}

Building ParseBuilding(const json::object& object) {
    const json::string_view x_key = "x";
    const json::string_view y_key = "y";
    const json::string_view width_key = "w";
    const json::string_view height_key = "h";

    return Building {Rectangle {
        Point {
            Coord(object.at(x_key).as_int64()),
            Coord(object.at(y_key).as_int64()),
        },
        Size {
            Dimension(object.at(width_key).as_int64()),
            Dimension(object.at(height_key).as_int64()),
        },
    }};
}

Office ParseOffice(const json::object& object) {
    const json::string_view id_key = "id";
    const json::string_view x_key = "x";
    const json::string_view y_key = "y";
    const json::string_view offset_x_key = "offsetX";
    const json::string_view offset_y_key = "offsetY";

    return Office {
        Office::Id(json::value_to<std::string>(object.at(id_key))),
        Point {
            Coord(object.at(x_key).as_int64()),
            Coord(object.at(y_key).as_int64()),
        },
        Offset {
            Dimension(object.at(offset_x_key).as_int64()),
            Dimension(object.at(offset_y_key).as_int64()),
        },
    };
}

Map ParseMap(const json::object& object, Map::Config config) {
    const json::string_view id_key = "id";
    const json::string_view name_key = "name";
    const json::string_view roads_key = "roads";
    const json::string_view buildings_key = "buildings";
    const json::string_view offices_key = "offices";

    if (auto it = object.find("dogSpeed"); it != object.end()) {
        config.dog_speed = it->value().as_double();
    }

    Map map(
        Map::Id(json::value_to<std::string>(object.at(id_key))),
        json::value_to<std::string>(object.at(name_key)),
        config
    );

    for (const auto& node : object.at(roads_key).as_array()) {
        map.AddRoad(ParseRoad(node.as_object()));
    }

    for (const auto& node : object.at(buildings_key).as_array()) {
        map.AddBuilding(ParseBuilding(node.as_object()));
    }

    for (const auto& node : object.at(offices_key).as_array()) {
        map.AddOffice(ParseOffice(node.as_object()));
    }

    return map;
}

Game LoadGame(const std::filesystem::path& json_path) {
    json::object document = LoadJson(json_path).as_object();
    Game game;

    const json::string_view maps_key = "maps";

    Map::Config default_config;

    if (auto it = document.find("defaultDogSpeed"); it != document.end()) {
        default_config.dog_speed = it->value().as_double();
    }

    for (const auto& node : document.at(maps_key).as_array()) {
        game.AddMap(ParseMap(node.as_object(), default_config));
    }

    return game;
}

json::value SerializeRoads(const model::Map::Roads& roads) {
    const json::string_view start_x_key = "x0";
    const json::string_view start_y_key = "y0";
    const json::string_view end_x_key = "x1";
    const json::string_view end_y_key = "y1";

    json::array array;
    array.reserve(roads.size());

    for (const auto& road : roads) {
        const auto& start = road.GetStart();
        const auto& end = road.GetEnd();

        json::object object {
            {start_x_key, start.x},
            {start_y_key, start.y},
        };

        if (road.IsHorizontal()) {
            object[end_x_key] = end.x;
        } else {
            object[end_y_key] = end.y;
        }

        array.push_back(std::move(object));
    }

    return array;
}

json::value SerializeBuildings(const model::Map::Buildings& buildings) {
    const json::string_view x_key = "x";
    const json::string_view y_key = "y";
    const json::string_view width_key = "w";
    const json::string_view height_key = "h";

    json::array array;
    array.reserve(buildings.size());

    for (const auto& building : buildings) {
        const auto& [position, size] = building.GetBounds();

        array.push_back(json::value {
            {x_key, position.x},
            {y_key, position.y},
            {width_key, size.width},
            {height_key, size.height},
        });
    }

    return array;
}

json::value SerializeOffices(const model::Map::Offices& offices) {
    const json::string_view id_key = "id";
    const json::string_view x_key = "x";
    const json::string_view y_key = "y";
    const json::string_view offset_x_key = "offsetX";
    const json::string_view offset_y_key = "offsetY";

    json::array array;
    array.reserve(offices.size());

    for (const auto& office : offices) {
        const auto& pos = office.GetPosition();
        const auto& offset = office.GetOffset();

        array.push_back(json::value {
            {id_key, *office.GetId()},
            {x_key, pos.x},
            {y_key, pos.y},
            {offset_x_key, offset.dx},
            {offset_y_key, offset.dy},
        });
    }

    return array;
}

json::value SerializeMapInfo(const model::Map& map) {
    const json::string_view id_key = "id";
    const json::string_view name_key = "name";
    const json::string_view roads_key = "roads";
    const json::string_view buildings_key = "buildings";
    const json::string_view offices_key = "offices";

    return json::value {
        {id_key, *map.GetId()},
        {name_key, map.GetName()},
        {roads_key, SerializeRoads(map.GetRoads())},
        {buildings_key, SerializeBuildings(map.GetBuildings())},
        {offices_key, SerializeOffices(map.GetOffices())},
    };
}

json::value SerializeMapsList(const model::Game::Maps& maps) {
    const json::string_view id_key = "id";
    const json::string_view name_key = "name";

    json::array array;
    array.reserve(maps.size());

    for (const auto& map : maps) {
        array.push_back(json::value {
            {id_key, *map.GetId()},
            {name_key, map.GetName()},
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
            {"name", player->GetName()},
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
            return "L";
        case model::Direction::EAST:
            return "R";
        case model::Direction::NORTH:
            return "U";
        case model::Direction::SOUTH:
            return "D";
        case model::Direction::NONE:
            return "";
        default:
            return "";
    }
}

json::value SerializeGameState(const app::Application::Players& players) {
    json::object object;
    object.reserve(players.size());

    for (size_t i = 0; i < players.size(); ++i) {
        const auto& player = players[i];
        const model::Dog* dog = player->GetDog();

        object[std::to_string(*player->GetId())] = json::object {
            {"pos", SerializePosition(dog->GetPosition())},
            {"speed", SerializeSpeed(dog->GetSpeed())},
            {"dir", SerializeDirection(dog->GetDirection())},
        };
    }

    return json::object {
        {"players", std::move(object)},
    };
}

}  // namespace serde::json
