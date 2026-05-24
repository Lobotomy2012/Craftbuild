module;

#include <includes.hpp>
#include <string>
#include <stdexcept>
#include <vector>
#include <compare>

module misc.types;

// uint128
uint128::uint128() : __value__(0), __over__(0) {}
uint128::uint128(const uint64& x) : __value__(x), __over__(0) {}
uint128::uint128(const uint128& other) : __value__(other.__value__), __over__(other.__over__) {}
uint128::uint128(const std::string& x) : __value__(0), __over__(0) {
    if (x.empty()) {
        return;
    }
    if (x[0] == '-') {
        throw std::runtime_error("uint128 cannot be negative");
    }

    __value__ = 0;
    __over__ = 0;

    for (char c : x) {
        if (c < '0' || c > '9') {
            throw std::runtime_error("Invalid literal for uint128(): " + std::string(1, c));
        }

        operator*=(10);
        operator+=(c - '0');
    }
}

uint128& uint128::operator=(const uint64& x) {
    __value__ = x;
    __over__ = 0;
    return *this;
}
uint128& uint128::operator=(const uint128& other) {
    __value__ = other.__value__;
    __over__ = other.__over__;
    return *this;
}
uint128& uint128::operator+=(const uint64& x) {
    return operator+=(uint128(x));
}
uint128& uint128::operator+=(const uint128& other) {
    uint64_t old_value = __value__;
    __value__ += other.__value__;
    __over__ += other.__over__ + (__value__ < old_value);
    return *this;
}
uint128& uint128::operator-=(const uint64& x) {
    return operator-=(uint128(x));
}
uint128& uint128::operator-=(const uint128& other) {
    uint64_t old_value = __value__;
    __value__ -= other.__value__;
    __over__ -= other.__over__ + (__value__ > old_value);
    return *this;
}
uint128& uint128::operator*=(const uint64& x) {
    return operator*=(uint128(x));
}
uint128& uint128::operator*=(const uint128& other) {
    if (other == uint128(0)) {
        __value__ = 0;
        __over__ = 0;
        return *this;
    }
    if (other == uint128(1)) return *this;
    if (other == uint128(2)) {
        operator+=(*this);
        return *this;
    }

    uint128 cache = *this;
    for (uint128 i = other; i > 0; i >>= 1) {
        if ((i.__value__ & 1) == 1) {
            operator+=(cache);
        }
        cache += cache;
    }
    return *this;
}
uint128& uint128::operator/=(const uint64& other) {
    uint128 q, r;
    divmod(*this, other, q, r);
    *this = q;
    return *this;
}
uint128& uint128::operator/=(const uint128& other) {
    uint128 q, r;
    divmod(*this, other, q, r);
    *this = q;
    return *this;
}
uint128& uint128::operator%=(const uint64& other) {
    uint128 q, r;
    divmod(*this, other, q, r);
    *this = r;
    return *this;
}
uint128& uint128::operator%=(const uint128& other) {
    uint128 q, r;
    divmod(*this, other, q, r);
    *this = r;
    return *this;
}
uint128& uint128::operator>>=(const uint64& x) {
    return operator>>=(uint128(x));
}
uint128& uint128::operator>>=(const uint128& x) {
    if (x >= 64) {
        __value__ = __over__ >> (x.to_uint64() - 64);
        __over__ = 0;
    }
    else {
        __value__ = (__over__ << (64 - x.to_uint64())) | (__value__ >> x.to_uint64());
        __over__ >>= x.to_uint64();
    }
    return *this;
}
uint128& uint128::operator<<=(const uint64& x) {
    return operator<<=(uint128(x));
}
uint128& uint128::operator<<=(const uint128& x) {
    if (x >= 64) {
        __over__ = __value__ << (x.to_uint64() - 64);
        __value__ = 0;
    }
    else {
        __over__ = (__over__ << (64 - x.to_uint64())) | (__value__ >> x.to_uint64());
        __value__ <<= x.to_uint64();
    }
    return *this;
}
uint128& uint128::operator&=(const uint64& x) {
    return operator&=(uint128(x));
}
uint128& uint128::operator&=(const uint128& other) {
    __value__ &= other.__value__;
    __over__ &= other.__over__;
    return *this;
}
uint128& uint128::operator|=(const uint64& x) {
    return operator|=(uint128(x));
}
uint128& uint128::operator|=(const uint128& other) {
    __value__ |= other.__value__;
    __over__ |= other.__over__;
    return *this;
}
uint128& uint128::operator^=(const uint64& x) {
    return operator^=(uint128(x));
}
uint128& uint128::operator^=(const uint128& other) {
    __value__ ^= other.__value__;
    __over__ ^= other.__over__;
    return *this;
}

