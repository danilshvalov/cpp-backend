#pragma once

#include "web/core.h"
#include "app/app.h"

#include <boost/system/error_code.hpp>

#include <string>
#include <string_view>
#include <optional>

namespace web {

namespace sys = boost::system;

std::string_view GetMimeType(std::string_view path);

std::string DecodeUrl(std::string_view url);

void ReportError(sys::error_code ec, std::string_view what);

template<typename Body, typename Allocator>
std::optional<app::Token> TryExtractToken(
    const web::HttpRequest<Body, Allocator>& req
) {
    auto it = req.find(http::field::authorization);
    if (it == req.end()) {
        return std::nullopt;
    }

    std::string_view header = it->value();
    constexpr std::string_view bearer_prefix = "Bearer ";
    if (!header.starts_with(bearer_prefix)) {
        return std::nullopt;
    }
    header.remove_prefix(bearer_prefix.size());

    if (header.size() != 32) {
        return std::nullopt;
    }

    return app::Token(std::string(header));
}

}  // namespace web
