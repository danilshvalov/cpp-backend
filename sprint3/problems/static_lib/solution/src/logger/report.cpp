#include "logger/report.h"
#include "logger/json.h"

namespace logger {

namespace logging = boost::log;

void ReportError(sys::error_code ec, std::string_view where) {
    BOOST_LOG_TRIVIAL(error) << logging::add_value(
                                    logger::json::additional_data,
                                    boost::json::value {
                                        {"code", ec.value()},
                                        {"text", ec.message()},
                                        {"where", where},
                                    }
                                )
                             << "error";
}

}  // namespace logger
