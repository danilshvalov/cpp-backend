#pragma once

#include <string>
#include <filesystem>
#include <optional>

namespace model {

namespace fs = std::filesystem;

struct LootType {
    std::string name;
    fs::path file;
    std::string type;
    std::optional<int64_t> rotation;
    std::optional<std::string> color;
    double scale;
    size_t value;
};
}  // namespace model