uint128 uint128::operator+(const uint64& x) const {
    uint128 result = *this;
    result += x;
    return result;
}
uint128 uint128::operator+(const uint128& other) const {
    uint128 result = *this;
    result += other;
    return result;
}
uint128 uint128::operator-(const uint64& x) const {
    uint128 result = *this;
    result -= x;
    return result;
}
uint128 uint128::operator-(const uint128& other) const {
    uint128 result = *this;
    result -= other;
    return result;
}
uint128 uint128::operator*(const uint64& x) const {
    uint128 result = *this;
    result *= x;
    return result;
}
uint128 uint128::operator*(const uint128& other) const {
    uint128 result = *this;
    result *= other;
    return result;
}
uint128 uint128::operator/(const uint64& other) const {
    uint128 result = *this;
    result /= other;
    return result;
}
uint128 uint128::operator/(const uint128& other) const {
    uint128 result = *this;
    result /= other;
    return result;
}
uint128 uint128::operator%(const uint64& other) const {
    uint128 result = *this;
    result %= other;
    return result;
}
uint128 uint128::operator%(const uint128& other) const {
    uint128 result = *this;
    result %= other;
    return result;
}
uint128 uint128::operator>>(const uint64& x) const {
    uint128 result = *this;
    result >>= x;
    return result;
}
uint128 uint128::operator>>(const uint128& x) const {
    uint128 result = *this;
    result >>= x;
    return result;
}
uint128 uint128::operator<<(const uint64& x) const {
    uint128 result = *this;
    result <<= x;
    return result;
}
uint128 uint128::operator<<(const uint128& x) const {
    uint128 result = *this;
    result <<= x;
    return result;
}
uint128 uint128::operator&(const uint64& x) const {
    uint128 result = *this;
    result &= x;
    return result;
}
uint128 uint128::operator&(const uint128& other) const {
    uint128 result = *this;
    result &= other;
    return result;
}
uint128 uint128::operator|(const uint64& x) const {
    uint128 result = *this;
    result |= x;
    return result;
}
uint128 uint128::operator|(const uint128& other) const {
    uint128 result = *this;
    result |= other;
    return result;
}
uint128 uint128::operator^(const uint64& x) const {
    uint128 result = *this;
    result ^= x;
    return result;
}
uint128 uint128::operator^(const uint128& other) const {
    uint128 result = *this;
    result ^= other;
    return result;
}
uint128 uint128::operator~() const {
    uint128 result = *this;
    result ^= (uint128(0xFFFFFFFFFFFFFFFFULL) + 0xFFFFFFFFFFFFFFFFULL);
    return result;
}

std::strong_ordering uint128::operator<=>(const uint128& other) const {
    if (__over__ != other.__over__) {
        return __over__ < other.__over__ ? std::strong_ordering::less : std::strong_ordering::greater;
    }
    if (__value__ < other.__value__) return std::strong_ordering::less;
    else if (__value__ > other.__value__) return std::strong_ordering::greater;

    return std::strong_ordering::equal;
}

bool uint128::operator==(const uint128& other) const {
    return (__over__ == other.__over__) && (__value__ == other.__value__);
}

std::string uint128::__str__() const {
    if (__over__ == 0) return std::to_string(__value__);

    return calculate_fast(__over__, __value__);
}

