#pragma once
#include "sdk.h"
#define BOOST_BEAST_USE_STD_STRING_VIEW

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>

#include <chrono>

#include "json_logger.h"

namespace http_server {

namespace net = boost::asio;
using tcp = net::ip::tcp;
namespace beast = boost::beast;
namespace http = beast::http;
namespace sys = boost::system;
namespace logging = boost::log;
namespace json = boost::json;

void ReportError(sys::error_code ec, std::string_view what);

class SessionBase {
  public:
    SessionBase(const SessionBase&) = delete;
    SessionBase& operator=(const SessionBase&) = delete;

    void Run();

    template<typename Body, typename Fields>
    void Write(http::response<Body, Fields>&& response) {
        auto safe_response =
            std::make_shared<http::response<Body, Fields>>(std::move(response));

        auto self = GetSharedThis();
        http::async_write(
            stream_,
            *safe_response,
            [safe_response, self](sys::error_code ec, size_t bytes_written) {
                self->OnWrite(safe_response->need_eof(), ec, bytes_written);
            }
        );
    }

  protected:
    template<typename Body, typename Allocator>
    using HttpRequest = http::request<Body, http::basic_fields<Allocator>>;

    template<typename Body, typename Allocator>
    using HttpResponse = http::response<Body, http::basic_fields<Allocator>>;

    using StringRequest = http::request<http::string_body>;

    explicit SessionBase(tcp::socket&& socket) : stream_(std::move(socket)) {}

    ~SessionBase() = default;

    void Read();

    void OnRead(sys::error_code ec, size_t bytes_read);

    void OnWrite(bool close, sys::error_code ec, size_t bytes_written);

    void Close();

    tcp::endpoint GetRemoteEndpoint() const {
        return stream_.socket().remote_endpoint();
    }

    virtual void HandleRequest(StringRequest&& request) = 0;

    virtual std::shared_ptr<SessionBase> GetSharedThis() = 0;

  private:
    beast::tcp_stream stream_;
    beast::flat_buffer buffer_;
    StringRequest request_;
};

template<typename RequestHandler>
class Session :
    public SessionBase,
    public std::enable_shared_from_this<Session<RequestHandler>> {
  public:
    template<typename Handler>
    Session(tcp::socket&& socket, Handler&& request_handler) :
        SessionBase(std::move(socket)),
        request_handler_(std::forward<Handler>(request_handler)) {}

  private:
    std::shared_ptr<SessionBase> GetSharedThis() override {
        return this->shared_from_this();
    }

    template<typename Body, typename Allocator>
    void LogRequest(const HttpRequest<Body, Allocator>& request) const {
        BOOST_LOG_TRIVIAL(info)
            << logging::add_value(
                   json_logger::additional_data,
                   json::value {
                       {"ip", GetRemoteEndpoint().address().to_string()},
                       {"URI", request.target()},
                       {"method", request.method_string()},
                   }
               )
            << "request received";
    }

    template<typename Body, typename Allocator>
    void LogResponse(const HttpResponse<Body, Allocator>& response) const {
        auto response_time =
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now() - receive_time_
            );

        json::object response_log {
            {"response_time", response_time.count()},
            {"code", response.result_int()},
        };

        if (const auto it = response.find(http::field::content_type);
            it != response.end()) {
            response_log["content_type"] = std::string(it->value());
        } else {
            response_log["content_type"] = nullptr;
        }

        BOOST_LOG_TRIVIAL(info)
            << logging::add_value(json_logger::additional_data, response_log)
            << "response sent";
    }

    void HandleRequest(StringRequest&& request) override {
        LogRequest(request);

        receive_time_ = std::chrono::system_clock::now();
        request_handler_(
            std::move(request),
            [self = this->shared_from_this()](auto&& response) {
                self->LogResponse(response);
                self->Write(std::move(response));
            }
        );
    }

    RequestHandler request_handler_;
    std::chrono::system_clock::time_point receive_time_;
};

template<typename RequestHandler>
class Listener : public std::enable_shared_from_this<Listener<RequestHandler>> {
  public:
    template<typename Handler>
    Listener(
        net::io_context& io,
        const tcp::endpoint& endpoint,
        Handler&& request_handler
    ) :
        io_(io),
        acceptor_(net::make_strand(io)),
        request_handler_(std::forward<Handler>(request_handler)) {
        acceptor_.open(endpoint.protocol());
        acceptor_.set_option(net::socket_base::reuse_address(true));
        acceptor_.bind(endpoint);
        acceptor_.listen(net::socket_base::max_listen_connections);
    }

    void Run() {
        DoAccept();
    }

  private:
    void DoAccept() {
        acceptor_.async_accept(
            net::make_strand(io_),
            beast::bind_front_handler(
                &Listener::OnAccept,
                this->shared_from_this()
            )
        );
    }

    void OnAccept(sys::error_code ec, tcp::socket socket) {
        if (ec) {
            return ReportError(ec, "accept");
        }

        AsyncRunSession(std::move(socket));
        DoAccept();
    }

    void AsyncRunSession(tcp::socket&& socket) {
        std::make_shared<Session<RequestHandler>>(
            std::move(socket),
            request_handler_
        )
            ->Run();
    }

    net::io_context& io_;
    tcp::acceptor acceptor_;
    RequestHandler request_handler_;
};

template<typename RequestHandler>
void ServeHttp(
    net::io_context& io,
    const tcp::endpoint& endpoint,
    RequestHandler&& handler
) {
    using MyListener = Listener<std::decay_t<RequestHandler>>;
    std::make_shared<MyListener>(
        io,
        endpoint,
        std::forward<RequestHandler>(handler)
    )
        ->Run();
}

}  // namespace http_server
