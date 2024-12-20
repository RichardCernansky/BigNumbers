#pragma once
#include <vector>
#include <string>
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <numeric>
#include <sstream>

class BigInteger {
private:
    std::vector<int> digits; // Digits stored in reverse order
    bool is_negative;

    void removeLeadingZeros() {
        while (digits.size() > 1 && digits.back() == 0) {
            digits.pop_back();
        }
        if (digits.size() == 1 && digits[0] == 0) { // check for "-000"
            is_negative = false;
        }
    }

public:
    // constructors
    BigInteger() : is_negative(false) { digits.push_back(0); }

    BigInteger(int64_t n) : is_negative(n < 0) {
        if (n < 0) n = -n; // if n negative, make it positive for following algorithm
        else if (n == 0) digits.push_back(0);
        while (n > 0) {
            digits.push_back(n % 10);
            n /= 10;
        }
    }

    explicit BigInteger(const std::string& str) {
        if (str.empty()) throw std::invalid_argument("Invalid input string");

        size_t start = 0;
        is_negative = (str[0] == '-');
        if (str[0] == '-' || str[0] == '+') {
            start = 1;
        }

        for (size_t i = str.size(); i > start; --i) {
            if (!isdigit(str[i - 1])) {
                throw std::invalid_argument("Invalid input string");
            }
            digits.push_back(str[i - 1] - '0');
        }
        removeLeadingZeros();
    }
    // copy constructor
    BigInteger(const BigInteger& other) = default;
    // assignment operator
    BigInteger& operator=(const BigInteger& rhs) = default;

    // Unary operators
    const BigInteger& operator+() const { return *this; } //no-op

    BigInteger operator-() const {
        BigInteger result = *this;
        if (result != 0) { // check for zero
            result.is_negative = !result.is_negative; // flip negativity
        }
        return result;
    }

