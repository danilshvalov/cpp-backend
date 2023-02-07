#include "collision_detector.h"

#include <cassert>

namespace model::physics {

CollectionResult TryCollectPoint(Point a, Point b, Point c) {
    // Проверим, что перемещение ненулевое.
    // Тут приходится использовать строгое равенство, а не приближённое,
    // пскольку при сборе заказов придётся учитывать перемещение даже на небольшое
    // расстояние.
    assert(b.x != a.x || b.y != a.y);
    const double u_x = c.x - a.x;
    const double u_y = c.y - a.y;
    const double v_x = b.x - a.x;
    const double v_y = b.y - a.y;
    const double u_dot_v = u_x * v_x + u_y * v_y;
    const double u_len2 = u_x * u_x + u_y * u_y;
    const double v_len2 = v_x * v_x + v_y * v_y;
    const double proj_ratio = u_dot_v / v_len2;
    const double sq_distance = u_len2 - (u_dot_v * u_dot_v) / v_len2;

    return CollectionResult(sq_distance, proj_ratio);
}

std::vector<GatheringEvent>
FindGatherEvents(const ItemGathererProvider& provider) {
    const auto is_points_equal = [](Point p1, Point p2) {
        return p1.x == p2.x && p1.y == p2.y;
    };

    std::vector<GatheringEvent> result;

    for (size_t gatherer_id = 0; gatherer_id < provider.GatherersCount();
         ++gatherer_id) {
        const Gatherer gatherer = provider.GetGatherer(gatherer_id);
        if (is_points_equal(gatherer.start_pos, gatherer.end_pos)) {
            continue;
        }

        for (size_t item_id = 0; item_id < provider.ItemsCount(); ++item_id) {
            const Item item = provider.GetItem(item_id);

            const CollectionResult collect_result = TryCollectPoint(
                gatherer.start_pos,
                gatherer.end_pos,
                item.position
            );

            if (collect_result.IsCollected(item.width + gatherer.width)) {
                result.push_back(GatheringEvent {
                    .item_id = item_id,
                    .gatherer_id = gatherer_id,
                    .sq_distance = collect_result.sq_distance,
                    .time = collect_result.proj_ratio,
                });
            }
        }
    }

    std::sort(
        result.begin(),
        result.end(),
        [](const auto& lhs, const auto& rhs) { return lhs.time < rhs.time; }
    );
    return result;
}

}  // namespace model::physics
