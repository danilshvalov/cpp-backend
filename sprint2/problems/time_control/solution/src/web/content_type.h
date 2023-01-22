#pragma once

#include <string_view>

namespace web {

struct ContentType {
    ContentType() = delete;

    struct text {
        constexpr static std::string_view plain = "text/plain";
        constexpr static std::string_view html = "text/html";
        constexpr static std::string_view css = "text/css";
        constexpr static std::string_view javascript = "text/javascript";
    };

    struct application {
        constexpr static std::string_view json = "application/json";
        constexpr static std::string_view xml = "application/xml";
        constexpr static std::string_view octet_stream =
            "application/octet-stream";
    };

    struct image {
        constexpr static std::string_view png = "image/png";
        constexpr static std::string_view jpeg = "image/jpeg";
        constexpr static std::string_view gif = "image/gif";
        constexpr static std::string_view bmp = "image/bmp";
        constexpr static std::string_view icon = "image/vnd.microsoft.icon";
        constexpr static std::string_view tiff = "image/tiff";
        constexpr static std::string_view svg = "image/svg+xml";
    };

    struct audio {
        constexpr static std::string_view mpeg = "audio/mpeg";
    };
};

}  // namespace web
