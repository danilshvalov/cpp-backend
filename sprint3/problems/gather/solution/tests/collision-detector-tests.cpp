#define _USE_MATH_DEFINES
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <cmath>
#include <limits>

#include "../src/collision_detector.h"

using namespace Catch::Matchers;
using namespace collision_detector;

const std::string TAG = "[FindGatherEvents]";

class TestItemGathererProvider : public ItemGathererProvider {
  public:
    using Items = std::vector<Item>;
    using Gatherers = std::vector<Gatherer>;

    TestItemGathererProvider(Items items, Gatherers gatherers) :
        items_(std::move(items)),
        gatherers_(std::move(gatherers)) {}

    size_t ItemsCount() const override {
        return items_.size();
    }

    size_t GatherersCount() const override {
        return gatherers_.size();
    }

    Item GetItem(size_t idx) const override {
        return items_[idx];
    }

    Gatherer GetGatherer(size_t idx) const override {
        return gatherers_[idx];
    }

  private:
    Items items_;
    Gatherers gatherers_;
};

TEST_CASE("Find events without items or gatherers", TAG) {
    Item item {
        .position = geom::Point2D {0.0, 0.0},
        .width = 0.6,
    };

    Gatherer gatherer {
        .start_pos = geom::Point2D {0.0, 0.0},
        .end_pos = geom::Point2D {10.0, 10.0},
        .width = 0.6,
    };

    SECTION("Without items and gatherers") {
        auto events = FindGatherEvents(TestItemGathererProvider({}, {}));
        CHECK(events.empty());
    }

    SECTION("Without items") {
        auto events =
            FindGatherEvents(TestItemGathererProvider({}, {gatherer}));
        CHECK(events.empty());
    }

    SECTION("Without gatherers") {
        auto events = FindGatherEvents(TestItemGathererProvider({item}, {}));
        CHECK(events.empty());
    }
}

TEST_CASE("Find events with non-intersecting items gatherers", TAG) {
    Item item {
        .position = geom::Point2D {0.0, 0.0},
        .width = 0.6,
    };

    Gatherer gatherer {
        .start_pos = geom::Point2D {5.0, 5.0},
        .end_pos = geom::Point2D {10.0, 10.0},
        .width = 0.6,
    };

    auto events =
        FindGatherEvents(TestItemGathererProvider({item}, {gatherer}));

    CHECK(events.empty());
}

TEST_CASE("Item is on the path of gatherer movement", TAG) {
    Item item1 {
        .position = geom::Point2D {5.0, 5.0},
        .width = 0.6,
    };

    Gatherer gatherer {
        .start_pos = geom::Point2D {0.0, 0.0},
        .end_pos = geom::Point2D {10.0, 10.0},
        .width = 0.6,
    };

    auto events =
        FindGatherEvents(TestItemGathererProvider({item1}, {gatherer}));

    REQUIRE(events.size() == 1);
    const auto& event = events.front();

    CHECK(event.item_id == 0);
    CHECK(event.gatherer_id == 0);
    CHECK_THAT(event.sq_distance, WithinAbs(0.0, 1e-5));
    CHECK_THAT(
        event.time,
        WithinRel(item1.position.x / gatherer.end_pos.x, 1e-5)
    );
}

