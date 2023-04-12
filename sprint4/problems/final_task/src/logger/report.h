#pragma once

#include <boost/system/error_code.hpp>

#include <string_view>

namespace logger {

namespace sys = boost::system;
void ReportError(sys::error_code ec, std::string_view what);

}  // namespace logger
