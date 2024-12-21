#pragma once
#include <vector>
#include <string>
#include <iostream>
#include <stdexcept>
#include <sstream>

#define UPPORT_MORE_OPS 0
#define SSUPPORT_IFSTREAM 0
#define SUPPORT_EVAL 0 // special bonus

class BigInteger {
private:
    using BaseType = uint32_t; // Base type for digits
    using DoubleBaseType = uint64_t; // For operations that require larger capacity
    static constexpr BaseType BASE = BaseType(1) << (sizeof(BaseType) * 4); // Base is int/2

    std::vector<BaseType> digits; // Digits stored in reverse order
    bool is_negative;

    void removeLeadingZeros() {
        while (digits.size() > 1 && digits.back() == 0) {
            digits.pop_back();
        }
        if (digits.size() == 1 && digits[0] == 0) {
            is_negative = false;
        }
    }

public:
    // constructors
    BigInteger() : is_negative(false) { digits.push_back(0); }

    BigInteger(int64_t n) : is_negative(n < 0) {
        digits.clear();

        if (n == 0) {
            digits.push_back(0); // Handle zero case
            return;
        }

        if (n < 0) n = -n;
        while (n > 0) {
            digits.push_back(n % BASE);
            n /= BASE;
        }
    }

    explicit BigInteger(const std::string& str) {
        if (str.empty()) throw std::invalid_argument("Invalid input string");

        size_t start = 0;
        if (str[0] == '-' || str[0] == '+') {
            start = 1;
        }

        digits.clear();

        static constexpr size_t chunk_size = 9; // Maximum number of decimal digits that fit into BaseType
        BigInteger base(BASE);

        for (size_t i = str.size(); i > start;) {
            size_t chunk_end = i;
            size_t chunk_start = (i >= chunk_size) ? i - chunk_size : start;
            std::string chunk = str.substr(chunk_start, chunk_end - chunk_start);

            // convert the chunk
            BigInteger chunk_value(std::stoll(chunk));

            // Combine the chunk into the current value
            *this *= base;
            *this += chunk_value;

            i = chunk_start;
        }

        is_negative = (str[0] == '-');
        // ensure that the number 0 has a consistent representation
        if (digits.empty()) digits.push_back(0);
    }

    // Copy constructor and assignment operator
    BigInteger(const BigInteger& other) = default;
    BigInteger& operator=(const BigInteger& rhs) = default;

    // Unary operators
    BigInteger operator-() const {
        BigInteger result = *this;
        if (result != 0) {
            result.is_negative = !result.is_negative;
        }
        return result;
    }

    // Arithmetic operators
    BigInteger& operator+=(const BigInteger& rhs) {
        if (is_negative == rhs.is_negative) {
            BaseType carry = 0;
            size_t max_size = std::max(digits.size(), rhs.digits.size());
            digits.resize(max_size, 0);

            for (size_t i = 0; i < max_size || carry; ++i) {
                if (i == digits.size()) digits.push_back(0);
                DoubleBaseType sum = DoubleBaseType(digits[i]) +
                                     (i < rhs.digits.size() ? rhs.digits[i] : 0) + carry;
                digits[i] = sum % BASE;
                carry = sum / BASE;
            }
        } else {
            *this -= -rhs;
        }
        removeLeadingZeros();
        return *this;
    }

    BigInteger& operator-=(const BigInteger& rhs) {
        if (is_negative == rhs.is_negative) {
            if (abs() >= rhs.abs()) {
                BaseType borrow = 0;
                for (size_t i = 0; i < rhs.digits.size() || borrow; ++i) {
                    DoubleBaseType sub = DoubleBaseType(digits[i]) -
                                         (i < rhs.digits.size() ? rhs.digits[i] : 0) - borrow;
                    borrow = sub >= BASE;
                    if (borrow) sub += BASE;
                    digits[i] = sub;
                }
                removeLeadingZeros();
            } else {
                BigInteger temp = rhs;
                temp -= *this;
                *this = -temp;
            }
        } else {
            *this += -rhs;
        }
        return *this;
    }