TEST_CASE(
    "Item and gatherer are at a distance slightly less than their width",
    TAG
) {
    const double width = 0.6;
    const double offset = width * 2 - std::numeric_limits<double>::epsilon();

    Item item {
        .position = geom::Point2D {5.0, 5.0},
        .width = width,
    };

    SECTION("X axis") {
        Gatherer gatherer {
            .start_pos = geom::Point2D {item.position.x + offset, 0.0},
            .end_pos =
                geom::Point2D {
                    item.position.x + offset,
                    item.position.y,
                },
            .width = width,
        };

        auto events =
            FindGatherEvents(TestItemGathererProvider({item}, {gatherer}));

        REQUIRE(events.size() == 1);
        const auto& event = events.front();

        CHECK(event.item_id == 0);
        CHECK(event.gatherer_id == 0);
        CHECK_THAT(event.sq_distance, WithinRel(offset * offset, 1e-5));
        CHECK_THAT(
            event.time,
            WithinRel(item.position.y / gatherer.end_pos.y, 1e-5)
        );
    }

    SECTION("Y axis") {
        Gatherer gatherer {
            .start_pos = geom::Point2D {0.0, item.position.y + offset},
            .end_pos =
                geom::Point2D {
                    item.position.x,
                    item.position.y + offset,
                },
            .width = width,
        };

        auto events =
            FindGatherEvents(TestItemGathererProvider({item}, {gatherer}));

        REQUIRE(events.size() == 1);
        const auto& event = events.front();

        CHECK(event.item_id == 0);
        CHECK(event.gatherer_id == 0);
        CHECK_THAT(event.sq_distance, WithinRel(offset * offset, 1e-5));
        CHECK_THAT(
            event.time,
            WithinRel(item.position.x / gatherer.end_pos.x, 1e-5)
        );
    }
}

TEST_CASE("Item and gatherer are at a distance of their width", TAG) {
    const double width = 0.6;
    const double offset = width * 2;

    Item item {
        .position = geom::Point2D {5.0, 5.0},
        .width = width,
    };

    SECTION("X axis") {
        Gatherer gatherer {
            .start_pos = geom::Point2D {item.position.x + offset, 0.0},
            .end_pos =
                geom::Point2D {
                    item.position.x + offset,
                    item.position.y,
                },
            .width = width,
        };

        auto events =
            FindGatherEvents(TestItemGathererProvider({item}, {gatherer}));

        CHECK(events.empty());
    }

    SECTION("Y axis") {
        Gatherer gatherer {
            .start_pos = geom::Point2D {0.0, item.position.y + offset},
            .end_pos =
                geom::Point2D {
                    item.position.x,
                    item.position.y + offset,
                },
            .width = width,
        };

        auto events =
            FindGatherEvents(TestItemGathererProvider({item}, {gatherer}));

        CHECK(events.empty());
    }
}

TEST_CASE("Multiple items and one gatherer", TAG) {
    Item item1 {
        .position = geom::Point2D {5.0, 5.0},
        .width = 0.6,
    };

    Item item2 {
        .position = geom::Point2D {7.0, 7.0},
        .width = 0.6,
    };

    Item item3 {
        .position = geom::Point2D {-5.0, -5.0},
        .width = 0.6,
    };

    Gatherer gatherer {
        .start_pos = geom::Point2D {0.0, 0.0},
        .end_pos = geom::Point2D {10.0, 10.0},
        .width = 0.6,
    };

    SECTION("Two items on the way") {
        auto events = FindGatherEvents(
            TestItemGathererProvider({item1, item2}, {gatherer})
        );

        REQUIRE(events.size() == 2);
        const auto& first = events.front();
        const auto& second = events.back();

        CHECK(first.item_id == 0);
        CHECK(first.gatherer_id == 0);
        CHECK_THAT(first.sq_distance, WithinAbs(0.0, 1e-5));
        CHECK_THAT(
            first.time,
            WithinRel(item1.position.x / gatherer.end_pos.x, 1e-5)
        );

        CHECK(second.item_id == 1);
        CHECK(second.gatherer_id == 0);
        CHECK_THAT(second.sq_distance, WithinAbs(0.0, 1e-5));
        CHECK_THAT(
            second.time,
            WithinRel(item2.position.x / gatherer.end_pos.x, 1e-5)
        );
    }
    SECTION("One item is on the way, and one is not") {
        auto events = FindGatherEvents(
            TestItemGathererProvider({item1, item3}, {gatherer})
        );

        REQUIRE(events.size() == 1);
        const auto& event = events.front();

        CHECK(event.item_id == 0);
        CHECK(event.gatherer_id == 0);
        CHECK_THAT(event.sq_distance, WithinAbs(0.0, 1e-5));
        CHECK_THAT(
            event.time,
            WithinRel(item1.position.x / gatherer.end_pos.x, 1e-5)
        );
    }
}

