#pragma once

#include "postgres/connection_pool.h"
#include "postgres/unit_of_work.h"
#include "app/unit_of_work.h"

#include <string>

namespace postgres {

struct DatabaseConfig {
    size_t pool_size = 1;
    std::string url;
};

class Database {
  public:
    explicit Database(const DatabaseConfig& config);

    app::UnitOfWorkFactory& GetUnitOfWorkFactory() {
        return unit_factory_;
    }

  private:
    ConnectionPool connection_pool_;
    UnitOfWorkFactoryImpl unit_factory_{connection_pool_};
};

} // namespace postgres
