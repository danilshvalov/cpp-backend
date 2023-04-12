#pragma once

#include <boost/beast/http.hpp>

namespace web {

namespace beast = boost::beast;
namespace http = beast::http;

template <typename Body, typename Allocator>
using HttpRequest = http::request<Body, http::basic_fields<Allocator>>;

template <typename Body, typename Allocator>
using HttpResponse = http::response<Body, http::basic_fields<Allocator>>;

using StringRequest = http::request<http::string_body>;

using StringResponse = http::response<http::string_body>;

using FileResponse = http::response<http::file_body>;

} // namespace web
