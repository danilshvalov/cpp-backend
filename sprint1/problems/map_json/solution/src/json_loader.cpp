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
        throw std::runtime_error("Cannot open configuration file " +
                                 json_path.string());
    }

    std::string json_string(std::istreambuf_iterator<char>(json_file.rdbuf()),
                            std::istreambuf_iterator<char>());

    return json::parse(json_string);
}

Road ParseRoad(const json::object& object) {
    Point start{
        Coord(object.at("x0").as_int64()),
        Coord(object.at("y0").as_int64()),
    };

    if (object.contains("x1")) {
        Coord end_x = object.at("x1").as_int64();
        return Road(Road::HORIZONTAL, start, end_x);
    } else {
        Coord end_y = object.at("y1").as_int64();
        return Road(Road::VERTICAL, start, end_y);
    }
}

Building ParseBuilding(const json::object& object) {
    return Building{Rectangle{
        Point{
            Coord(object.at("x").as_int64()),
            Coord(object.at("y").as_int64()),
        },
        Size{
            Dimension(object.at("w").as_int64()),
            Dimension(object.at("h").as_int64()),
        },
    }};
}

Office ParseOffice(const json::object& object) {
    return Office{
        Office::Id(json::value_to<std::string>(object.at("id"))),
        Point{
            Coord(object.at("x").as_int64()),
            Coord(object.at("y").as_int64()),
        },
        Offset{
            Dimension(object.at("offsetX").as_int64()),
            Dimension(object.at("offsetY").as_int64()),
        },
    };
}

Map ParseMap(const json::object& object) {
    Map map{
        Map::Id(json::value_to<std::string>(object.at("id"))),
        json::value_to<std::string>(object.at("name")),
    };

    for (const auto& node : object.at("roads").as_array()) {
        map.AddRoad(ParseRoad(node.as_object()));
    }

    for (const auto& node : object.at("buildings").as_array()) {
        map.AddBuilding(ParseBuilding(node.as_object()));
    }

    for (const auto& node : object.at("offices").as_array()) {
        map.AddOffice(ParseOffice(node.as_object()));
    }

    return map;
}

Game LoadGame(const std::filesystem::path& json_path) {
    auto document = LoadJson(json_path);
    Game game;

    for (const auto& node : document.at("maps").as_array()) {
        game.AddMap(ParseMap(node.as_object()));
    }

    return game;
}

}  // namespace json_loader
