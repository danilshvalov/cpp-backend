#pragma once
#include "serde/json.h"
#include "model/map.h"
#include "app/app.h"
#include "web/core.h"
#include "web/server.h"
#include "web/content_type.h"
#include "web/utils.h"
#include "utils/path.h"

#include <boost/json/serialize.hpp>
#include <boost/json/value.hpp>
#include <boost/json/parse.hpp>

#include <filesystem>
#include <sstream>
#include <optional>

namespace handlers {

namespace beast = boost::beast;
namespace http = beast::http;
namespace json = boost::json;
namespace sys = boost::system;
namespace net = boost::asio;
namespace fs = std::filesystem;

using Strand = net::strand<net::io_context::executor_type>;

struct ResponseConfig {
    bool keep_alive = false;
    bool no_cache = false;
    std::string_view allow;
};

template<typename Body = http::string_body>
class ResponseBuilder {
  public:
    struct Config {
        bool add_body = true;
    };

    ResponseBuilder(Config config = {}) : config_(std::move(config)) {}

    ResponseBuilder(const web::StringRequest& req) {
        res_.version(req.version());
        res_.keep_alive(req.keep_alive());
        config_.add_body = req.method() != http::verb::head;
    }

    void Set(http::field field, std::string_view value) {
        res_.set(field, value);
    }

    void SetStatus(http::status status) {
        res_.result(status);
    }

    void SetNoCache() {
        res_.set(http::field::cache_control, "no-cache");
    }

    void SetAllow(std::string_view allow) {
        res_.set(http::field::allow, allow);
    }

    void SetJsonBody(json::value value, http::status status = http::status::ok) {
        SetStatus(status);
        std::string body = json::serialize(value);

        res_.set(http::field::content_type, web::ContentType::application::json);
        res_.content_length(body.size());

        if (config_.add_body) {
            res_.body() = std::move(body);
        }
    }

    http::response<Body> MakeResponse() const {
        return res_;
    }

    operator http::response<Body>() const {
        return res_;
    }

  protected:
    http::response<Body> res_;
    Config config_;
};

class JsonResponseBuilder : public ResponseBuilder<http::string_body> {
  public:
    using ResponseBuilder::ResponseBuilder;

    void SetBadRequest(std::string_view message = "Bad request") {
        SetJsonBody(
            json::value {
                {"code", "badRequest"},
                {"message", message},
            },
            http::status::bad_request
        );
    }

    void SetInvalidMethod(std::string_view message = "Invalid method") {
        SetJsonBody(
            json::value {
                {"code", "invalidMethod"},
                {"message", message},
            },
            http::status::method_not_allowed
        );
    }

    void SetInvalidArgument(std::string_view message = "Invalid argument") {
        SetJsonBody(
            json::value {
                {"code", "invalidArgument"},
                {"message", message},
            },
            http::status::bad_request
        );
    }

    void SetInvalidToken(std::string_view message = "Invalid token") {
        SetJsonBody(
            {
                {"code", "invalidToken"},
                {"message", message},
            },
            http::status::unauthorized
        );
    }

    void SetUnknownToken(std::string_view message = "Unknown token") {
        SetJsonBody(
            {
                {"code", "unknownToken"},
                {"message", message},
            },
            http::status::unauthorized
        );
    }

    void SetMapNotFound(std::string_view message = "Map not found") {
        SetJsonBody(
            json::value {
                {"code", "mapNotFound"},
                {"message", message},
            },
            http::status::not_found
        );
    }
};

class ApiHandlerImpl {
  public:
    ApiHandlerImpl(app::Application& app, web::StringRequest&& req) : app_(app), req_(std::move(req)) {}

    web::StringResponse HandleApiRequest() const {
        std::string_view target = req_.target();
        if (target.back() == '/') {
            target.remove_suffix(1);
        }

        target.remove_prefix(api_uri_.size());

        if (target == "/game/join") {
            return HandleGameJoinRequest();
        }

        if (target == "/game/players") {
            return HandlePlayersRequest();
        }

        if (target == "/game/state") {
            return HandleGameStateRequest();
        }

        if (target == "/game/player/action") {
            return HandlePlayerAction();
        }

        if (target == "/game/tick") {
            return HandleGameTick();
        }

        if (!target.starts_with(maps_uri_)) {
            JsonResponseBuilder res(req_);
            res.SetBadRequest();
            return res;
        }

        target.remove_prefix(maps_uri_.size());

        if (target.empty()) {
            return HandleMapsListRequest();
        }

        if (target.front() != '/') {
            JsonResponseBuilder res(req_);
            res.SetBadRequest();
            return res;
        }

        target.remove_prefix(1);
        return HandleMapInfoRequest(std::string(target));
    }

