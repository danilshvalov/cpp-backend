#include "model/map.h"

#include <stdexcept>

namespace model {

void Map::AddOffice(Office office) {
    if (warehouse_id_to_index_.contains(office.GetId())) {
        throw std::invalid_argument("Duplicate warehouse");
    }

    const size_t index = offices_.size();
    Office& new_office = offices_.emplace_back(std::move(office));
    try {
        warehouse_id_to_index_.emplace(new_office.GetId(), index);
    } catch (...) {
        // Удаляем офис из вектора, если не удалось вставить в unordered_map
        offices_.pop_back();
        throw;
    }
}

}  // namespace model
