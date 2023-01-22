#include "parse.h"

#include <boost/program_options.hpp>
#include <iostream>

namespace cli {
std::optional<Args> ParseCommandLine(int argc, const char* const argv[]) {
    namespace po = boost::program_options;

    po::options_description desc {"All options"};
    Args args;
    desc.add_options()("help,h", "produce help message")(
        "tick-period,t",
        po::value(&args.tick_period)->value_name("milliseconds"),
        "set tick period"
    )("config-file,c",
      po::value(&args.config_file)->value_name("file"),
      "set config file path"
    )("www-root,w",
      po::value(&args.www_root)->value_name("dir"),
      "set static files root"
    )("randomize-spawn-points",
      po::value(&args.randomize_spawn_points),
      "spawn dogs at random positions");
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.contains("help")) {
        std::cout << desc;
        std::exit(0);
    }

    if (!vm.contains("config-file")) {
        throw std::runtime_error("Configuration file is not specified");
    }

    if (!vm.contains("www-root")) {
        throw std::runtime_error("Path to the static content is not specified");
    }

    return args;
}
}  // namespace cli
