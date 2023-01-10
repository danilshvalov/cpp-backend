#pragma once

namespace model {

using Dimension = int;
using Coord = Dimension;

struct Point {
    Coord x;
    Coord y;
};

struct Size {
    Dimension width;
    Dimension height;
};

struct Rectangle {
    Point position;
    Size size;
};

struct Offset {
    Dimension dx;
    Dimension dy;
};

}  // namespace model