std::string uint128::add_big_numbers(const std::string& a, const std::string& b) {
    if (a.empty() and b.empty()) return "0";
    if (a.empty()) return b;
    if (b.empty()) return a;

    std::string result;
    const int GROUP_SIZE = 5;
    const int64_t BASE = 100000;

    int __len___a = static_cast<int>(a.size());
    int __len___b = static_cast<int>(b.size());
    int max___len__ = std::max(__len___a, __len___b);

    int64_t carry = 0;
    int pos = 0;

    while (pos < max___len__ or carry > 0) {
        int64_t sum = carry;

        if (pos < __len___a) {
            int start_a = std::max(0, __len___a - pos - GROUP_SIZE);
            int end_a = __len___a - pos;
            int group___len___a = end_a - start_a;

            if (group___len___a > 0) {
                int64_t group_val = stoll(a.substr(start_a, group___len___a));
                sum += group_val;
            }
        }

        if (pos < __len___b) {
            int start_b = std::max(0, __len___b - pos - GROUP_SIZE);
            int end_b = __len___b - pos;
            int group___len___b = end_b - start_b;

            if (group___len___b > 0) {
                int64_t group_val = stoll(b.substr(start_b, group___len___b));
                sum += group_val;
            }
        }

        carry = sum / BASE;
        int64_t remainder = sum % BASE;

        std::string group_str = std::to_string(remainder);
        if (pos + GROUP_SIZE < max___len__ or carry > 0) {
            group_str = std::string(GROUP_SIZE - group_str.size(), '0') + group_str;
        }

        result = group_str + result;
        pos += GROUP_SIZE;
    }

    size_t non_zero_pos = result.find_first_not_of('0');
    if (non_zero_pos == std::string::npos) {
        return "0";
    }

    return result.substr(non_zero_pos);
}

uint64 uint128::to_uint64() const {
    if (__over__ != 0) {
        throw std::overflow_error("uint128 value too large to convert to uint64");
    }
    return __value__;
}

std::string uint128::multiply_big_number(const std::string& num, const int& multiplier) {
    if (multiplier == 0) return "0";
    if (multiplier == 1) return num;

    std::string result;
    int carry = 0;

    for (int64_t i = static_cast<int64_t>(num.length()) - 1; i >= 0; i--) {
        int product = (num[i] - '0') * multiplier + carry;
        result.push_back((product % 10) + '0');
        carry = product / 10;
    }

    while (carry > 0) {
        result.push_back((carry % 10) + '0');
        carry /= 10;
    }

    reverse(result.begin(), result.end());
    return result;
}

std::string uint128::power_of_2_fast(const int& exponent) {
    if (exponent == 0) return "1";

    std::vector<std::string> cache;
    cache.push_back("1");

    for (int i = 1; i <= exponent; i++) {
        std::string prev = cache.back();
        cache.push_back(add_big_numbers(prev, prev));
    }

    return cache[exponent];
}

std::string uint128::calculate_fast(const uint64_t& over, const uint64_t& value) {
    if (over == 0) return std::to_string(value);

    if (over == 1) {
        if (value >= 1) {
            return add_big_numbers("18446744073709551616", std::to_string(value - 1));
        }
        else {
            return "18446744073709551615";
        }
    }

    std::string base_2_64 = "18446744073709551616";
    std::string high_part = "0";
    std::string over_str = std::to_string(over);

    for (int i = 0; i < over_str.length(); i++) {
        int digit = over_str[i] - '0';
        if (digit > 0) {
            std::string partial = base_2_64;
            for (int j = 0; j < over_str.length() - 1 - i; j++) {
                partial += "0";
            }

            for (int j = 0; j < digit; j++) {
                high_part = add_big_numbers(high_part, partial);
            }
        }
    }

    if (value >= over) {
        return add_big_numbers(high_part, std::to_string(value - over));
    }
    else {
        return add_big_numbers(high_part, std::to_string(value));
    }
}

