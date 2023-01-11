#pragma once
#define BOOST_BEAST_USE_STD_STRING_VIEW

#include "web/session.h"
#include "web/utils.h"

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>

#include <memory>

namespace web {

namespace net = boost::asio;
using tcp = net::ip::tcp;
namespace beast = boost::beast;

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

}  // namespace web
