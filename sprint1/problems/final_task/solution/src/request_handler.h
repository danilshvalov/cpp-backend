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
        std::string_view target = req.target();
        std::string_view maps_api_uri = "/api/v1/maps";

        if (target.starts_with(maps_api_uri)) {
            if (target == maps_api_uri) {
                HandleMapsListRequest(std::move(req), std::move(send));
            } else {
                HandleMapRequest(std::move(req), std::move(send));
            }
        } else if (target.starts_with("/api/")) {
            HandleUnknownRequest(std::move(req), std::move(send));
        }
    }

   private:
    template <typename Body, typename Allocator>
    static HttpResponse<Body, Allocator> MakeJsonResponse(http::status status,
                                                          unsigned version,
                                                          std::string_view body,
                                                          bool keep_alive) {
        HttpResponse<Body, Allocator> response(status, version);

        response.set(http::field::content_type, ContentType::APPLICATION_JSON);
        response.body() = body;
        response.content_length(body.size());
        response.keep_alive(keep_alive);

        return response;
    }

    template <typename Body, typename Allocator, typename Send>
    void HandleMapsListRequest(HttpRequest<Body, Allocator>&& req,
                               Send&& send) {
        std::string body = json::serialize(
            json_serializer::SerializeMapsList(game_.GetMaps()));

        send(MakeJsonResponse<Body, Allocator>(http::status::ok, req.version(),
                                               body, req.keep_alive()));
    }

    template <typename Body, typename Allocator, typename Send>
    void HandleMapRequest(HttpRequest<Body, Allocator>&& req, Send&& send) {
        using namespace std::literals;

        std::string map_id{req.target().substr("/api/v1/maps/"sv.size())};
        const model::Map* map = game_.FindMap(model::Map::Id(map_id));

        if (!map) {
            std::string body = json::serialize(json::value{
                {"code", "mapNotFound"},
                {"message", "Map not found"},
            });

            send(MakeJsonResponse<Body, Allocator>(http::status::not_found,
                                                   req.version(), body,
                                                   req.keep_alive()));
        } else {
            std::string body =
                json::serialize(json_serializer::SerializeMapInfo(*map));

            send(MakeJsonResponse<Body, Allocator>(
                http::status::ok, req.version(), body, req.keep_alive()));
        }
    }

    template <typename Body, typename Allocator, typename Send>
    void HandleUnknownRequest(HttpRequest<Body, Allocator>&& req, Send&& send) {
        std::string body = json::serialize(json::value{
            {"code", "badRequest"},
            {"message", "Bad request"},
        });

        send(MakeJsonResponse<Body, Allocator>(
            http::status::bad_request, req.version(), body, req.keep_alive()));
    }

    model::Game& game_;
};

}  // namespace http_handler
