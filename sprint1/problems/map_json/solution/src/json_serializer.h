#pragma once

#include <boost/json.hpp>

#include "model.h"

namespace json_serializer {
namespace json = boost::json;

json::value SerializeMap(const model::Map& map);

json::value SerializeMapsInfo(const model::Game::Maps& maps);

}  // namespace json_serializer
