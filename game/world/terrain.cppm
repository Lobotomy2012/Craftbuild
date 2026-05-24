module;

#include <includes.hpp>

#include <algorithm>
#include <random>
#include <stdexcept>

export module game.world.terrain;

import misc.ptr;
import misc.types;

export namespace craftbuild {
    struct WorldGenerationContext {
        int32 min_y = 0;
        int32 height = 255;

        int32 top_y() const {
            return min_y + height - 1;
        }
    };

    class RandomSource {
    private:
        std::mt19937 generator;

    public:
        explicit RandomSource(uint32 seed) : generator(seed) {}

        int32 next_int(int32 bound) {
            if (bound <= 0) throw std::invalid_argument("RandomSource::next_int bound must be positive");
            std::uniform_int_distribution<int32> distribution(0, bound - 1);
            return distribution(generator);
        }

        int32 next_int(int32 min_inclusive, int32 max_inclusive) {
            if (min_inclusive > max_inclusive) return min_inclusive;
            std::uniform_int_distribution<int32> distribution(min_inclusive, max_inclusive);
            return distribution(generator);
        }
    };

    enum class VerticalAnchorType {
        ABSOLUTE,
        ABOVE_BOTTOM,
        BELOW_TOP,
    };

    struct VerticalAnchor {
        VerticalAnchorType type = VerticalAnchorType::ABSOLUTE;
        int32 value = 0;

        static VerticalAnchor absolute(int32 y) {
            return { VerticalAnchorType::ABSOLUTE, y };
        }

        static VerticalAnchor above_bottom(int32 offset) {
            return { VerticalAnchorType::ABOVE_BOTTOM, offset };
        }

        static VerticalAnchor below_top(int32 offset) {
            return { VerticalAnchorType::BELOW_TOP, offset };
        }

        int32 resolve_y(const WorldGenerationContext& context) const {
            switch (type) {
            case VerticalAnchorType::ABSOLUTE:     return value;
            case VerticalAnchorType::ABOVE_BOTTOM: return context.min_y + value;
            case VerticalAnchorType::BELOW_TOP:    return context.top_y() - value;
            }

            return value;
        }
    };

    enum class HeightProviderType {
        CONSTANT,
        UNIFORM,
        BIASED_TO_BOTTOM,
        VERY_BIASED_TO_BOTTOM,
        TRAPEZOID,
        WEIGHTED_LIST,
    };

    class HeightProvider {
    public:
        virtual ~HeightProvider() = default;

        virtual int32 sample(RandomSource& random, const WorldGenerationContext& context) const = 0;
        virtual HeightProviderType get_type() const = 0;
    };

    using HeightProviderPtr = ptr<const HeightProvider>;

    class ConstantHeight final : public HeightProvider {
    private:
        VerticalAnchor value;

    public:
        explicit ConstantHeight(VerticalAnchor value) : value(value) {}

        static HeightProviderPtr of(VerticalAnchor value) {
            return new ConstantHeight(value);
        }

        const VerticalAnchor& get_value() const {
            return value;
        }

        int32 sample(RandomSource&, const WorldGenerationContext& context) const override {
            return value.resolve_y(context);
        }

        HeightProviderType get_type() const override {
            return HeightProviderType::CONSTANT;
        }
    };

    class UniformHeight final : public HeightProvider {
    private:
        VerticalAnchor min_inclusive;
        VerticalAnchor max_inclusive;

    public:
        UniformHeight(VerticalAnchor min_inclusive, VerticalAnchor max_inclusive) : min_inclusive(min_inclusive), max_inclusive(max_inclusive) {}

        static HeightProviderPtr of(VerticalAnchor min_inclusive, VerticalAnchor max_inclusive) {
            return new UniformHeight(min_inclusive, max_inclusive);
        }

        int32 sample(RandomSource& random, const WorldGenerationContext& context) const override {
            const int32 min_y = min_inclusive.resolve_y(context);
            const int32 max_y = max_inclusive.resolve_y(context);
            if (min_y > max_y) return min_y;
            return random.next_int(min_y, max_y);
        }

        HeightProviderType get_type() const override {
            return HeightProviderType::UNIFORM;
        }
    };

    class BiasedToBottomHeight final : public HeightProvider {
    private:
        VerticalAnchor min_inclusive;
        VerticalAnchor max_inclusive;
        int32 inner = 1;

    public:
        BiasedToBottomHeight(VerticalAnchor min_inclusive, VerticalAnchor max_inclusive, int32 inner = 1) : min_inclusive(min_inclusive), max_inclusive(max_inclusive), inner(std::max<int32>(1, inner)) {}

