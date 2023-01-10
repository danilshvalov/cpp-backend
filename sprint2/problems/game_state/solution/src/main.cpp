#include "sdk.h"
//
#include <boost/asio/io_context.hpp>
#include <boost/asio/signal_set.hpp>
#include <iostream>
#include <thread>

#include "serde/json.h"
#include "handlers/request_handler.h"
#include "utils/thread.h"
#include "logger/json.h"
#include "app/app.h"

using namespace std::literals;
namespace net = boost::asio;
namespace sys = boost::system;
namespace logging = boost::log;
namespace json = boost::json;

int main(int argc, const char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: game_server <game-config-json> <static-files>"
                  << std::endl;
        return EXIT_FAILURE;
    }
    try {
        logger::json::InitBoostLogFilter();

        const unsigned num_threads = std::thread::hardware_concurrency();
        net::io_context io(num_threads);

        app::Application app(io, serde::json::LoadGame(argv[1]));

        net::signal_set signals(io, SIGINT, SIGTERM);
        signals.async_wait([&io](
                               const sys::error_code& ec,
                               [[maybe_unused]] int signal_number
                           ) {
            if (!ec) {
                io.stop();
            }
        });

        auto handler = std::make_shared<handlers::RequestHandler>(app, argv[2]);

        const auto address = net::ip::make_address("0.0.0.0");
        constexpr net::ip::port_type port = 8080;
        web::ServeHttp(
            io,
            {address, port},
            [&handler](auto&& req, auto&& send) {
                (*handler
                )(std::forward<decltype(req)>(req),
                  std::forward<decltype(send)>(send));
            }
        );

        BOOST_LOG_TRIVIAL(info) << logging::add_value(
                                       logger::json::additional_data,
                                       json::value {
                                           {"port", port},
                                           {"address", address.to_string()},
                                       }
                                   )
                                << "Server has started...";

        utils::RunWorkers(std::max(1u, num_threads), [&io] { io.run(); });

        BOOST_LOG_TRIVIAL(info) << logging::add_value(
                                       logger::json::additional_data,
                                       json::value {{"code", 0}}
                                   )
                                << "server exited";
    } catch (const std::exception& ex) {
        BOOST_LOG_TRIVIAL(info) << logging::add_value(
                                       logger::json::additional_data,
                                       json::value {
                                           {"code", EXIT_FAILURE},
                                           {"exception", ex.what()},
                                       }
                                   )
                                << "server exited";
        return EXIT_FAILURE;
    }
}
