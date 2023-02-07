#pragma once
#include <chrono>
#include <functional>

namespace model {

/*
 *  Генератор трофеев
 */
class LootGenerator {
  public:
    using RandomGenerator = std::function<double()>;
    using TimeInterval = std::chrono::milliseconds;

    struct Config {
        TimeInterval base_interval;
        double probability;
    };

    /*
     * base_interval - базовый отрезок времени > 0
     * probability - вероятность появления трофея в течение базового интервала времени
     * random_generator - генератор псевдослучайных чисел в диапазоне от [0 до 1]
     */
    LootGenerator(
        Config config,
        RandomGenerator random_gen = DefaultGenerator
    ) :
        config_(std::move(config)),
        random_generator_ {std::move(random_gen)} {}

    /*
     * Возвращает количество трофеев, которые должны появиться на карте спустя
     * заданный промежуток времени.
     * Количество трофеев, появляющихся на карте не превышает количество мародёров.
     *
     * time_delta - отрезок времени, прошедший с момента предыдущего вызова Generate
     * loot_count - количество трофеев на карте до вызова Generate
     * looter_count - количество мародёров на карте
     */
    unsigned Generate(
        TimeInterval time_delta,
        unsigned loot_count,
        unsigned looter_count
    );

    const TimeInterval& GetInterval() const {
        return config_.base_interval;
    }

  private:
    static double DefaultGenerator() noexcept {
        return 1.0;
    };

    Config config_;
    TimeInterval time_without_loot_ {};
    RandomGenerator random_generator_;
};

}  // namespace model
