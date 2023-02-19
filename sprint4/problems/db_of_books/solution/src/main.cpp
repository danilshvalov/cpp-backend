#include "controllers.h"

#include <iostream>

#include <pqxx/connection>
#include <boost/json/parse.hpp>
#include <boost/json/value.hpp>
#include <boost/json/serialize.hpp>

namespace json = boost::json;

int main(int argc, const char* argv[]) {
    try {
        if (argc == 1) {
            std::cout << "Usage: connect_db <conn-string>\n";
            return EXIT_SUCCESS;
        } else if (argc != 2) {
            std::cerr << "Invalid command line\n";
            return EXIT_FAILURE;
        }

        pqxx::connection connection {argv[1]};
        controllers::JsonDatabaseController controller(connection);

        while (true) {
            std::string line;
            std::getline(std::cin, line);

            json::value input = json::parse(line);

            auto output = controller.Process(input);
            if (!output) {
                break;
            }

            std::cout << json::serialize(*output);
        }

    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}
