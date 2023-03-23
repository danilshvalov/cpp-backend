#pragma once

#include "serde/archive/lost_objects_bag.h"
#include "serde/archive/point.h"
#include "serde/archive/speed.h"
#include "model/dog.h"

namespace serde::archive {

class DogRepr {
  public:
    explicit DogRepr() = default;

    explicit DogRepr(const model::Dog& dog) :
        position_(dog.GetPosition()),
        prev_position_(dog.GetPrevPosition()),
        speed_(dog.GetSpeed()),
        direction_(static_cast<int>(dog.GetDirection())),
        bag_(dog.GetBag()),
        width_(dog.GetWidth()),
        score_(dog.GetScore()) {}

    [[nodiscard]] model::Dog Restore() const {
        return model::Dog{
            position_.Restore(),
            prev_position_.Restore(),
            speed_.Restore(),
            static_cast<model::Direction>(direction_),
            bag_.Restore(),
            width_,
            score_,
        };
    }

    template <typename Archive>
    void serialize(Archive& ar, [[maybe_unused]] const unsigned version) {
        ar& position_;
        ar& prev_position_;
        ar& speed_;
        ar& direction_;
        ar& bag_;
        ar& width_;
        ar& score_;
    }

  private:
    PointRepr position_;
    PointRepr prev_position_;
    SpeedRepr speed_;
    int direction_;
    LostObjectsBagRepr bag_;
    double width_;
    size_t score_;
};

} // namespace serde::archive