        static HeightProviderPtr of(VerticalAnchor min_inclusive, VerticalAnchor max_inclusive, int32 inner = 1) {
            return new BiasedToBottomHeight(min_inclusive, max_inclusive, inner);
        }

        int32 sample(RandomSource& random, const WorldGenerationContext& context) const override {
            const int32 min_y = min_inclusive.resolve_y(context);
            const int32 max_y = max_inclusive.resolve_y(context);
            const int32 bound = max_y - min_y - inner + 1;
            if (bound <= 0) return min_y;

            const int32 offset = random.next_int(bound);
            return random.next_int(offset + inner) + min_y;
        }

        HeightProviderType get_type() const override {
            return HeightProviderType::BIASED_TO_BOTTOM;
        }
    };

    class VeryBiasedToBottomHeight final : public HeightProvider {
    private:
        VerticalAnchor min_inclusive;
        VerticalAnchor max_inclusive;
        int32 inner = 1;

    public:
        VeryBiasedToBottomHeight(VerticalAnchor min_inclusive, VerticalAnchor max_inclusive, int32 inner = 1) : min_inclusive(min_inclusive), max_inclusive(max_inclusive), inner(std::max<int32>(1, inner)) {}

        static HeightProviderPtr of(VerticalAnchor min_inclusive, VerticalAnchor max_inclusive, int32 inner = 1) {
            return new VeryBiasedToBottomHeight(min_inclusive, max_inclusive, inner);
        }

        int32 sample(RandomSource& random, const WorldGenerationContext& context) const override {
            const int32 min_y = min_inclusive.resolve_y(context);
            const int32 max_y = max_inclusive.resolve_y(context);
            if (max_y - min_y - inner + 1 <= 0) return min_y;

            const int32 first = random.next_int(min_y + inner, max_y);
            const int32 second = random.next_int(min_y, first - 1);
            return random.next_int(min_y, second - 1 + inner);
        }

        HeightProviderType get_type() const override {
            return HeightProviderType::VERY_BIASED_TO_BOTTOM;
        }
    };

    class TrapezoidHeight final : public HeightProvider {
    private:
        VerticalAnchor min_inclusive;
        VerticalAnchor max_inclusive;
        int32 plateau = 0;

    public:
        TrapezoidHeight(VerticalAnchor min_inclusive, VerticalAnchor max_inclusive, int32 plateau = 0) : min_inclusive(min_inclusive), max_inclusive(max_inclusive), plateau(plateau) {}

        static HeightProviderPtr of(VerticalAnchor min_inclusive, VerticalAnchor max_inclusive, int32 plateau = 0) {
            return new TrapezoidHeight(min_inclusive, max_inclusive, plateau);
        }

        int32 sample(RandomSource& random, const WorldGenerationContext& context) const override {
            const int32 min_y = min_inclusive.resolve_y(context);
            const int32 max_y = max_inclusive.resolve_y(context);
            if (min_y > max_y) return min_y;

            const int32 range = max_y - min_y;
            if (plateau >= range) return random.next_int(min_y, max_y);

            const int32 first_range = (range - plateau) / 2;
            const int32 second_range = range - first_range;
            return min_y + random.next_int(0, second_range) + random.next_int(0, first_range);
        }

        HeightProviderType get_type() const override {
            return HeightProviderType::TRAPEZOID;
        }
    };

    class WeightedListHeight final : public HeightProvider {
    public:
        struct Entry {
            HeightProviderPtr provider;
            int32 weight = 1;
        };

    private:
        std::vector<Entry> distribution;
        int32 total_weight = 0;

    public:
        explicit WeightedListHeight(std::vector<Entry> distribution) : distribution(std::move(distribution)) {
            for (const Entry& entry : this->distribution) {
                if (entry.provider and entry.weight > 0) total_weight += entry.weight;
            }
        }

        static HeightProviderPtr of(std::vector<Entry> distribution) {
            return new WeightedListHeight(std::move(distribution));
        }

        int32 sample(RandomSource& random, const WorldGenerationContext& context) const override {
            if (total_weight <= 0) return 0;

            int32 chosen = random.next_int(total_weight);
            for (const Entry& entry : distribution) {
                if (not entry.provider or entry.weight <= 0) continue;
                if (chosen < entry.weight) return entry.provider.value().sample(random, context);
                chosen -= entry.weight;
            }

            return 0;
        }

        HeightProviderType get_type() const override {
            return HeightProviderType::WEIGHTED_LIST;
        }
    };
}