    BigInteger& operator*=(const BigInteger& rhs) {
        BigInteger result;
        result.digits.resize(digits.size() + rhs.digits.size(), 0);

        for (size_t i = 0; i < digits.size(); ++i) {
            DoubleBaseType carry = 0;
            for (size_t j = 0; j < rhs.digits.size() || carry; ++j) {
                DoubleBaseType prod = result.digits[i + j] +
                                      DoubleBaseType(digits[i]) *
                                          (j < rhs.digits.size() ? rhs.digits[j] : 0) +
                                      carry;
                result.digits[i + j] = prod % BASE;
                carry = prod / BASE;
            }
        }

        result.is_negative = is_negative != rhs.is_negative;
        result.removeLeadingZeros();
        *this = result;
        return *this;
    }

    BigInteger& operator/=(const BigInteger& rhs) {
        *this = divide_with_remainder(rhs).first;
        return *this;
    }

    BigInteger& operator%=(const BigInteger& rhs) {
        *this = divide_with_remainder(rhs).second;
        return *this;
    }

    std::pair<BigInteger, BigInteger> divide_with_remainder(const BigInteger& divisor) {
        if (divisor == 0) {
            throw std::invalid_argument("Division by zero");
        }

        auto temp_is_negative = is_negative;
        is_negative = false;

        BigInteger dividend = *this;
        BigInteger quotient(0);

        BigInteger denom = divisor;
        BigInteger current(1);

        // Ensure denom is not larger than dividend initially
        if (denom > dividend) {
            return {BigInteger(0), dividend};
        }

        // Shift denom and current left until denom is larger than dividend
        while (denom <= dividend) {
            denom <<= 1;
            current <<= 1;
        }

        // Shift denom and current back to the highest valid position
        denom >>= 1;
        current >>= 1;

        // Perform the division using bitwise OR to accumulate the quotient
        while (current != BigInteger(0)) {
            if (dividend >= denom) {
                dividend -= denom;
                quotient |= current; // Accumulate the current power of two in the quotient
            }
            denom >>= 1;
            current >>= 1;
        }

       BigInteger remainder = dividend;

        quotient.is_negative = is_negative != divisor.is_negative;
        remainder.is_negative = is_negative;

        quotient.removeLeadingZeros();
        remainder.removeLeadingZeros();

        is_negative = temp_is_negative;

        return {quotient, remainder};
    }

    // Spaceship operator (<=>)
    std::strong_ordering operator<=>(const BigInteger& rhs) const {
        if (is_negative != rhs.is_negative) {
            return is_negative ? std::strong_ordering::less : std::strong_ordering::greater;
        }

        if (digits.size() != rhs.digits.size()) {
            return (digits.size() < rhs.digits.size()) == is_negative
                       ? std::strong_ordering::greater
                       : std::strong_ordering::less;
        }

        for (size_t i = digits.size(); i-- > 0;) {
            if (digits[i] != rhs.digits[i]) {
                return (digits[i] < rhs.digits[i]) == is_negative
                           ? std::strong_ordering::greater
                           : std::strong_ordering::less;
            }
        }
        return std::strong_ordering::equal;
    }

    // Equality and Inequality Operators
    bool operator==(const BigInteger& rhs) const {
        return (*this <=> rhs) == 0; // Use the spaceship operator
    }

    bool operator!=(const BigInteger& rhs) const {
        return !(*this == rhs); // Use the equality operator
    }

    // bitwise operators
    BigInteger& operator<<=(size_t shift) {
        size_t base_shifts = shift / (sizeof(BaseType) * 8); // Full base shifts
        size_t bit_shifts = shift % (sizeof(BaseType) * 8); // Remaining bit shifts

        // shift the digits vector
        if (base_shifts > 0) {
            digits.insert(digits.begin(), base_shifts, 0); // Insert zeros at the front
        }

        if (bit_shifts > 0) {
            BaseType carry = 0;
            for (size_t i = 0; i < digits.size(); ++i) {
                DoubleBaseType shifted = (DoubleBaseType(digits[i]) << bit_shifts) | carry;
                digits[i] = BaseType(shifted % BASE);
                carry = BaseType(shifted / BASE);
            }
            if (carry > 0) {
                digits.push_back(carry);
            }
        }

        return *this;
    }

