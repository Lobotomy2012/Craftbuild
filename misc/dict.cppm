module;

#include <includes.hpp>
#include <unordered_map>

export module misc.dict;

import misc.hasher;

export namespace craftbuild {
    template<typename T, typename T2>
    requires Hashable<T>
    using Dict = std::unordered_map<T, T2, Hasher<T>>;
}