#pragma once

#include <string>
#include <optional>

namespace cli {

struct Args {
    size_t tick_period = 0;
    std::string config_file;
    std::string www_root;
    bool randomize_spawn_points = false;
    std::string state_file;
    size_t save_period = 0;
};

[[nodiscard]] std::optional<Args>
ParseCommandLine(int argc, const char* const argv[]);

} // namespace cli
