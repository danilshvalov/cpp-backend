#include "handlers/api_handler.h"
#include "serde/json.h"
#include "web/response_builder.h"
#include "web/utils.h"

#include <boost/url/url_view.hpp>
#include <boost/url/parse.hpp>

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

        if (target == "/game/records") {
            return HandleRecordsAction();
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
        web::JsonResponseBuilder res(req_);
        res.SetNoCache();

        if (auto method = req_.method();
            method != http::verb::get && method != http::verb::head) {
            res.SetInvalidMethod();
            res.SetAllow("GET,HEAD");
            return res;
        }

        const model::Map* map = app_.FindMap(model::Map::Id(std::move(map_id)));

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
            app::JoinGameData data = serde::json::ParseJoinGameData(
                json::parse(req_.body()).as_object()
            );

            if (auto* map = app_.FindMap(data.map_id); !map) {
                res.SetMapNotFound();
                return res;
            }

            app::JoinGameResult info = app_.JoinGame(std::move(data));

            res.SetJsonBody({
                {"authToken", *info.token},
                {"playerId", *info.id},
            });
            return res;
        } catch (const std::invalid_argument& e) {
            res.SetInvalidArgument(e.what());
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
            res.SetJsonBody(serde::json::SerializeGameState(
                app_.GetPlayers(token), app_.GetLostObjects(token)
            ));
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

        if (auto it = req_.find(http::field::content_type);
            it == req_.end() ||
            it->value() != web::ContentType::application::json) {
            res.SetInvalidArgument("Invalid content type");
            return res;
        }

        return ExecuteAuthorized([&](const app::Token& token) {
            try {
                auto body = json::parse(req_.body()).as_object();
                app_.MovePlayer(token, serde::json::ParseDirection(body));
                res.SetJsonBody(json::object{});
                return res;
            } catch (const std::exception&) {
                res.SetInvalidArgument("Failed to parse action");
                return res;
            }
        });
    }

    web::StringResponse HandleRecordsAction() const {
        web::JsonResponseBuilder res(req_);
        res.SetNoCache();

        if (auto method = req_.method();
            method != http::verb::get && method != http::verb::head) {
            res.SetInvalidMethod();
            res.SetAllow("GET,HEAD");
            return res;
        }

        const std::string_view start_key = "start";
        const std::string_view max_items_key = "maxItems";

        app::PlayerRecordsData data;

        auto params = boost::urls::url_view{req_.target()}.params();
        if (auto it = params.find(start_key); it != params.end()) {
            data.start = std::stoi((*it).value);
        }
        if (auto it = params.find(max_items_key); it != params.end()) {
            data.max_items = std::stoi((*it).value);
        }

        const auto& records = app_.GetPlayerRecords(data);
        res.SetJsonBody(serde::json::SerializePlayerRecords(records));
        return res;
    }

    web::StringResponse HandleGameTick() const {
        web::JsonResponseBuilder res(req_);
        res.SetNoCache();

        if (req_.method() != http::verb::post) {
            res.SetInvalidMethod("Only POST method is expected");
            res.SetAllow("POST");
            return res;
        }

        if (auto it = req_.find(http::field::content_type);
            it == req_.end() ||
            it->value() != web::ContentType::application::json) {
            res.SetInvalidArgument("Invalid content type");
            return res;
        }

        try {
            auto body = json::parse(req_.body()).as_object();
            app_.UpdateGameState(serde::json::ParseTick(body));
            res.SetJsonBody(json::object{});
            return res;
        } catch (const std::exception& e) {
            res.SetInvalidArgument("Failed to parse tick request JSON");
            return res;
        }
    }

    template <typename Fn>
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

} // namespace handlers
