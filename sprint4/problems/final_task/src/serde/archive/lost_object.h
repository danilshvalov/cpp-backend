#pragma once

#include "serde/archive/point.h"
#include "model/lost_object.h"

namespace serde::archive {

class LostObjectRepr {
  public:
    explicit LostObjectRepr() = default;

    explicit LostObjectRepr(const model::LostObject& lost_object) :
        id_(*lost_object.GetId()),
        position_(lost_object.GetPosition()),
        type_(lost_object.GetType()),
        value_(lost_object.GetValue()),
        width_(lost_object.GetWidth()),
        picked_up_(lost_object.IsPickedUp()) {}

    [[nodiscard]] model::LostObject Restore() const {
        model::LostObject lost_object{
            model::LostObject::Id(id_),
            position_.Restore(),
            type_,
            value_,
            width_,
        };
        if (picked_up_) {
            lost_object.SetPickedUp();
        }
        return lost_object;
    }

    template <typename Archive>
    void serialize(Archive& ar, [[maybe_unused]] const unsigned version) {
        ar& id_;
        ar& position_;
        ar& type_;
        ar& value_;
        ar& width_;
        ar& picked_up_;
    }

  private:
    size_t id_;
    PointRepr position_;
    size_t type_;
    size_t value_;
    double width_;
    bool picked_up_;
};

} // namespace serde::archive
