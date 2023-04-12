#pragma once

#include <string>
#include <vector>
#include <optional>

#include "app/player_record.h"

namespace app {

class UseCases {
  public:
    virtual void SavePlayerRecords(const std::vector<PlayerRecord>& records
    ) = 0;
    virtual std::vector<PlayerRecord>
    GetPlayerRecords(size_t offset, size_t limit) = 0;

  protected:
    ~UseCases() = default;
};

} // namespace app
