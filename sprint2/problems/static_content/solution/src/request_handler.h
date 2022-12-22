#pragma once
#include "http_server.h"
#include "json_serializer.h"
#include "model.h"

#include <boost/json.hpp>

#include <filesystem>
#include <sstream>

namespace http_handler {
namespace beast = boost::beast;
namespace http = beast::http;
namespace json = boost::json;
namespace sys = boost::system;
namespace fs = std::filesystem;

bool IsSubPath(fs::path path, fs::path base);

struct ContentType {
    ContentType() = delete;

    struct text {
        constexpr static std::string_view plain = "text/plain";
        constexpr static std::string_view html = "text/html";
        constexpr static std::string_view css = "text/css";
        constexpr static std::string_view javascript = "text/javascript";
    };

    struct application {
        constexpr static std::string_view json = "application/json";
        constexpr static std::string_view xml = "application/xml";
        constexpr static std::string_view octet_stream =
            "application/octet-stream";
    };

    struct image {
        constexpr static std::string_view png = "image/png";
        constexpr static std::string_view jpeg = "image/jpeg";
        constexpr static std::string_view gif = "image/gif";
        constexpr static std::string_view bmp = "image/bmp";
        constexpr static std::string_view icon = "image/vnd.microsoft.icon";
        constexpr static std::string_view tiff = "image/tiff";
        constexpr static std::string_view svg = "image/svg+xml";
    };

    struct audio {
        constexpr static std::string_view mpeg = "audio/mpeg";
    };
};

std::string_view GetMimeType(std::string_view path);

std::string DecodeUrl(std::string_view url);

class RequestHandler {
  public:
    explicit RequestHandler(
        model::Game& game,
        std::string static_path,
        std::string api_uri = "/api",
        std::string maps_uri = "/v1/maps"
    ) :
        game_ {game},
        static_path_(std::move(static_path)),
        api_uri_(std::move(api_uri)),
        maps_uri_(std::move(maps_uri)) {}

    RequestHandler(const RequestHandler&) = delete;
    RequestHandler& operator=(const RequestHandler&) = delete;

    template<typename Body, typename Allocator>
    using HttpRequest = http::request<Body, http::basic_fields<Allocator>>;

    template<typename Body, typename Allocator>
    using HttpResponse = http::response<Body, http::basic_fields<Allocator>>;

    using StringResponse = http::response<http::string_body>;

    using FileResponse = http::response<http::file_body>;

    template<typename Body, typename Allocator, typename Send>
    void operator()(HttpRequest<Body, Allocator>&& req, Send&& send) {
        if (req.target().starts_with(api_uri_)) {
            return HandleApiRequest(std::move(req), std::forward<Send>(send));
        } else {
            return HandleStaticRequest(
                std::move(req),
                std::forward<Send>(send)
            );
        }
    }

  private:
    template<typename Body, typename Allocator, typename Send>
    void HandleApiRequest(HttpRequest<Body, Allocator>&& req, Send&& send) {
        const auto make_json_response = [&](http::status status,
                                            std::string_view body) {
            HttpResponse<Body, Allocator> res(status, req.version());

            res.set(http::field::content_type, ContentType::application::json);
            res.body() = body;
            res.prepare_payload();
            res.keep_alive(req.keep_alive());

            return res;
        };

        const auto handle_bad_request = [&](std::string_view message =
                                                "Bad request") {
            return send(make_json_response(
                http::status::bad_request,
                json::serialize(json::value {
                    {"code", "badRequest"},
                    {"message", message},
                })
            ));
        };

        const auto handle_map_info_request = [&](std::string map_id) {
            using namespace std::literals;

            const model::Map* map =
                game_.FindMap(model::Map::Id(std::move(map_id)));

            if (!map) {
                return send(make_json_response(
                    http::status::not_found,
                    json::serialize(json::value {
                        {"code", "mapNotFound"},
                        {"message", "Map not found"},
                    })
                ));
            }

            return send(make_json_response(
                http::status::ok,
                json::serialize(json_serializer::SerializeMapInfo(*map))
            ));
        };

        const auto handle_maps_list_request = [&]() {
            return send(make_json_response(
                http::status::ok,
                json::serialize(
                    json_serializer::SerializeMapsList(game_.GetMaps())
                )
            ));
        };

        if (req.method() != http::verb::get &&
            req.method() != http::verb::head) {
            return handle_bad_request(
                "Server supports only GET and HEAD methods"
            );
        }

        std::string_view target = req.target();
        if (target.back() == '/') {
            target.remove_suffix(1);
        }

        target.remove_prefix(api_uri_.size());

        if (!target.starts_with(maps_uri_)) {
            return handle_bad_request();
        }

        target.remove_prefix(maps_uri_.size());

        if (target.empty()) {
            return handle_maps_list_request();
        }

        if (target.front() != '/') {
            return handle_bad_request();
        }

        target.remove_prefix(1);
        return handle_map_info_request(std::string(target));
    }

    template<typename Body, typename Allocator, typename Send>
    void HandleStaticRequest(HttpRequest<Body, Allocator>&& req, Send&& send) {
        const auto make_string_response = [&](http::status status,
                                              std::string_view body) {
            StringResponse res(status, req.version());

            res.set(http::field::content_type, ContentType::text::plain);
            res.body() = body;
            res.prepare_payload();
            res.keep_alive(req.keep_alive());

            return res;
        };

        const auto make_file_response = [&](http::status status,
                                            http::file_body::value_type&& body,
                                            std::string_view content_type) {
            FileResponse res(status, req.version());

            res.set(http::field::content_type, content_type);
            if (req.method() == http::verb::get) {
                res.body() = std::move(body);
                res.prepare_payload();
            } else {
                res.content_length(body.size());
            }
            res.keep_alive(req.keep_alive());

            return res;
        };

        const auto handle_bad_request = [&](std::string_view message) {
            return send(make_string_response(http::status::bad_request, message)
            );
        };

        const auto handle_not_found_request = [&](std::string_view message) {
            return send(make_string_response(http::status::not_found, message));
        };

        fs::path path = static_path_ + DecodeUrl(req.target());
        if (fs::is_directory(path)) {
            path /= "index.html";
        }

        if (!IsSubPath(path, static_path_)) {
            return handle_bad_request("Incorrect URL");
        }

        http::file_body::value_type file;
        if (sys::error_code ec;
            file.open(path.c_str(), beast::file_mode::read, ec), ec) {
            return handle_not_found_request("This page does not exist");
        }

        return send(make_file_response(
            http::status::ok,
            std::move(file),
            GetMimeType(path.c_str())
        ));
    }

    model::Game& game_;
    const std::string static_path_;
    const std::string api_uri_;
    const std::string maps_uri_;
};

}  // namespace http_handler
