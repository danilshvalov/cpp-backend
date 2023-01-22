#pragma once

#include <boost/asio/strand.hpp>
#include <boost/asio/io_context.hpp>

namespace app {

namespace net = boost::asio;

using Strand = net::strand<net::io_context::executor_type>;
}  // namespace app
