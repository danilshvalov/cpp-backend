#include "json_logger.h"

namespace json_logger {

namespace keywords = boost::log::keywords;
namespace expr = boost::log::expressions;

void JsonFormatter(
    const logging::record_view& rec,
    logging::formatting_ostream& strm
) {
    json::object log;

    if (auto ts = rec[timestamp]; ts) {
        log["timestamp"] = to_iso_extended_string(*ts);
    }

    if (auto message = rec[expr::smessage]; message) {
        log["message"] = *message;
    }

    if (auto data = rec[additional_data]; data) {
        log["data"] = *data;
    }

    strm << log;
}

void InitBoostLogFilter() {
    logging::add_common_attributes();
    logging::add_console_log(
        std::cout,
        keywords::auto_flush = true,
        keywords::format = &JsonFormatter
    );
}
}  // namespace json_logger
