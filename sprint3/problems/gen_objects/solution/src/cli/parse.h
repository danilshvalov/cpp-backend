#pragma once

#include <string>
#include <optional>

namespace cli {

struct Args {
    size_t tick_period = 0;
    std::string config_file;
    std::string www_root;
    bool randomize_spawn_points = false;
};

[[nodiscard]] std::optional<Args> ParseCommandLine(
    int argc,
    const char* const argv[]
);

}  // namespace cli