  private:
    web::StringResponse HandleMapInfoRequest(std::string map_id) const {
        const model::Map* map = app_.FindMap(model::Map::Id(std::move(map_id)));

        JsonResponseBuilder res(req_);
        res.SetNoCache();

        if (!map) {
            res.SetMapNotFound();
            return res;
        }

        res.SetJsonBody(serde::json::SerializeMapInfo(*map));
        return res;
    };

    web::StringResponse HandleMapsListRequest() const {
        JsonResponseBuilder res(req_);
        res.SetNoCache();
        res.SetJsonBody(serde::json::SerializeMapsList(app_.ListMaps()));
        return res;
    };

    web::StringResponse HandleGameJoinRequest() const {
        JsonResponseBuilder res(req_);
        res.SetNoCache();

        if (req_.method() != http::verb::post) {
            res.SetInvalidMethod("Only POST method is expected");
            res.SetAllow("POST");
            return res;
        }

        try {
            json::value data = json::parse(req_.body()).as_object();

            model::Map::Id map_id {std::string(data.at("mapId").as_string())};
            std::string name {data.at("userName").as_string()};

            if (name.empty()) {
                res.SetInvalidArgument("Invalid name");
                return res;
            }

            app::JoinGameResult info = app_.JoinGame(std::move(map_id), std::move(name));

            res.SetJsonBody({
                {"authToken", *info.token},
                {"playerId", *info.id},
            });
            return res;
        } catch (const std::invalid_argument&) {
            // TODO: rework
            res.SetMapNotFound();
            return res;
        } catch (const std::exception&) {
            res.SetInvalidArgument("Join game request parse error");
            return res;
        }
    }

    web::StringResponse HandlePlayersRequest() const {
        JsonResponseBuilder res(req_);
        res.SetNoCache();

        if (auto method = req_.method(); method != http::verb::get && method != http::verb::head) {
            res.SetInvalidMethod();
            res.SetAllow("GET,HEAD");
            return res;
        }

        return ExecuteAuthorized([&](const app::Token& token) {
            const auto& players = app_.GetPlayers(token);
            res.SetJsonBody(serde::json::SerializePlayers(players));
            return res;
        });
    }

    web::StringResponse HandleGameStateRequest() const {
        JsonResponseBuilder res(req_);
        res.SetNoCache();

        if (auto method = req_.method(); method != http::verb::get && method != http::verb::head) {
            res.SetInvalidMethod();
            res.SetAllow("GET,HEAD");
            return res;
        }

        return ExecuteAuthorized([&](const app::Token& token) {
            const auto& players = app_.GetPlayers(token);
            res.SetJsonBody(serde::json::SerializeGameState(players));
            return res;
        });
    }

    web::StringResponse HandlePlayerAction() const {
        JsonResponseBuilder res(req_);
        res.SetNoCache();

        if (req_.method() != http::verb::post) {
            res.SetInvalidMethod("Only POST method is expected");
            res.SetAllow("POST");
            return res;
        }

        if (auto it = req_.find(http::field::content_type);
            it == req_.end() || it->value() != web::ContentType::application::json) {
            res.SetInvalidArgument("Invalid content type");
            return res;
        }

        return ExecuteAuthorized([&](const app::Token& token) {
            try {
                // TODO: remove
                // std::cout << req_.body() << std::endl;
                auto body = json::parse(req_.body()).as_object();
                // std::cout << body << std::endl;
                // std::cout << static_cast<int>(serde::json::ParseDirection(body)) << std::endl;
                app_.MovePlayer(token, serde::json::ParseDirection(body));
                res.SetJsonBody(json::object {});
                return res;
            } catch (const std::exception&) {
                res.SetInvalidArgument("Failed to parse action");
                return res;
            }
        });
    }

