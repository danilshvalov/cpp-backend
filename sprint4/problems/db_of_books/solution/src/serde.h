#pragma once

#include "model.h"

#include <vector>

#include <boost/json/value.hpp>

namespace serde {

namespace json = boost::json;

model::Book ParseBook(const json::object& object);

json::value SerializeBook(const model::Book& book);

json::value SerializeBooks(const std::vector<model::Book>& books);

}  // namespace serde
