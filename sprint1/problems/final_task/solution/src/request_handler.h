#pragma once
#include "http_server.h"
#include "json_serializer.h"
#include "model.h"

#include <boost/json.hpp>

namespace http_handler {
namespace beast = boost::beast;
namespace http = beast::http;
namespace json = boost::json;

struct ContentType {
    ContentType() = delete;
    constexpr static std::string_view APPLICATION_JSON = "application/json";
};

class RequestHandler {
   public:
    explicit RequestHandler(model::Game& game) : game_{game} {}

    RequestHandler(const RequestHandler&) = delete;
    RequestHandler& operator=(const RequestHandler&) = delete;

    template <typename Body, typename Allocator>
    using HttpRequest = http::request<Body, http::basic_fields<Allocator>>;

    template <typename Body, typename Allocator>
    using HttpResponse = http::response<Body, http::basic_fields<Allocator>>;

    template <typename Body, typename Allocator, typename Send>
    void operator()(HttpRequest<Body, Allocator>&& req, Send&& send) {
        const auto make_json_response = [&](http::status status,
                                            std::string_view body) {
            HttpResponse<Body, Allocator> response(status, req.version());

            response.set(http::field::content_type,
                         ContentType::APPLICATION_JSON);
            response.body() = body;
            response.content_length(body.size());
            response.keep_alive(req.keep_alive());

            return response;
        };

        const auto handle_bad_request = [&]() {
            return make_json_response(http::status::bad_request,
                                      json::serialize(json::value{
                                          {"code", "badRequest"},
                                          {"message", "Bad request"},
                                      }));
        };

        const auto get_map_info = [&](std::string map_id) {
            using namespace std::literals;

            const model::Map* map =
                game_.FindMap(model::Map::Id(std::move(map_id)));

            if (!map) {
                return make_json_response(http::status::not_found,
                                          json::serialize(json::value{
                                              {"code", "mapNotFound"},
                                              {"message", "Map not found"},
                                          }));
            }

            return make_json_response(
                http::status::ok,
                json::serialize(json_serializer::SerializeMapInfo(*map)));
        };

        const auto get_maps_list = [&]() {
            return make_json_response(
                http::status::ok,
                json::serialize(
                    json_serializer::SerializeMapsList(game_.GetMaps())));
        };

        std::string_view target = req.target();
        const std::string_view api_uri = "/api";
        std::string_view maps_uri = "/v1/maps";

        if (req.method() != http::verb::get) {
            return send(handle_bad_request());
        }

        if (!target.starts_with(api_uri)) {
            return send(handle_bad_request());
        }

        target.remove_prefix(api_uri.size());

        if (!target.starts_with(maps_uri)) {
            return send(handle_bad_request());
        }

        target.remove_prefix(maps_uri.size());

        if (target.empty() || target == "/") {
            return send(get_maps_list());
        }

        if (target.front() != '/') {
            return send(handle_bad_request());
        }

        return send(get_map_info(
            std::string{target.substr(1, target.find_last_not_of('/'))}));
    }

   private:
    model::Game& game_;
};

}  // namespace http_handler