TEST_CASE("Multiple gatherers and one item", TAG) {
    Item item {
        .position = geom::Point2D {5.0, 5.0},
        .width = 0.6,
    };

    Gatherer gatherer1 {
        .start_pos = geom::Point2D {2.0, 2.0},
        .end_pos = geom::Point2D {10.0, 10.0},
        .width = 0.6,
    };

    Gatherer gatherer2 {
        .start_pos = geom::Point2D {0.0, 0.0},
        .end_pos = geom::Point2D {10.0, 10.0},
        .width = 0.6,
    };

    Gatherer gatherer3 {
        .start_pos = geom::Point2D {2.0, 2.0},
        .end_pos = geom::Point2D {0.0, 0.0},
        .width = 0.6,
    };

    SECTION("Two gatherers on the way") {
        auto events = FindGatherEvents(
            TestItemGathererProvider({item}, {gatherer1, gatherer2})
        );

        REQUIRE(events.size() == 2);
        const auto& first = events.front();
        const auto& second = events.back();

        CHECK(first.item_id == 0);
        CHECK(first.gatherer_id == 0);
        CHECK_THAT(first.sq_distance, WithinAbs(0.0, 1e-5));
        CHECK_THAT(
            first.time,
            WithinRel(
                (item.position.x - gatherer1.start_pos.x) /
                    (gatherer1.end_pos.x - gatherer1.start_pos.x),
                1e-5
            )
        );

        CHECK(second.item_id == 0);
        CHECK(second.gatherer_id == 1);
        CHECK_THAT(second.sq_distance, WithinAbs(0.0, 1e-5));
        CHECK_THAT(
            second.time,
            WithinRel(item.position.x / gatherer2.end_pos.x, 1e-5)
        );
    }
    SECTION("One gatherer is on the way, and one is not") {
        auto events = FindGatherEvents(
            TestItemGathererProvider({item}, {gatherer1, gatherer3})
        );

        REQUIRE(events.size() == 1);
        const auto& event = events.front();

        CHECK(event.item_id == 0);
        CHECK(event.gatherer_id == 0);
        CHECK_THAT(event.sq_distance, WithinAbs(0.0, 1e-5));
        CHECK_THAT(
            event.time,
            WithinRel(
                (item.position.x - gatherer1.start_pos.x) /
                    (gatherer1.end_pos.x - gatherer1.start_pos.x),
                1e-5
            )
        );
    }
}

TEST_CASE("Gatherers with different collection times", TAG) {
    Item item {
        .position = geom::Point2D {5.0, 5.0},
        .width = 0.6,
    };

    Gatherer gatherer1 {
        .start_pos = geom::Point2D {0.0, 0.0},
        .end_pos = geom::Point2D {10.0, 10.0},
        .width = 0.6,
    };

    Gatherer gatherer2 {
        .start_pos = geom::Point2D {4.0, 4.0},
        .end_pos = geom::Point2D {10.0, 10.0},
        .width = 0.6,
    };

    Gatherer gatherer3 {
        .start_pos = geom::Point2D {2.0, 2.0},
        .end_pos = geom::Point2D {10.0, 10.0},
        .width = 0.6,
    };

    auto events = FindGatherEvents(
        TestItemGathererProvider({item}, {gatherer1, gatherer2, gatherer3})
    );

    REQUIRE(events.size() == 3);
    CHECK(events[0].gatherer_id == 1);
    CHECK(events[1].gatherer_id == 2);
    CHECK(events[2].gatherer_id == 0);
}
