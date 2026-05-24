module;

#include <godot_cpp/core/math_defs.hpp>
#include <includes.hpp>
#include <cstdint>

export module misc.types;

export using int8 = int8_t;
export using int16 = int16_t;
export using int32 = int32_t;
export using int64 = int64_t;
export using uint8 = uint8_t;
export using uint16 = uint16_t;
export using uint32 = uint32_t;
export using uint64 = uint64_t;
export using byte = char;
export using ubyte = unsigned char;
export using byte32 = char32_t;
export using float32 = float;
export using float64 = double;
export using real = godot::real_t;
export using size = size_t;
export using none = void;

export class uint128 {
private:
    uint64 __over__;
    uint64 __value__;

public:
    uint128();
    uint128(const uint64& x);
    uint128(const uint128& other);
    uint128(const std::string& other);

    uint128& operator=(const uint64& x);
    uint128& operator=(const uint128& other);

    uint128& operator+=(const uint64& x);
    uint128& operator+=(const uint128& other);
    uint128& operator-=(const uint64& x);
    uint128& operator-=(const uint128& other);
    uint128& operator*=(const uint64& x);
    uint128& operator*=(const uint128& other);
    uint128& operator/=(const uint64& x);
    uint128& operator/=(const uint128& other);
    uint128& operator%=(const uint64& x);
    uint128& operator%=(const uint128& other);
    uint128& operator>>=(const uint64& x);
    uint128& operator>>=(const uint128& x);
    uint128& operator<<=(const uint64& x);
    uint128& operator<<=(const uint128& x);
    uint128& operator&=(const uint64& x);
    uint128& operator&=(const uint128& x);
    uint128& operator|=(const uint64& x);
    uint128& operator|=(const uint128& x);
    uint128& operator^=(const uint64& x);
    uint128& operator^=(const uint128& x);

    uint128 operator+(const uint64& x) const;
    uint128 operator+(const uint128& other) const;
    uint128 operator-(const uint64& x) const;
    uint128 operator-(const uint128& other) const;
    uint128 operator*(const uint64& x) const;
    uint128 operator*(const uint128& other) const;
    uint128 operator/(const uint64& x) const;
    uint128 operator/(const uint128& other) const;
    uint128 operator%(const uint64& x) const;
    uint128 operator%(const uint128& other) const;
    uint128 operator>>(const uint64& x) const;
    uint128 operator>>(const uint128& x) const;
    uint128 operator<<(const uint64& x) const;
    uint128 operator<<(const uint128& x) const;
    uint128 operator&(const uint64& other) const;
    uint128 operator&(const uint128& other) const;
    uint128 operator|(const uint64& other) const;
    uint128 operator|(const uint128& other) const;
    uint128 operator^(const uint64& other) const;
    uint128 operator^(const uint128& other) const;
    uint128 operator~() const;

    std::strong_ordering operator<=>(const uint128& other) const;
    bool operator==(const uint128& other) const;

    static std::string add_big_numbers(const std::string& a, const std::string& b);
    uint64 to_uint64() const;

    friend class int128;

private:
    std::string __str__() const;

    static std::string multiply_big_number(const std::string& num, const int& multiplier);
    static std::string power_of_2_fast(const int& exponent);
    static std::string calculate_fast(const uint64& over, const uint64& value);
    static std::string fast_multiply_by_power_of_2(const uint64& num, const int& power);
    static bool get_bit(const uint128& other, int index);
    static void set_bit(uint128& other, int index, bool value);
    static void divmod(uint128& other, const uint128& divisor, uint128& quotient, uint128& remainder);
};

export class int128 {
private:
    uint128 __value__;

    std::string __str__() const;

public:
    int128();
    int128(const int64& x);
    int128(const int128& other);
    int128(const uint128& other);
    int128(const std::string& other);

    int128& operator=(const int64& x);
    int128& operator=(const int128& x);
    int128& operator+=(const int64& x);
    int128& operator+=(const int128& other);
    int128& operator-=(const int64& x);
    int128& operator-=(const int128& other);
    int128& operator*=(const int64& x);
    int128& operator*=(const int128& other);
    int128& operator/=(const int64& x);
    int128& operator/=(const int128& other);
    int128& operator%=(const int64& x);
    int128& operator%=(const int128& other);
    int128& operator>>=(const int64& x);
    int128& operator<<=(const int64& x);

    int128 operator-() const;
    int128 operator+(const int64& x) const;
    int128 operator+(const int128& other) const;
    int128 operator-(const int64& x) const;
    int128 operator-(const int128& other) const;
    int128 operator*(const int64& x) const;
    int128 operator*(const int128& other) const;
    int128 operator/(const int64& x) const;
    int128 operator/(const int128& other) const;
    int128 operator%(const int64& x) const;
    int128 operator%(const int128& other) const;
    int128 operator>>(const int64& x) const;
    int128 operator<<(const int64& x) const;

    std::strong_ordering operator<=>(const int128& other) const;
    bool operator==(const int128& other) const;
    int64 to_int64() const;
};