#include "json_logger.h"

#include <boost/date_time.hpp>

namespace json_logger {

namespace keywords = boost::log::keywords;
namespace expr = boost::log::expressions;

void JsonFormatter(
    const logging::record_view& rec,
    logging::formatting_ostream& strm
) {
    json::object log({
        {"timestamp", boost::posix_time::to_iso_extended_string(boost::posix_time::microsec_clock::local_time())},
    });

    if (auto message = rec[expr::smessage]; message) {
        log["message"] = *message;
    }

    if (auto data = rec[additional_data]; data) {
        log["data"] = *data;
    }

    strm << log;
}

void InitBoostLogFilter() {
    logging::add_console_log(
        std::cout,
        keywords::auto_flush = true,
        keywords::format = &JsonFormatter
    );
}
}  // namespace json_logger
