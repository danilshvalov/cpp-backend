#pragma once

#include <filesystem>

namespace utils {

namespace fs = std::filesystem;

bool IsSubPath(fs::path path, fs::path base);

}  // namespace utils
