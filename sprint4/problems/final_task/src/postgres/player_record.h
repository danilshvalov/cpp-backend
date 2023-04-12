#pragma once

#include "app/player_record.h"

#include <pqxx/transaction>

namespace postgres {

class PlayerRecordRepositoryImpl : public app::PlayerRecordRepository {
  public:
    explicit PlayerRecordRepositoryImpl(pqxx::work& work) : work_(work) {}

    void Save(const app::PlayerRecord& record) override;

    void SaveAll(const std::vector<app::PlayerRecord>& records) override;

    std::vector<app::PlayerRecord> GetAll(size_t offset, size_t limit) override;

  private:
    pqxx::work& work_;
};

} // namespace postgres
