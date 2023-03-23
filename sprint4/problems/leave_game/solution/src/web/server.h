#pragma once

#include "web/listener.h"

namespace web {

namespace net = boost::asio;
using tcp = net::ip::tcp;

template <typename RequestHandler>
void ServeHttp(
    net::io_context& io, const tcp::endpoint& endpoint, RequestHandler&& handler
) {
    using MyListener = Listener<std::decay_t<RequestHandler>>;
    std::make_shared<MyListener>(
        io, endpoint, std::forward<RequestHandler>(handler)
    )
        ->Run();
}

} // namespace web
