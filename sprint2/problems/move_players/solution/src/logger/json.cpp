#include "logger/json.h"

namespace logger::json {

namespace keywords = boost::log::keywords;
namespace expr = boost::log::expressions;

void JsonFormatter(
    const logging::record_view& rec,
    logging::formatting_ostream& strm
) {
    auto ts = rec[timestamp];

    json::object log({
        {"timestamp", to_iso_extended_string(*ts)},
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
    logging::add_common_attributes();
    logging::add_console_log(
        std::cout,
        keywords::auto_flush = true,
        keywords::format = &JsonFormatter
    );
}
}  // namespace logger::json