std::string uint128::fast_multiply_by_power_of_2(const uint64& num, const int& power) {
    if (num == 0) return "0";
    std::string num_str = std::to_string(num);
    std::string power_result = power_of_2_fast(power);

    if (num == 1) return power_result;

    std::string result = "0";
    for (int i = 0; i < num_str.length(); i++) {
        int digit = num_str[i] - '0';
        if (digit > 0) {
            std::string partial = power_result;
            for (int j = 0; j < num_str.length() - 1 - i; j++) {
                partial += "0";
            }
            result = add_big_numbers(result, multiply_big_number(partial, digit));
        }
    }

    return result;
}
bool uint128::get_bit(const uint128& other, int index) {
    if (index >= 64)
        return (other.__over__ >> (index - 64)) & 1;
    else
        return (other.__value__ >> index) & 1;
}
void uint128::set_bit(uint128& other, int index, bool value) {
    if (index >= 64) {
        if (value) other.__over__ |= (uint64(1) << (index - 64));
        else other.__over__ &= ~(uint64(1) << (index - 64));
    }
    else {
        if (value)
            other.__value__ |= (uint64(1) << index);
        else
            other.__value__ &= ~(uint64(1) << index);
    }
}
void uint128::divmod(uint128& other, const uint128& divisor, uint128& quotient, uint128& remainder) {
    if (divisor == uint128(0)) {
        throw std::runtime_error("Division by zero");
    }

    quotient = uint128(0);
    remainder = uint128(0);

    for (int i = 127; i >= 0; i--) {
        remainder <<= 1;

        if (get_bit(other, i)) {
            remainder.__value__ |= 1;
        }

        if (remainder >= divisor) {
            remainder -= divisor;
            set_bit(other, i, true);
        }
    }
}

// int128
int128::int128() : __value__(0) {}
int128::int128(const int64& x) {
    if (x >= 0) {
        __value__ = uint128(static_cast<uint64_t>(x));
    }
    else {
        uint64_t abs_x = static_cast<uint64_t>(-x);
        __value__ = ~(uint128(abs_x)) + 1;
    }
}

int128::int128(const uint128& x) : __value__(x) {}
int128::int128(const int128& other) : __value__(other.__value__) {}
int128::int128(const std::string& x) : __value__(0) {
    if (x.empty()) return;
    if (x[0] == '-') {
        uint128 abs_val = uint128(x.substr(1));
        __value__ = ~abs_val + 1;
    }
    else {
        __value__ = uint128(x);
    }
}

int128& int128::operator=(const int64& x) {
    if (x >= 0) {
        __value__ = uint128(static_cast<uint64_t>(x));
    }
    else {
        uint64_t abs_x = static_cast<uint64_t>(-x);
        __value__ = ~(uint128(abs_x)) + 1;
    }
    return *this;
}

int128& int128::operator=(const int128& other) {
    __value__ = other.__value__;
    return *this;
}

int128& int128::operator+=(const int64& x) {
    return operator+=(int128(x));
}
int128& int128::operator+=(const int128& other) {
    __value__ += other.__value__;
    return *this;
}

int128& int128::operator-=(const int64& x) {
    return operator-=(int128(x));
}
int128& int128::operator-=(const int128& other) {
    __value__ -= other.__value__;
    return *this;
}

int128& int128::operator*=(const int64& x) {
    return operator*=(int128(x));
}
int128& int128::operator*=(const int128& other) {
    __value__ *= other.__value__;
    return *this;
}

int128& int128::operator/=(const int64& x) {
    return operator/=(int128(x));
}
int128& int128::operator/=(const int128& other) {
    if (other.__value__ == uint128(0)) throw std::runtime_error("Division by zero");

    bool this_neg = (__value__ >> 127) == uint128(1);
    bool other_neg = (other.__value__ >> 127) == uint128(1);

    uint128 a = this_neg ? (~__value__ + 1) : __value__;
    uint128 b = other_neg ? (~other.__value__ + 1) : other.__value__;

    uint128 q = a / b;

    __value__ = (this_neg != other_neg) ? (~q + 1) : q;
    return *this;
}

