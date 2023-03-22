#include "postgres/player_record.h"
#include "postgres/consts.h"

#include <fmt/core.h>
#include <pqxx/result>

namespace postgres {

void PlayerRecordRepositoryImpl::Save(const app::PlayerRecord& record) {
    work_.exec_params(
        R"(
            INSERT INTO hall_of_fame (name, score, play_time_ms)
            VALUES ($1, $2, $3);
        )",
        record.GetName(), record.GetScore(), record.GetPlayTime().count()
    );
}

void PlayerRecordRepositoryImpl::SaveAll(
    const std::vector<app::PlayerRecord>& records
) {
    for (const auto& r : records) {
        Save(r);
    }
}

std::vector<app::PlayerRecord>
PlayerRecordRepositoryImpl::GetAll(size_t offset, size_t limit) {
    if (limit > player_records_limit) {
        throw std::runtime_error(
            "The number of values cannot be more than " +
            std::to_string(player_records_limit)
        );
    }
    const auto query_text = fmt::format(
        R"(
            SELECT name, score, play_time_ms
            FROM hall_of_fame
            ORDER BY score DESC, play_time_ms, name
            LIMIT {} OFFSET {};
        )",
        limit, offset
    );

    std::vector<app::PlayerRecord> records;
    for (auto [name, score, play_time] :
         work_.query<std::string, size_t, int64_t>(query_text)) {
        records.push_back(app::PlayerRecord{
            name,
            score,
            std::chrono::milliseconds(play_time),
        });
    }

    return records;
}

} // namespace postgres
