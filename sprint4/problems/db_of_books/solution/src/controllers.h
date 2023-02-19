#pragma once

#include "database.h"
#include "serde.h"

#include <optional>

#include <pqxx/connection>
#include <boost/json/value.hpp>
#include <boost/json/parse.hpp>

namespace controllers {

namespace json = boost::json;

class JsonDatabaseController {
  public:
    JsonDatabaseController(pqxx::connection& connection) :
        book_table_(connection) {}

    std::optional<json::value> Process(const json::value& value) {
        try {
            const auto& object = value.as_object();
            const auto& action = object.at("action").as_string();
            const auto& payload = object.at("payload").as_object();

            if (action == "add_book") {
                book_table_.AddBook(serde::ParseBook(payload));
                return json::object({{"result", true}});
            } else if (action == "all_books") {
                return serde::SerializeBooks(book_table_.GetBooks());
            } else if (action == "exit") {
                return std::nullopt;
            }
        } catch (const std::exception& e) {
            return json::object({{"result", false}});
        }

        return json::object({{"result", false}});
    }

  private:
    database::BookTable book_table_;
};

}  // namespace controllers
