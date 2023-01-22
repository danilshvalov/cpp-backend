#include "utils/error.h"
#include "logger/json.h"

namespace utils {

namespace json = boost::json;
namespace logging = boost::log;

void ReportError(sys::error_code ec, std::string_view where) {
    BOOST_LOG_TRIVIAL(error) << logging::add_value(
                                    logger::json::additional_data,
                                    json::value {
                                        {"code", ec.value()},
                                        {"text", ec.message()},
                                        {"where", where},
                                    }
                                )
                             << "error";
}

}  // namespace utils
