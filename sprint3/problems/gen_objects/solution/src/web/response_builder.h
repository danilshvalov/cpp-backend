#pragma once

#include "web/core.h"
#include "web/content_type.h"

#include <boost/json/serialize.hpp>
#include <boost/json/value.hpp>
#include <boost/json/parse.hpp>

namespace web {

namespace json = boost::json;

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

    void SetJsonBody(
        json::value value,
        http::status status = http::status::ok
    ) {
        SetStatus(status);
        std::string body = json::serialize(value);

        res_.set(
            http::field::content_type,
            web::ContentType::application::json
        );
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

}  // namespace web
