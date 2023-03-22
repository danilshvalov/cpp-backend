#pragma once

#include "serde/archive/lost_object.h"
#include "model/lost_objects_bag.h"

#include <boost/serialization/vector.hpp>

namespace serde::archive {

class LostObjectsBagRepr {
  public:
    explicit LostObjectsBagRepr() = default;

    explicit LostObjectsBagRepr(const model::LostObjectsBag& lost_objects_bag) :
        capacity_(lost_objects_bag.Capacity()) {
        lost_objects_.reserve(lost_objects_bag.Size());
        for (const auto& lost_object : lost_objects_bag) {
            lost_objects_.push_back(LostObjectRepr(lost_object));
        }
    }

    [[nodiscard]] model::LostObjectsBag Restore() const {
        model::LostObjectsBag lost_objects_bag(capacity_);
        for (const auto& lost_object : lost_objects_) {
            lost_objects_bag.Add(lost_object.Restore());
        }
        return lost_objects_bag;
    }

    template <typename Archive>
    void serialize(Archive& ar, [[maybe_unused]] const unsigned version) {
        ar& lost_objects_;
        ar& capacity_;
    }

  private:
    std::vector<LostObjectRepr> lost_objects_;
    size_t capacity_;
};

} // namespace serde::archive
