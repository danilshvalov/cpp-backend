#pragma once

#include "app/use_cases.h"
#include "app/unit_of_work.h"

namespace app {

class UseCasesImpl : public UseCases {
  public:
    explicit UseCasesImpl(UnitOfWorkFactory& unit_factory) :
        unit_factory_(unit_factory) {}

    void SavePlayerRecords(const std::vector<PlayerRecord>& records) override {
        auto work = unit_factory_.CreateUnitOfWork();
        work->PlayerRecords().SaveAll(records);
        work->Commit();
    }

    std::vector<PlayerRecord>
    GetPlayerRecords(size_t offset, size_t limit) override {
        auto work = unit_factory_.CreateUnitOfWork();
        return work->PlayerRecords().GetAll(offset, limit);
    }

  private:
    UnitOfWorkFactory& unit_factory_;
};
} // namespace app
