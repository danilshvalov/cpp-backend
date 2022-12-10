#define BOOST_BEAST_USE_STD_STRING_VIEW

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/write.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <iostream>
#include <optional>
#include <thread>

namespace net = boost::asio;
namespace beast = boost::beast;
namespace http = beast::http;
namespace sys = boost::system;

using StringRequest = http::request<http::string_body>;
using StringResponse = http::response<http::string_body>;
using tcp = net::ip::tcp;

using namespace std::literals;

struct ContentType {
    ContentType() = delete;
    constexpr static std::string_view TEXT_HTML = "text/html";
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

std::optional<StringRequest> ReadRequest(tcp::socket& socket,
                                         beast::flat_buffer& buffer) {
    sys::error_code ec;
    StringRequest request;

    http::read(socket, buffer, request, ec);

    if (ec == http::error::end_of_stream) {
        return std::nullopt;
    }

    if (ec) {
        throw std::runtime_error(
            "Failed to read request: "s.append(ec.message()));
    }

    return request;
}

template <typename RequestHandler>
void HandleConnection(tcp::socket& socket, RequestHandler&& handle_request) {
    try {
        beast::flat_buffer buffer;

        while (auto request = ReadRequest(socket, buffer)) {
            StringResponse response = handle_request(*std::move(request));
            http::write(socket, response);

            if (response.need_eof()) {
                break;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    beast::error_code ec;
    socket.shutdown(tcp::socket::shutdown_send, ec);
}

int main() {
    constexpr unsigned short port = 8080;
    net::io_context io;

    tcp::acceptor acceptor(io, {tcp::v4(), port});

    std::cout << "Server has started..." << std::endl;

    while (true) {
        tcp::socket socket(io);
        acceptor.accept(socket);

        std::thread t(
            [](tcp::socket socket) { HandleConnection(socket, HandleRequest); },
            std::move(socket));

        t.detach();
    }
}
