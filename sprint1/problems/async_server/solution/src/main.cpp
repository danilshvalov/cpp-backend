#include "sdk.h"
//
#include <boost/asio/signal_set.hpp>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

#include "http_server.h"

namespace {
namespace net = boost::asio;
using namespace std::literals;
namespace sys = boost::system;
namespace http = boost::beast::http;

// Запрос, тело которого представлено в виде строки
using StringRequest = http::request<http::string_body>;
// Ответ, тело которого представлено в виде строки
using StringResponse = http::response<http::string_body>;

struct ContentType {
    ContentType() = delete;
    constexpr static std::string_view TEXT_HTML = "text/html"sv;
    // При необходимости внутрь ContentType можно добавить и другие типы
    // контента
};

StringResponse HandleGetHeadRequest(StringRequest&& request) {
    std::string body = "Hello, "s.append(request.target().substr(1));
    StringResponse response(http::status::ok, request.version());
    response.set(http::field::content_type, ContentType::TEXT_HTML);
    if (request.method() == http::verb::get) {
        response.body() = body;
    }
    response.content_length(body.size());
    response.keep_alive(request.keep_alive());
    return response;
}

StringResponse HandleNotAllowedRequest(StringRequest&& request) {
    StringResponse response(http::status::method_not_allowed,
                            request.version());
    response.set(http::field::content_type, ContentType::TEXT_HTML);
    response.set(http::field::allow, "GET, HEAD");
    response.body() = "Invalid method";
    response.content_length(response.body().size());
    response.keep_alive(request.keep_alive());
    return response;
}

StringResponse HandleRequest(StringRequest&& request) {
    switch (request.method()) {
        case http::verb::get:
        case http::verb::head:
            return HandleGetHeadRequest(std::move(request));
        default:
            return HandleNotAllowedRequest(std::move(request));
    }
}

// Запускает функцию fn на n потоках, включая текущий
template <typename Fn>
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

int main() {
    const unsigned num_threads = std::thread::hardware_concurrency();

    net::io_context io(num_threads);

    // Подписываемся на сигналы и при их получении завершаем работу сервера
    net::signal_set signals(io, SIGINT, SIGTERM);
    signals.async_wait(
        [&io](const sys::error_code& ec, [[maybe_unused]] int signal_number) {
            if (!ec) {
                io.stop();
            }
        });

    const auto address = net::ip::make_address("0.0.0.0");
    constexpr net::ip::port_type port = 8080;
    http_server::ServeHttp(io, {address, port}, [](auto&& req, auto&& sender) {
        sender(HandleRequest(std::forward<decltype(req)>(req)));
    });

    // Эта надпись сообщает тестам о том, что сервер запущен и готов
    // обрабатывать запросы
    std::cout << "Server has started..."sv << std::endl;

    RunWorkers(num_threads, [&io] { io.run(); });
}
