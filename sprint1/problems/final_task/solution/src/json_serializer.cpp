#include "json_serializer.h"

namespace json_serializer {

json::value SerializeRoads(const model::Map::Roads& roads) {
    json::array array;
    array.reserve(roads.size());

    for (const auto& road : roads) {
        const auto& start = road.GetStart();
        const auto& end = road.GetEnd();

        json::object object{
            {"x0", start.x},
            {"y0", start.y},
        };

        if (road.IsHorizontal()) {
            object["x1"] = end.x;
        } else {
            object["y1"] = end.y;
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

        array.push_back(json::value{
            {"x", position.x},
            {"y", position.y},
            {"w", size.width},
            {"h", size.height},
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

        array.push_back(json::value{
            {"id", *office.GetId()},
            {"x", pos.x},
            {"y", pos.y},
            {"offsetX", offset.dx},
            {"offsetY", offset.dy},
        });
    }

    return array;
}

json::value SerializeMap(const model::Map& map) {
    return json::value{
        {"id", *map.GetId()},
        {"name", map.GetName()},
        {"roads", SerializeRoads(map.GetRoads())},
        {"buildings", SerializeBuildings(map.GetBuildings())},
        {"offices", SerializeOffices(map.GetOffices())},
    };
}

json::value SerializeMapsInfo(const model::Game::Maps& maps) {
    json::array array;
    array.reserve(maps.size());

    for (const auto& map : maps) {
        array.push_back(json::value{
            {"id", *map.GetId()},
            {"name", map.GetName()},
        });
    }

    return array;
}

}  // namespace json_serializer