    web::StringResponse HandleGameTick() const {
        JsonResponseBuilder res(req_);
        res.SetNoCache();

        if (req_.method() != http::verb::post) {
            res.SetInvalidMethod("Only POST method is expected");
            res.SetAllow("POST");
            return res;
        }

        if (auto it = req_.find(http::field::content_type);
            it == req_.end() || it->value() != web::ContentType::application::json) {
            res.SetInvalidArgument("Invalid content type");
            return res;
        }

        try {
            std::cout << req_.body() << std::endl;
            auto body = json::parse(req_.body()).as_object();
            std::cout << serde::json::ParseTick(body).count() << std::endl;
            app_.UpdateGameState(serde::json::ParseTick(body));
            res.SetJsonBody(json::object {});
            return res;
        } catch (const std::exception& e) {
            res.SetInvalidArgument("Failed to parse tick request JSON");
            return res;
        }
    }

    template<typename Fn>
    web::StringResponse ExecuteAuthorized(Fn&& action) const {
        JsonResponseBuilder res(req_);
        res.SetNoCache();

        auto token = web::TryExtractToken(req_);
        if (!token) {
            res.SetInvalidToken("Authorization header is missing");
            return res;
        }

        if (!app_.HasPlayer(*token)) {
            res.SetUnknownToken("Player token has not been found");
            return res;
        }

        return action(*token);
    }

    app::Application& app_;
    web::StringRequest req_;

    std::string api_uri_ = "/api/v1";
    std::string maps_uri_ = "/maps";
};

class ApiHandler {
  public:
    ApiHandler(app::Application& app) : app_(app) {}

    bool IsApiRequest(const web::StringRequest& req) {
        return req.target().starts_with(uri_prefix_);
    }

    web::StringResponse HandleApiRequest(web::StringRequest&& req) {
        return ApiHandlerImpl(app_, std::move(req)).HandleApiRequest();
    }

  private:
    app::Application& app_;
    std::string uri_prefix_ = "/api";
};

class RequestHandler : public std::enable_shared_from_this<RequestHandler> {
  public:
    explicit RequestHandler(
        app::Application& app,
        std::string static_path,
        std::string api_uri = "/api",
        std::string maps_uri = "/v1/maps"
    ) :
        app_(app),
        api_handler_(app),
        static_path_(std::move(static_path)),
        api_uri_(std::move(api_uri)),
        maps_uri_(std::move(maps_uri)) {}

    RequestHandler(const RequestHandler&) = delete;
    RequestHandler& operator=(const RequestHandler&) = delete;

    template<typename Body, typename Allocator, typename Send>
    void operator()(web::HttpRequest<Body, Allocator>&& req, Send&& send) {
        if (req.target().starts_with(api_uri_)) {
            // send(api_handler_.HandleApiRequest(std::move(req)));
            return net::dispatch(
                app_.GetStrand(),
                [self = shared_from_this(), req = std::move(req), send = std::forward<Send>(send)]() mutable {
                    send(self->api_handler_.HandleApiRequest(std::move(req)));
                }
            );
        } else {
            return HandleStaticRequest(std::move(req), std::forward<Send>(send));
        }
    }

  private:
    template<typename Body, typename Allocator, typename Send>
    void HandleStaticRequest(web::HttpRequest<Body, Allocator>&& req, Send&& send) {
        const auto make_string_response = [&](http::status status, std::string_view body) {
            web::StringResponse res(status, req.version());

            res.set(http::field::content_type, web::ContentType::text::plain);
            res.body() = body;
            res.prepare_payload();
            res.keep_alive(req.keep_alive());

            return res;
        };

        const auto make_file_response =
            [&](http::status status, http::file_body::value_type&& body, std::string_view content_type) {
                web::FileResponse res(status, req.version());

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
            return send(make_string_response(http::status::bad_request, message));
        };

        const auto handle_not_found_request = [&](std::string_view message) {
            return send(make_string_response(http::status::not_found, message));
        };

        fs::path path = static_path_ + web::DecodeUrl(req.target());
        if (fs::is_directory(path)) {
            path /= "index.html";
        }

        if (!utils::IsSubPath(path, static_path_)) {
            return handle_bad_request("Incorrect URL");
        }

        http::file_body::value_type file;
        if (sys::error_code ec; file.open(path.c_str(), beast::file_mode::read, ec), ec) {
            return handle_not_found_request("This page does not exist");
        }

        return send(make_file_response(http::status::ok, std::move(file), web::GetMimeType(path.c_str())));
    }

    app::Application& app_;
    ApiHandler api_handler_;
    const std::string static_path_;
    const std::string api_uri_;
    const std::string maps_uri_;
};

}  // namespace handlers
