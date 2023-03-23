#pragma once

#include <memory>

#include "app/player_record.h"

namespace app {

struct UnitOfWork {
    virtual void Commit() = 0;
    virtual PlayerRecordRepository& PlayerRecords() = 0;
    virtual ~UnitOfWork() = default;
};

using UnitOfWorkHolder = std::unique_ptr<UnitOfWork>;

class UnitOfWorkFactory {
  public:
    virtual UnitOfWorkHolder CreateUnitOfWork() = 0;

  protected:
    ~UnitOfWorkFactory() = default;
};

} // namespace app
