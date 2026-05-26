module;

#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/variant/vector3i.hpp>

#include <includes.hpp>

export module game.pos;

import misc.types;

using namespace godot;

template<typename T1, typename T2>
concept AbleToCast = requires (T2 t2) {
    (T1)t2;
};

export namespace craftbuild {
    template <typename T>
    requires std::is_arithmetic_v<T>
    struct Pos {
        T x, y, z;

        Pos() = default;
        Pos(T x, T y, T z) : x(x), y(y), z(z) {}
        Pos(const Vector3& v) : x(v.x), y(v.y), z(v.z) {}
        Pos(const Vector3i& v) : x(v.x), y(v.y), z(v.z) {}
        template<typename T2>
        requires AbleToCast<T, T2>
        Pos(const Pos<T2>& pos) : x((T)pos.x), y((T)pos.y), z((T)pos.z) {}

#define def_operator(op) Pos& operator##op##=(const Pos& other) { x op##= other.x, y op##= other.y, z op##= other.z; return *this;}

        def_operator(+);
        def_operator(-);
        def_operator(*);
        def_operator(/);
        def_operator(%);

#define def_operator(op) Pos operator##op(const Pos& other) const { Pos result; result.x op##= other.x, result.y op##= other.y, result.z op##= other.z; return result;}

        def_operator(+);
        def_operator(-);
        def_operator(*);
        def_operator(/);
        def_operator(%);

#undef def_operator

        operator Vector3() {
            return Vector3(static_cast<float32>(x), static_cast<float32>(y), static_cast<float32>(z));
        }
        operator Vector3i() {
            return Vector3i(static_cast<int>(x), static_cast<int>(y), static_cast<int>(z));
        }

        operator const Vector3() const {
            return Vector3(static_cast<const float32>(x), static_cast<const float32>(y), static_cast<const float32>(z));
        }
        operator const Vector3i() const {
            return Vector3i(static_cast<const int>(x), static_cast<const int>(y), static_cast<const int>(z));
        }

        bool operator==(const Pos& other) const {
            return x == other.x and y == other.y and z == other.z;
        }
    };

    template <typename T>
    requires std::is_arithmetic_v<T>
    struct PosHash {
        size operator()(const Pos<T>& pos) const {
            return std::hash<T>()(pos.x) ^ (std::hash<T>()(pos.y) << 16) ^ (std::   hash<T>()(pos.z) << 8);
        }
    };
}