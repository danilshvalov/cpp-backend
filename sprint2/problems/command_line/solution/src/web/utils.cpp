#define BOOST_BEAST_USE_STD_STRING_VIEW

#include "web/utils.h"
#include "web/content_type.h"

#include <boost/beast/core/string.hpp>

#include <sstream>
#include <iomanip>

namespace web {

namespace beast = boost::beast;

std::string DecodeUrl(std::string_view url) {
    std::string result;

    for (size_t pos = 0; pos < url.size(); ++pos) {
        if (url[pos] != '%') {
            result += url[pos];
            continue;
        }

        if (pos >= url.size() - 2) {
            break;
        }

        std::stringstream ss;
        ss << std::hex << url.substr(pos + 1, 2);
        uint16_t symbol;
        ss >> symbol;

        result += static_cast<char>(symbol);
        pos += 2;
    }

    return result;
}

std::string_view GetMimeType(std::string_view path) {
    const std::string_view ext = [&path] {
        const size_t pos = path.rfind(".");
        if (pos == path.npos) {
            return std::string_view();
        }
        return path.substr(pos);
    }();

    const auto is_ext = [&ext](const auto&... args) {
        return (beast::iequals(ext, args) || ...);
    };

    if (is_ext(".htm", ".html")) {
        return ContentType::text::html;
    } else if (is_ext(".css")) {
        return ContentType::text::css;
    } else if (is_ext(".txt")) {
        return ContentType::text::plain;
    } else if (is_ext(".js")) {
        return ContentType::text::javascript;
    } else if (is_ext(".json")) {
        return ContentType::application::json;
    } else if (is_ext(".xml")) {
        return ContentType::application::xml;
    } else if (is_ext(".png")) {
        return ContentType::image::png;
    } else if (is_ext(".jpg", ".jpe", ".jpeg")) {
        return ContentType::image::jpeg;
    } else if (is_ext(".gif")) {
        return ContentType::image::gif;
    } else if (is_ext(".bmp")) {
        return ContentType::image::bmp;
    } else if (is_ext(".ico")) {
        return ContentType::image::icon;
    } else if (is_ext(".tiff", ".tif")) {
        return ContentType::image::tiff;
    } else if (is_ext(".svg", ".svgz")) {
        return ContentType::image::svg;
    } else if (is_ext(".mp3")) {
        return ContentType::audio::mpeg;
    } else {
        return ContentType::application::octet_stream;
    }
}

}  // namespace web
