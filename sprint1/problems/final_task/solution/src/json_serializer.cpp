#include "json_serializer.h"

namespace json_serializer {

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
    const json::string_view height_key = "w";

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

}  // namespace json_serializer
