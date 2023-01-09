#pragma once

#include <boost/system/error_code.hpp>

#include <string>
#include <string_view>

namespace web {

namespace sys = boost::system;

std::string_view GetMimeType(std::string_view path);

std::string DecodeUrl(std::string_view url);

void ReportError(sys::error_code ec, std::string_view what);

}  // namespace web
