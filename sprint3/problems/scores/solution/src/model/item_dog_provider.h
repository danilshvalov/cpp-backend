#pragma once

#include "model/physics/collision.h"

namespace model {

class ItemDogProvider : public physics::ItemGathererProvider {
  public:
    using Items = std::vector<physics::Item>;
    using Gatherers = std::vector<physics::Gatherer>;

    ItemDogProvider(Items items, Gatherers gatherers) :
        items_(std::move(items)),
        gatherers_(std::move(gatherers)) {}

    size_t ItemsCount() const override {
        return items_.size();
    }

    size_t GatherersCount() const override {
        return gatherers_.size();
    }

    physics::Item GetItem(size_t idx) const override {
        return items_[idx];
    }

    physics::Gatherer GetGatherer(size_t idx) const override {
        return gatherers_[idx];
    }

  private:
    Items items_;
    Gatherers gatherers_;
};

}  // namespace model
