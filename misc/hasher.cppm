module;

#include <includes.hpp>
#include <xhash>

export module misc.hasher;

import misc.number;

export namespace craftbuild {
    template <typename T>
    struct Hasher;

    template <>
    struct Hasher<uint8> {
        size operator()(uint8 value) const {
            return std::hash<uint8>{}(value);
        }
    };

    template <typename T>
    concept Hashable = requires(T t) {
        Hasher<T>{}(t);
    };
}