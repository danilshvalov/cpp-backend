#include "postgres/database.h"

namespace postgres {

Database::Database(const DatabaseConfig& config) :
    connection_pool_(config.pool_size, [url = config.url]() {
        return std::make_shared<pqxx::connection>(url);
    }) {
    auto conn = connection_pool_.GetConnection();
    pqxx::work work_{*conn};
    work_.exec(R"(
        CREATE TABLE IF NOT EXISTS hall_of_fame (
            id SERIAL PRIMARY KEY,
            name VARCHAR(100) NOT NULL,
            score INTEGER NOT NULL CONSTRAINT score_non_negative CHECK (score >= 0),
            play_time_ms INTEGER NOT NULL CONSTRAINT play_time_non_negative CHECK (play_time_ms >= 0)
        );
        CREATE INDEX IF NOT EXISTS hall_of_fame_index ON hall_of_fame (score DESC, play_time_ms, name);
    )");
    work_.commit();
}

} // namespace postgres