int128& int128::operator%=(const int64& other) {
    return operator%=(int128(other));
}
int128& int128::operator%=(const int128& other) {
    if (other.__value__ == uint128(0)) throw std::runtime_error("Division by zero");

    bool this_neg = (__value__ >> 127) == uint128(1);
    bool other_neg = (other.__value__ >> 127) == uint128(1);

    uint128 a = this_neg ? (~__value__ + 1) : __value__;
    uint128 b = other_neg ? (~other.__value__ + 1) : other.__value__;

    uint128 r = a % b;

    __value__ = this_neg ? (~r + 1) : r;
    return *this;
}

int128& int128::operator>>=(const int64& x) {
    bool neg = (__value__ >> 127) == uint128(1);
    __value__ >>= x;

    if (neg) {
        uint64_t shift = (uint64_t)x;
        if (shift > 0) {
            if (shift >= 128) {
                __value__ = ~uint128(0);
            }
            else {
                uint128 mask = (~uint128(0)) << (128 - shift);
                __value__ |= mask;
            }
        }
    }
    return *this;
}

int128& int128::operator<<=(const int64& x) {
    __value__ <<= x;
    return *this;
}

int128 int128::operator-() const {
    int128 result;
    result.__value__ = ~__value__ + 1;
    return result;
}

int128 int128::operator+(const int64& x) const { return int128(*this) += x; }
int128 int128::operator+(const int128& other) const { return int128(*this) += other; }
int128 int128::operator-(const int64& x) const { return int128(*this) -= x; }
int128 int128::operator-(const int128& other) const { return int128(*this) -= other; }
int128 int128::operator*(const int64& x) const { return int128(*this) *= x; }
int128 int128::operator*(const int128& other) const { return int128(*this) *= other; }
int128 int128::operator/(const int64& x) const { return int128(*this) /= x; }
int128 int128::operator/(const int128& other) const { return int128(*this) /= other; }
int128 int128::operator%(const int64& x) const { return int128(*this) %= x; }
int128 int128::operator%(const int128& other) const { return int128(*this) %= other; }
int128 int128::operator>>(const int64& x) const { return int128(*this) >>= x; }
int128 int128::operator<<(const int64& x) const { return int128(*this) <<= x; }

std::strong_ordering int128::operator<=>(const int128& other) const {
    bool this_neg = (__value__ >> 127) == uint128(1);
    bool other_neg = (other.__value__ >> 127) == uint128(1);

    if (this_neg != other_neg) {
        return this_neg ? std::strong_ordering::less : std::strong_ordering::greater;
    }

    return __value__ <=> other.__value__;
}

bool int128::operator==(const int128& other) const {
    return __value__ == other.__value__;
}

int64_t int128::to_int64() const {
    bool is_neg = (__value__ >> 127) == uint128(1);
    if (is_neg) {
        uint128 abs_val = ~__value__ + 1;
        if (abs_val > uint128(9223372036854775808ULL)) {
            throw std::overflow_error("int128 value too small to convert to int64_t");
        }
        if (abs_val == uint128(9223372036854775808ULL)) {
            return -9223372036854775807LL - 1;
        }
        return -(int64_t)(abs_val.to_uint64());
    }
    else {
        if (__value__ > uint128(9223372036854775807ULL)) {
            throw std::overflow_error("int128 value too large to convert to int64_t");
        }
        return (int64_t)(__value__.to_uint64());
    }
}

std::string int128::__str__() const {
    static thread_local char buf[256];
    bool is_neg = (__value__ >> 127) == uint128(1);

    if (is_neg) {
        uint128 abs_val = ~__value__ + 1;
        snprintf(buf, sizeof(buf), "-%s", abs_val.__str__());
    }
    else {
        snprintf(buf, sizeof(buf), "%s", __value__.__str__());
    }
    return buf;
}