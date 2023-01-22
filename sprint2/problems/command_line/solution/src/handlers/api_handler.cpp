#include "handlers/api_handler.h"
#include "serde/json.h"
#include "web/response_builder.h"
#include "web/utils.h"

namespace handlers {

namespace beast = boost::beast;
namespace http = beast::http;
namespace json = boost::json;

class ApiHandlerImpl {
  public:
    ApiHandlerImpl(app::Application& app, web::StringRequest&& req) :
        app_(app),
        req_(std::move(req)) {}

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

        if (target == "/game/tick" && !app_.HasTickPeriod()) {
            return HandleGameTick();
        }

        if (!target.starts_with(maps_uri_)) {
            web::JsonResponseBuilder res(req_);
            res.SetBadRequest();
            return res;
        }

        target.remove_prefix(maps_uri_.size());

        if (target.empty()) {
            return HandleMapsListRequest();
        }

        if (target.front() != '/') {
            web::JsonResponseBuilder res(req_);
            res.SetBadRequest();
            return res;
        }

        target.remove_prefix(1);
        return HandleMapInfoRequest(std::string(target));
    }

  private:
    web::StringResponse HandleMapInfoRequest(std::string map_id) const {
        const model::Map* map = app_.FindMap(model::Map::Id(std::move(map_id)));

        web::JsonResponseBuilder res(req_);
        res.SetNoCache();

        if (!map) {
            res.SetMapNotFound();
            return res;
        }

        res.SetJsonBody(serde::json::SerializeMapInfo(*map));
        return res;
    };

    web::StringResponse HandleMapsListRequest() const {
        web::JsonResponseBuilder res(req_);
        res.SetNoCache();
        res.SetJsonBody(serde::json::SerializeMapsList(app_.ListMaps()));
        return res;
    };

    web::StringResponse HandleGameJoinRequest() const {
        web::JsonResponseBuilder res(req_);
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

            if (auto* map = app_.FindMap(map_id); !map) {
                res.SetMapNotFound();
                return res;
            }

            app::JoinGameResult info =
                app_.JoinGame(std::move(map_id), std::move(name));

            res.SetJsonBody({
                {"authToken", *info.token},
                {"playerId", *info.id},
            });
            return res;
        } catch (const std::exception&) {
            res.SetInvalidArgument("Join game request parse error");
            return res;
        }
    }

    web::StringResponse HandlePlayersRequest() const {
        web::JsonResponseBuilder res(req_);
        res.SetNoCache();

        if (auto method = req_.method();
            method != http::verb::get && method != http::verb::head) {
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
        web::JsonResponseBuilder res(req_);
        res.SetNoCache();

        if (auto method = req_.method();
            method != http::verb::get && method != http::verb::head) {
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
        web::JsonResponseBuilder res(req_);
        res.SetNoCache();

        if (req_.method() != http::verb::post) {
            res.SetInvalidMethod("Only POST method is expected");
            res.SetAllow("POST");
            return res;
        }

        if (auto it = req_.find(http::field::content_type); it == req_.end() ||
            it->value() != web::ContentType::application::json) {
            res.SetInvalidArgument("Invalid content type");
            return res;
        }

        return ExecuteAuthorized([&](const app::Token& token) {
            try {
                auto body = json::parse(req_.body()).as_object();
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
        web::JsonResponseBuilder res(req_);
        res.SetNoCache();

        if (req_.method() != http::verb::post) {
            res.SetInvalidMethod("Only POST method is expected");
            res.SetAllow("POST");
            return res;
        }

        if (auto it = req_.find(http::field::content_type); it == req_.end() ||
            it->value() != web::ContentType::application::json) {
            res.SetInvalidArgument("Invalid content type");
            return res;
        }

        try {
            auto body = json::parse(req_.body()).as_object();
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
        web::JsonResponseBuilder res(req_);
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

ApiHandler::ApiHandler(app::Application& app) : app_(app) {}

bool ApiHandler::IsApiRequest(const web::StringRequest& req) {
    return req.target().starts_with(uri_prefix_);
}

web::StringResponse ApiHandler::HandleApiRequest(web::StringRequest&& req) {
    return ApiHandlerImpl(app_, std::move(req)).HandleApiRequest();
}

}  // namespace handlers
