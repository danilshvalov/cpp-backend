#pragma once
#include "sdk.h"
#define BOOST_BEAST_USE_STD_STRING_VIEW

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <iostream>

namespace http_server {

namespace net = boost::asio;
using tcp = net::ip::tcp;
namespace beast = boost::beast;
namespace http = beast::http;
namespace sys = boost::system;

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
    using HttpRequest = http::request<http::string_body>;

    explicit SessionBase(tcp::socket&& socket) : stream_(std::move(socket)) {}

    ~SessionBase() = default;

    void Read();

    void OnRead(sys::error_code ec, size_t bytes_read);

    void OnWrite(bool close, sys::error_code ec, size_t bytes_written);

    void Close();

    virtual void HandleRequest(HttpRequest&& request) = 0;

    virtual std::shared_ptr<SessionBase> GetSharedThis() = 0;

  private:
    beast::tcp_stream stream_;
    beast::flat_buffer buffer_;
    HttpRequest request_;
};

template<typename RequestHandler>
class Session:
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

    void HandleRequest(HttpRequest&& request) override {
        request_handler_(
            std::move(request),
            [self = this->shared_from_this()](auto&& response) {
                self->Write(std::move(response));
            }
        );
    }

    RequestHandler request_handler_;
};

template<typename RequestHandler>
class Listener: public std::enable_shared_from_this<Listener<RequestHandler>> {
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