    BigInteger& operator>>=(size_t shift) {
        size_t base_shifts = shift / (sizeof(BaseType) * 8); // Full base shifts
        size_t bit_shifts = shift % (sizeof(BaseType) * 8); // Remaining bit shifts

        // Shift the digits vector
        if (base_shifts >= digits.size()) {
            // If the shift is greater than the size, the number becomes 0
            digits.clear();
            digits.push_back(0);
            is_negative = false;
            return *this;
        }

        digits.erase(digits.begin(), digits.begin() + base_shifts); // Remove base_shifts elements

        if (bit_shifts > 0) {
            BaseType carry = 0;
            for (size_t i = digits.size(); i-- > 0;) {
                DoubleBaseType shifted = (DoubleBaseType(digits[i]) | (DoubleBaseType(carry) << (sizeof(BaseType) * 8 - bit_shifts))) >> bit_shifts;
                carry = digits[i] & ((1 << bit_shifts) - 1); // Save the remainder bits
                digits[i] = BaseType(shifted);
            }
        }

        removeLeadingZeros();
        return *this;
    }

    BigInteger& operator|=(const BigInteger& rhs) {
        size_t max_size = std::max(digits.size(), rhs.digits.size());
        digits.resize(max_size, 0);

        for (size_t i = 0; i < rhs.digits.size(); ++i) {
            digits[i] |= rhs.digits[i];
        }

        return *this;
    }

    double sqrt() const {
        if (is_negative) {
            throw std::runtime_error("Cannot compute square root of a negative number");
        }

        if (*this == 0) {
            return 0.0;
        }

        double value = 0.0;
        double base_multiplier = 1.0;

        for (size_t i = 0; i < digits.size(); ++i) {
            if (base_multiplier > std::numeric_limits<double>::max() / BASE) {
                throw std::runtime_error("Number is too large to convert to double for sqrt");
            }

            value += digits[i] * base_multiplier;
            base_multiplier *= BASE;
        }

        if (value > std::numeric_limits<double>::max()) {
            throw std::runtime_error("Number is too large to compute sqrt in double precision");
        }

        return std::sqrt(value);
    }

    friend std::ostream& operator<<(std::ostream& os, const BigInteger& n) {
        if (n.digits.empty()) {
            os << '0';
            return os;
        }

        if (n.is_negative) {
            os << '-';
        }

        BigInteger temp = n; // make a copy since we will modify it
        std::ostringstream result;

        const BigInteger ten(10); // base 10 for extraction
        BigInteger remainder;
        // extract digits in base 10
        while (temp != BigInteger()) {
            auto division_result = temp.divide_with_remainder(ten);
            temp = division_result.first;      // quotient
            remainder = division_result.second;// remainder
            result << remainder.digits[0];     // store remainder (base-10 digit)
        }

        std::string reversed_result = result.str();
        std::reverse(reversed_result.begin(), reversed_result.end());

        os << reversed_result;
        return os;
    }

    // helper methods
    [[nodiscard]] BigInteger abs() const {
        BigInteger result = *this;
        result.is_negative = false;
        return result;
    }
};

// binary operators for BigInteger
inline BigInteger operator+(BigInteger lhs, const BigInteger& rhs) {
    lhs += rhs;
    return lhs;
}

inline BigInteger operator-(BigInteger lhs, const BigInteger& rhs) {
    lhs -= rhs;
    return lhs;
}

inline BigInteger operator*(BigInteger lhs, const BigInteger& rhs) {
    lhs *= rhs;
    return lhs;
}

inline BigInteger operator/(BigInteger lhs, const BigInteger& rhs) {
    lhs /= rhs;
    return lhs;
}

inline BigInteger operator%(BigInteger lhs, const BigInteger& rhs) {
    lhs %= rhs;
    return lhs;
}
