#include "json_loader.h"

#include <boost/json.hpp>
#include <fstream>
#include <iterator>

namespace json_loader {

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

Map ParseMap(const json::object& object) {
    const json::string_view id_key = "id";
    const json::string_view name_key = "name";
    const json::string_view roads_key = "roads";
    const json::string_view buildings_key = "buildings";
    const json::string_view offices_key = "offices";

    Map map {
        Map::Id(json::value_to<std::string>(object.at(id_key))),
        json::value_to<std::string>(object.at(name_key)),
    };

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
    auto document = LoadJson(json_path);
    Game game;

    const json::string_view maps_key = "maps";

    for (const auto& node : document.at(maps_key).as_array()) {
        game.AddMap(ParseMap(node.as_object()));
    }

    return game;
}

}  // namespace json_loader
