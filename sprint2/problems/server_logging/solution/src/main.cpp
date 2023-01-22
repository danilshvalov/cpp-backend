#include "sdk.h"
//
#include <boost/asio/io_context.hpp>
#include <boost/asio/signal_set.hpp>
#include <iostream>
#include <thread>

#include "json_loader.h"
#include "request_handler.h"
#include "json_logger.h"

using namespace std::literals;
namespace net = boost::asio;
namespace sys = boost::system;
namespace logging = boost::log;
namespace json = boost::json;

namespace {

// Запускает функцию fn на n потоках, включая текущий
template<typename Fn>
void RunWorkers(unsigned n, const Fn& fn) {
    n = std::max(1u, n);
    std::vector<std::jthread> workers;
    workers.reserve(n - 1);
    // Запускаем n-1 рабочих потоков, выполняющих функцию fn
    while (--n) {
        workers.emplace_back(fn);
    }
    fn();
}

}  // namespace

int main(int argc, const char* argv[]) {
    json_logger::InitBoostLogFilter();

    if (argc != 3) {
        std::cerr << "Usage: game_server <game-config-json> <static-files>"
                  << std::endl;
        return EXIT_FAILURE;
    }
    try {
        model::Game game = json_loader::LoadGame(argv[1]);

        const unsigned num_threads = std::thread::hardware_concurrency();
        net::io_context ioc(num_threads);

        net::signal_set signals(ioc, SIGINT, SIGTERM);
        signals.async_wait([&ioc](
                               const sys::error_code& ec,
                               [[maybe_unused]] int signal_number
                           ) {
            if (!ec) {
                ioc.stop();
            }
        });

        http_handler::RequestHandler handler(game, argv[2]);

        const auto address = net::ip::make_address("0.0.0.0");
        constexpr net::ip::port_type port = 8080;
        http_server::ServeHttp(
            ioc,
            {address, port},
            [&handler](auto&& req, auto&& send) {
                handler(
                    std::forward<decltype(req)>(req),
                    std::forward<decltype(send)>(send)
                );
            }
        );

        BOOST_LOG_TRIVIAL(info) << logging::add_value(
                                       json_logger::additional_data,
                                       json::value {
                                           {"port", port},
                                           {"address", address.to_string()},
                                       }
                                   )
                                << "Server has started...";

        RunWorkers(std::max(1u, num_threads), [&ioc] { ioc.run(); });

        BOOST_LOG_TRIVIAL(info) << logging::add_value(
                                       json_logger::additional_data,
                                       json::value {{"code", 0}}
                                   )
                                << "server exited";
    } catch (const std::exception& ex) {
        BOOST_LOG_TRIVIAL(info) << logging::add_value(
                                       json_logger::additional_data,
                                       json::value {
                                           {"code", EXIT_FAILURE},
                                           {"exception", ex.what()},
                                       }
                                   )
                                << "server exited";
        return EXIT_FAILURE;
    }
}
