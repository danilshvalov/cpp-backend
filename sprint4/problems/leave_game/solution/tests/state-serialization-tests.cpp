#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <catch2/catch_test_macros.hpp>
#include <sstream>

#include "model/dog.h"
#include "serde/archive.h"

using namespace model;
using namespace std::literals;

namespace {

using InputArchive = boost::archive::text_iarchive;
using OutputArchive = boost::archive::text_oarchive;

struct Fixture {
    std::stringstream strm;
    OutputArchive output_archive{strm};
};

} // namespace

SCENARIO_METHOD(Fixture, "Point serialization") {
    GIVEN("A point") {
        const Point p{10, 20};
        WHEN("point is serialized") {
            output_archive << serde::archive::PointRepr{p};

            THEN("it is equal to point after serialization") {
                InputArchive input_archive{strm};
                serde::archive::PointRepr restored_point;
                input_archive >> restored_point;
                CHECK(p == restored_point.Restore());
            }
        }
    }
}

SCENARIO_METHOD(Fixture, "Dog Serialization") {
    GIVEN("a dog") {
        const auto dog = [] {
            Dog dog{{42.2, 12.5}, 3};
            dog.SetScore(42);
            CHECK(dog.GetBag().Add(LostObject{LostObject::Id{10}, {0, 0}, 0, 2u}
            ));
            dog.SetDirection(Direction::EAST);
            dog.SetSpeed({2.3, -1.2});
            return dog;
        }();

        WHEN("dog is serialized") {
            {
                serde::archive::DogRepr repr{dog};
                output_archive << repr;
            }

            THEN("it can be deserialized") {
                InputArchive input_archive{strm};
                serde::archive::DogRepr repr;
                input_archive >> repr;
                const auto restored = repr.Restore();

                CHECK(dog.GetPosition() == restored.GetPosition());
                CHECK(dog.GetSpeed() == restored.GetSpeed());

                const auto& dog_bag = dog.GetBag();
                const auto& restored_bag = restored.GetBag();

                CHECK(dog_bag.Capacity() == restored_bag.Capacity());
                CHECK(dog_bag.GetContent() == restored_bag.GetContent());
            }
        }
    }
}
