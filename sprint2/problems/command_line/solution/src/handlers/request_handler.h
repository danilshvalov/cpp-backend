#pragma once

#include "handlers/api_handler.h"
#include "web/content_type.h"
#include "web/utils.h"
#include "utils/path.h"

#include <filesystem>

namespace handlers {

namespace beast = boost::beast;
namespace http = beast::http;
namespace sys = boost::system;
namespace net = boost::asio;
namespace fs = std::filesystem;

class RequestHandler : public std::enable_shared_from_this<RequestHandler> {
  public:
    using Strand = net::strand<net::io_context::executor_type>;

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
            return net::dispatch(
                app_.GetStrand(),
                [self = shared_from_this(),
                 req = std::move(req),
                 send = std::forward<Send>(send)]() mutable {
                    send(self->api_handler_.HandleApiRequest(std::move(req)));
                }
            );
        } else {
            return HandleStaticRequest(
                std::move(req),
                std::forward<Send>(send)
            );
        }
    }

  private:
    template<typename Body, typename Allocator, typename Send>
    void HandleStaticRequest(
        web::HttpRequest<Body, Allocator>&& req,
        Send&& send
    ) {
        const auto make_string_response = [&](http::status status,
                                              std::string_view body) {
            web::StringResponse res(status, req.version());

            res.set(http::field::content_type, web::ContentType::text::plain);
            res.body() = body;
            res.prepare_payload();
            res.keep_alive(req.keep_alive());

            return res;
        };

        const auto make_file_response = [&](http::status status,
                                            http::file_body::value_type&& body,
                                            std::string_view content_type) {
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
            return send(make_string_response(http::status::bad_request, message)
            );
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
        if (sys::error_code ec;
            file.open(path.c_str(), beast::file_mode::read, ec), ec) {
            return handle_not_found_request("This page does not exist");
        }

        return send(make_file_response(
            http::status::ok,
            std::move(file),
            web::GetMimeType(path.c_str())
        ));
    }

    app::Application& app_;
    ApiHandler api_handler_;
    const std::string static_path_;
    const std::string api_uri_;
    const std::string maps_uri_;
};

}  // namespace handlers