    // Arithmetic operators:
    BigInteger& operator+=(const BigInteger& rhs) {
        if (is_negative == rhs.is_negative) {
            int carry = 0;
            size_t max_size = std::max(digits.size(), rhs.digits.size());
            digits.resize(max_size, 0);

            for (size_t i = 0; i < max_size || carry; ++i) {
                if (i == digits.size()) digits.push_back(0);
                digits[i] += (i < rhs.digits.size() ? rhs.digits[i] : 0) + carry;
                carry = digits[i] / 10;
                digits[i] %= 10;
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
                int borrow = 0;
                for (size_t i = 0; i < rhs.digits.size() || borrow; ++i) {
                    digits[i] -= (i < rhs.digits.size() ? rhs.digits[i] : 0) + borrow;
                    borrow = digits[i] < 0;
                    if (borrow) digits[i] += 10;
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
            int carry = 0;
            for (size_t j = 0; j < rhs.digits.size() || carry; ++j) {
                long long cur = result.digits[i + j] +
                               digits[i] * (j < rhs.digits.size() ? rhs.digits[j] : 0) + carry;
                result.digits[i + j] = cur % 10;
                carry = cur / 10;
            }
        }

        result.is_negative = is_negative != rhs.is_negative;
        result.removeLeadingZeros();
        *this = result;
        return *this;
    }

    BigInteger& operator/=(const BigInteger& rhs) {
        if (rhs == 0) throw std::invalid_argument("Division by zero");

        BigInteger dividend = abs();
        BigInteger divisor = rhs.abs();
        BigInteger quotient;
        BigInteger current;

        quotient.digits.resize(dividend.digits.size(), 0);

        for (size_t i = dividend.digits.size(); i-- > 0;) {
            current.digits.insert(current.digits.begin(), dividend.digits[i]);
            current.removeLeadingZeros();
            int x = 0, l = 0, r = 10;

            while (l <= r) {
                int m = (l + r) / 2;
                BigInteger t = divisor * m;
                if (t <= current) {
                    x = m;
                    l = m + 1;
                } else {
                    r = m - 1;
                }
            }

            quotient.digits[i] = x;
            current -= divisor * x;
        }

        quotient.is_negative = is_negative != rhs.is_negative;
        quotient.removeLeadingZeros();
        *this = quotient;
        return *this;
    }

    BigInteger& operator%=(const BigInteger& rhs) {
        *this -= (*this / rhs) * rhs;
        return *this;
    }

    // Comparison operators
    friend bool operator<(const BigInteger& lhs, const BigInteger& rhs) {
        if (lhs.is_negative != rhs.is_negative) return lhs.is_negative;
        if (lhs.digits.size() != rhs.digits.size()) {
            return lhs.is_negative ? lhs.digits.size() > rhs.digits.size()
                                   : lhs.digits.size() < rhs.digits.size();
        }
        for (size_t i = lhs.digits.size(); i-- > 0;) {
            if (lhs.digits[i] != rhs.digits[i]) {
                return lhs.is_negative ? lhs.digits[i] > rhs.digits[i]
                                       : lhs.digits[i] < rhs.digits[i];
            }
        }
        return false;
    }

    friend bool operator==(const BigInteger& lhs, const BigInteger& rhs) {
        return lhs.is_negative == rhs.is_negative && lhs.digits == rhs.digits;
    }

    friend bool operator!=(const BigInteger& lhs, const BigInteger& rhs) {
        return !(lhs == rhs);
    }

    friend bool operator<=(const BigInteger& lhs, const BigInteger& rhs) {
        return !(rhs < lhs);
    }

    friend bool operator>(const BigInteger& lhs, const BigInteger& rhs) {
        return rhs < lhs;
    }

    friend bool operator>=(const BigInteger& lhs, const BigInteger& rhs) {
        return !(lhs < rhs);
    }

    // stream support
    friend std::ostream& operator<<(std::ostream& os, const BigInteger& n) {
        if (n.is_negative) os << '-';
        for (size_t i = n.digits.size(); i-- > 0;) {
            os << n.digits[i];
        }
        return os;
    }

    // helper method
    BigInteger abs() const {
        BigInteger result = *this;
        result.is_negative = false;
        return result;
    }
};

class BigRational {
private:
    BigInteger numerator;
    BigInteger denominator;

    void normalize() {
        if (denominator < 0) {
            numerator = -numerator;
            denominator = -denominator;
        }
        BigInteger gcd = std::gcd(numerator.abs(), denominator.abs());
        numerator /= gcd;
        denominator /= gcd;
    }

public:
    // Constructors
    BigRational() : numerator(0), denominator(1) {}

    BigRational(int64_t a, int64_t b) : numerator(a), denominator(b) {
        if (b == 0) throw std::invalid_argument("Denominator cannot be zero");
        normalize();
    }

    BigRational(const std::string& a, const std::string& b)
        : numerator(BigInteger(a)), denominator(BigInteger(b)) {
        if (denominator == 0) throw std::invalid_argument("Denominator cannot be zero");
        normalize();
    }

    // Arithmetic operators
    BigRational& operator+=(const BigRational& rhs) {
        numerator = numerator * rhs.denominator + rhs.numerator * denominator;
        denominator *= rhs.denominator;
        normalize();
        return *this;
    }

    BigRational& operator-=(const BigRational& rhs) {
        numerator = numerator * rhs.denominator - rhs.numerator * denominator;
        denominator *= rhs.denominator;
        normalize();
        return *this;
    }

    BigRational& operator*=(const BigRational& rhs) {
        numerator *= rhs.numerator;
        denominator *= rhs.denominator;
        normalize();
        return *this;
    }

    BigRational& operator/=(const BigRational& rhs) {
        if (rhs.numerator == 0) throw std::invalid_argument("Division by zero");
        numerator *= rhs.denominator;
        denominator *= rhs.numerator;
        normalize();
        return *this;
    }

    // Stream support
    friend std::ostream& operator<<(std::ostream& os, const BigRational& n) {
        os << n.numerator;
        if (n.denominator != 1) {
            os << '/' << n.denominator;
        }
        return os;
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

// binary operators for BigRational
inline BigRational operator+(BigRational lhs, const BigRational& rhs) {
    lhs += rhs;
    return lhs;
}

inline BigRational operator-(BigRational lhs, const BigRational& rhs) {
    lhs -= rhs;
    return lhs;
}