#pragma once
#include <vector>
#include <string>
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <cmath>
#include <iomanip>
#include <cctype>

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

    // helper methods
    [[nodiscard]] BigInteger abs() const {
        BigInteger result = *this;
        result.is_negative = false;
        return result;
    }

    // bitwise operators
    BigInteger& operator<<=(size_t shift) {
        size_t base_shifts = shift / (sizeof(BaseType) * 4); // Full base shifts
        size_t bit_shifts = shift % (sizeof(BaseType) * 4); // Remaining bit shifts

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
        size_t base_shifts = shift / (sizeof(BaseType) * 4); // Full base shifts (digits to drop)
        size_t bit_shifts = shift % (sizeof(BaseType) * 4); // Remaining bit shifts within a digit

        // Handle full base shifts
        if (base_shifts >= digits.size()) {
            // If the shift is greater than or equal to the size, the number becomes 0
            digits.clear();
            digits.push_back(0);
            is_negative = false;
            return *this;
        }

        // Remove the full base shifts
        digits.erase(digits.begin(), digits.begin() + base_shifts);

        // Handle remaining bit shifts
        if (bit_shifts > 0) {
            BaseType carry = 0;
            for (size_t i = digits.size(); i-- > 0;) {
                // First, shift the current digit to the right
                BaseType shifted_digit = digits[i] >> bit_shifts;

                // Add the carry bits shifted into the high positions of the digit
                shifted_digit |= (carry << (sizeof(BaseType) * 4 - bit_shifts));

                // Update the carry to the lower bits of the current digit before the shift
                carry = digits[i] & ((1 << bit_shifts) - 1);

                // Store the result back in the digit
                digits[i] = shifted_digit;
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

    // Declare friends for operators
    friend std::strong_ordering operator<=>(const BigInteger& lhs, const BigInteger& rhs);
    friend bool operator==(const BigInteger& lhs, const BigInteger& rhs);
    friend bool operator!=(const BigInteger& lhs, const BigInteger& rhs);
    friend std::ostream& operator<<(std::ostream& os, const BigInteger& n);

public:
    // constructors
    BigInteger() : is_negative(false) { digits.push_back(0); }

    BigInteger(int64_t n) : is_negative(n < 0) {
        digits.clear();

        if (n == 0) {
            digits.push_back(0); // Handle zero case
            return;
        }

        if (n == INT64_MIN) {
            // Special handling for INT64_MIN
            digits.push_back(-(n % BASE)); // Add the least significant digit as positive
            n /= BASE;                    // Divide by BASE, which remains negative
            n = -n;                       // Negate to handle remaining digits positively
        } else if (n < 0) {
            n = -n;
        }

        while (n > 0) {
            digits.push_back(n % BASE);
            n /= BASE;
        }

        removeLeadingZeros();
    }

    explicit BigInteger(const std::string& str)
{
    // 1. Check empty string
    if (str.empty() || (str.size() == 1 && !std::isdigit(str[0])) ) {
        throw std::invalid_argument("Invalid input string: cannot be empty sign");
    }

    // 2. Disallow leading/trailing whitespace
    if (std::isspace(static_cast<unsigned char>(str.front())) ||
        std::isspace(static_cast<unsigned char>(str.back())))
    {
        throw std::invalid_argument("Invalid input string: leading/trailing whitespace not allowed");
    }

    // 3. Determine sign
    size_t start = 0;
    if (str[0] == '-') {
        is_negative = true;
        start = 1;
    } else if (str[0] == '+') {
        is_negative = false;
        start = 1;
    } else {
        is_negative = false;
    }

    // 4. Check that all remaining characters are digits
    for (size_t i = start; i < str.size(); ++i) {
        if (!std::isdigit(static_cast<unsigned char>(str[i]))) {
            throw std::invalid_argument("Invalid input string: non-digit character found");
        }
    }

    // 5. Prepare to parse chunk-by-chunk from the end
    static constexpr size_t  CHUNK_SIZE    = 9;             // 9 decimal digits fits into a 32-bit integer
    static constexpr uint64_t DECIMAL_BASE = 1000000000ULL; // 10^CHUNK_SIZE

    BigInteger result;
    BigInteger power_of_ten(1); // Will track powers of 10^CHUNK_SIZE
    size_t i = str.size();

    while (i > start) {
        // End of chunk is 'i', chunk start is either i-CHUNK_SIZE or the 'start' of the digits
        size_t chunk_end   = i;
        size_t chunk_start = (chunk_end >= CHUNK_SIZE) ? (chunk_end - CHUNK_SIZE) : start;
        std::string chunk  = str.substr(chunk_start, chunk_end - chunk_start);

        // Convert the extracted substring to a number
        uint64_t chunk_value = std::stoull(chunk);

        // Turn the chunk_value into a BigInteger, multiply by the appropriate power_of_ten, then add
        BigInteger chunk_big(chunk_value);
        chunk_big *= power_of_ten;
        result += chunk_big;

        // Increase power_of_ten by 10^CHUNK_SIZE for the next iteration
        power_of_ten *= DECIMAL_BASE;

        // Move i to the beginning of this chunk
        i = chunk_start;
    }

    // 6. Copy digits from the temporary result into this BigInteger
    digits = result.digits;
    removeLeadingZeros();
}

    // Copy constructor and assignment operator
    BigInteger(const BigInteger& other) = default;
    BigInteger& operator=(const BigInteger& rhs) = default;

    // Unary operators
    const BigInteger& operator+() const {
        return *this; // The unary plus operator just returns the number as is
    }

    BigInteger operator-() const {
        BigInteger result = *this;
        if (result != 0) {
            result.is_negative = !result.is_negative;
        }
        return result;
    }

    BigInteger& operator+=(const BigInteger& rhs) {
        if (rhs == 0) {
            return *this;
        }
        if (is_negative == rhs.is_negative) {
            // Same sign => do standard chunk-based addition
            size_t max_size = std::max(digits.size(), rhs.digits.size());
            BaseType carry = 0;
            for (size_t i = 0; i < max_size || carry; ++i) {
                if (i == digits.size()) {
                    digits.push_back(0);
                }
                // Use 64 bits to avoid overflow
                DoubleBaseType sum = static_cast<DoubleBaseType>(digits[i])
                                   + (i < rhs.digits.size() ? rhs.digits[i] : 0)
                                   + carry;
                digits[i] = static_cast<BaseType>(sum % BASE);
                carry = static_cast<BaseType>(sum / BASE);
            }
        } else {
            // Different signs => reduce to subtraction
            *this -= -rhs;
        }
        removeLeadingZeros();
        return *this;
    }

    BigInteger& operator-=(const BigInteger& rhs) {
        if (rhs == 0) {
            return *this;
        }
        if (is_negative == rhs.is_negative) {
            // Same sign => do actual subtraction
            // Compare absolute values to see which is bigger
            if (abs() >= rhs.abs()) {
                BaseType borrow = 0;
                for (size_t i = 0; i < rhs.digits.size() || borrow; ++i) {
                    if (i == digits.size()) {
                        // Extend if needed (though theoretically *this >= rhs
                        // means we should already have enough digits)
                        digits.push_back(0);
                    }
                    // 64 bits to avoid overflow on the subtraction
                    DoubleBaseType diff = static_cast<DoubleBaseType>(digits[i])
                                        - (i < rhs.digits.size() ? rhs.digits[i] : 0)
                                        - borrow;

                    // If diff < 0 in 64-bit signed sense, top bit of diff will be 1
                    // An easy check is:
                    borrow = (diff >> 63) & 1;
                    if (borrow) {
                        diff += BASE;
                    }
                    digits[i] = static_cast<BaseType>(diff);
                }
                removeLeadingZeros();
            } else {
                // If |rhs| is bigger, do: -(rhs - this)
                BigInteger temp = rhs;
                temp -= *this;
                *this = -temp;
            }
        } else {
            // Different signs => a - (-b) = a + b, etc.
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

    double sqrt() const
    {
        // 1. Negative check
        if (is_negative) {
            throw std::runtime_error("Cannot compute square root of a negative number");
        }

        // 2. Zero check
        if (*this == 0) {
            return 0.0;
        }

        // 3. Convert the BigInteger to a string using operator<<
        std::ostringstream oss;
        oss << *this;            // uses your inline operator<<(std::ostream&, const BigInteger&)
        std::string str_value = oss.str();

        // 4. Use std::stod in a try-catch to handle conversion failures
        double value;
        try {
            value = std::stod(str_value);
        }
        catch (const std::invalid_argument&) {
            throw std::runtime_error("Failed to convert BigInteger to double: invalid argument");
        }
        catch (const std::out_of_range&) {
            throw std::runtime_error("Failed to convert BigInteger to double: out of range");
        }

        // 5. Compute sqrt
        return std::sqrt(value);
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

inline std::strong_ordering operator<=>(const BigInteger& lhs, const BigInteger& rhs) {
    if (lhs.is_negative != rhs.is_negative) {
        return lhs.is_negative ? std::strong_ordering::less : std::strong_ordering::greater;
    }

    if (lhs.digits.size() != rhs.digits.size()) {
        return (lhs.digits.size() < rhs.digits.size()) == lhs.is_negative
                   ? std::strong_ordering::greater
                   : std::strong_ordering::less;
    }

    for (size_t i = lhs.digits.size(); i-- > 0;) {
        if (lhs.digits[i] != rhs.digits[i]) {
            return (lhs.digits[i] < rhs.digits[i]) == lhs.is_negative
                       ? std::strong_ordering::greater
                       : std::strong_ordering::less;
        }
    }
    return std::strong_ordering::equal;
}

inline bool operator==(const BigInteger& lhs, const BigInteger& rhs) {
    return (lhs <=> rhs) == 0; // Use the spaceship operator
}

inline bool operator!=(const BigInteger& lhs, const BigInteger& rhs) {
    return !(lhs == rhs); // Use the equality operator
}

inline std::ostream& operator<<(std::ostream& os, const BigInteger& n)
{
    if (n.digits.empty() || (n.digits.size() == 1 && n.digits[0] == 0)) {
        os << '0';
        return os;
    }

    if (n.is_negative) {
        os << '-';
    }


    const uint64_t INTERNAL_BASE = BigInteger::BASE;  // e.g. 2^16, 2^15, etc.
    static constexpr uint64_t DECIMAL_BASE = 1000000000000000000ULL; // 10^18


    std::vector<uint32_t> base10_digits;  // store decimal chunks
    BigInteger temp = n.abs();            // local copy of absolute value

    while (temp != BigInteger(0))
    {
        auto [q, r] = temp.divide_with_remainder(BigInteger((int64_t)DECIMAL_BASE));

        temp = q; // new quotient

        uint64_t remainderVal = 0;
        uint64_t factor = 1;
        for (size_t i = 0; i < r.digits.size(); ++i) {
            remainderVal += (uint64_t)r.digits[i] * factor;
            factor *= INTERNAL_BASE;
        }

        base10_digits.push_back((uint32_t)remainderVal);
    }


    auto it = base10_digits.rbegin();
    os << *it++;  // print the top chunk normally (no padding)

    while (it != base10_digits.rend()) {
        os << std::setw(9) << std::setfill('0') << *it++;
    }

    return os;
}


// ===================================================================
// ========================= BigRational =============================
// ===================================================================

// ----------------------------------------------------
// gcd helper for BigInteger
// ----------------------------------------------------
static BigInteger big_gcd(BigInteger a, BigInteger b)
{
    // We want a >= 0, b >= 0
    if (a < 0) a = -a;
    if (b < 0) b = -b;

    // Euclidean algorithm
    while (b != BigInteger(0)) {
        BigInteger r = a % b;
        a = b;
        b = r;
    }
    return a;
}

// ----------------------------------------------------
// BigRational
// ----------------------------------------------------
class BigRational
{
private:
    BigInteger numerator_;
    BigInteger denominator_; // always kept > 0 (unless numerator is 0)

    // Reduce fraction to canonical form
    void reduce()
    {
        if (numerator_ == 0) {
            // 0/anything => 0/1
            denominator_ = BigInteger(1);
            return;
        }

        BigInteger g = big_gcd(numerator_, denominator_);

        numerator_   /= g;
        denominator_ /= g;

        // Keep denominator positive
        if (denominator_ < 0) {
            denominator_ = -denominator_;
            numerator_   = -numerator_;
        }
    }

public:
    // ----------------------------------------------------
    // constructors
    // ----------------------------------------------------
    // 1) Default => 0
    BigRational()
        : numerator_(0), denominator_(1)
    { }

    // 2) from two int64_t
    BigRational(int64_t a, int64_t b)
        : numerator_(a), denominator_(b == 0 ? throw std::invalid_argument("Denominator == 0") : BigInteger(b))
    {
        reduce();
    }

    // 3) from two strings
    BigRational(const std::string& a, const std::string& b)
        : numerator_(a), denominator_((b == "0" || b.empty()) ? throw std::invalid_argument("Denominator == 0") : BigInteger(b))
    {
        reduce();
    }

    // copy constructor
    BigRational(const BigRational& other) = default;

    // copy assignment
    BigRational& operator=(const BigRational& rhs)
        = default;

    // ----------------------------------------------------
    // unary operators
    // ----------------------------------------------------
    const BigRational& operator+() const {
        return *this;
    }

    BigRational operator-() const {
        BigRational tmp(*this);
        tmp.numerator_ = -tmp.numerator_;
        // denominator_ stays positive, so sign is held in numerator_
        return tmp;
    }

    // ----------------------------------------------------
    // compound assignment
    // ----------------------------------------------------
    BigRational& operator+=(const BigRational& rhs)
    {
        // a/b + c/d = (ad + bc) / (bd)
        BigInteger a = numerator_;
        BigInteger b = denominator_;
        BigInteger c = rhs.numerator_;
        BigInteger d = rhs.denominator_;

        numerator_   = a * d + b * c;
        denominator_ = b * d;
        reduce();
        return *this;
    }

    BigRational& operator-=(const BigRational& rhs)
    {
        // a/b - c/d = (ad - bc) / (bd)
        BigInteger a = numerator_;
        BigInteger b = denominator_;
        BigInteger c = rhs.numerator_;
        BigInteger d = rhs.denominator_;

        numerator_   = a * d - b * c;
        denominator_ = b * d;
        reduce();
        return *this;
    }

    BigRational& operator*=(const BigRational& rhs)
    {
        // (a/b) * (c/d) = (ac)/(bd)
        numerator_   = numerator_   * rhs.numerator_;
        denominator_ = denominator_ * rhs.denominator_;
        reduce();
        return *this;
    }

    BigRational& operator/=(const BigRational& rhs)
    {
        // (a/b) / (c/d) = (a/b) * (d/c) = (ad)/(bc)
        if (rhs.numerator_ == BigInteger(0)) {
            throw std::invalid_argument("Division by zero in BigRational");
        }
        numerator_   = numerator_   * rhs.denominator_;
        denominator_ = denominator_ * rhs.numerator_;
        if (denominator_ == BigInteger(0)) {
            throw std::invalid_argument("Division by zero in BigRational (resulting denominator is 0)");
        }
        reduce();
        return *this;
    }

    // ----------------------------------------------------
    // sqrt
    // ----------------------------------------------------
    double sqrt() const
    {
        // if negative => error
        // if 0 => 0
        // else => double(numerator)/double(denominator), then std::sqrt
        if (numerator_ < 0) {
            throw std::runtime_error("Cannot take sqrt of negative fraction");
        }
        if (numerator_ == 0) {
            return 0.0;
        }

        // Convert numerator and denominator to double
        //   - using your BigInteger's operator<< + std::stod
        std::ostringstream ossN, ossD;
        ossN << numerator_;
        ossD << denominator_;

        double numVal = 0.0, denVal = 1.0;
        try {
            numVal = std::stod(ossN.str());
            denVal = std::stod(ossD.str());
        }
        catch (...) {
            throw std::runtime_error("Conversion to double failed in BigRational::sqrt()");
        }
        if (denVal == 0.0) {
            throw std::runtime_error("Denominator is zero in BigRational::sqrt()");
        }

        double fractionVal = numVal / denVal;
        // check fractionVal < 0 (though we handled negative fraction above)
        if (fractionVal < 0) {
            throw std::runtime_error("Fraction is negative, cannot take sqrt()");
        }
        return std::sqrt(fractionVal);
    }

    // friend declarations for comparison operators
    friend bool operator==(const BigRational& lhs, const BigRational& rhs);
    friend bool operator<(const BigRational& lhs, const BigRational& rhs);

    // stream output
    friend std::ostream& operator<<(std::ostream& os, const BigRational& r);
};

// non-member binary operators for BigRational
inline BigRational operator+(BigRational lhs, const BigRational& rhs) {
    lhs += rhs;
    return lhs;
}

inline BigRational operator-(BigRational lhs, const BigRational& rhs) {
    lhs -= rhs;
    return lhs;
}

inline BigRational operator*(BigRational lhs, const BigRational& rhs) {
    lhs *= rhs;
    return lhs;
}

inline BigRational operator/(BigRational lhs, const BigRational& rhs) {
    lhs /= rhs;
    return lhs;
}

// comparison
// we only need < and ==. The rest can be derived.
inline bool operator==(const BigRational& lhs, const BigRational& rhs)
{
    return (lhs.numerator_ == rhs.numerator_) &&
           (lhs.denominator_ == rhs.denominator_);
}

inline bool operator!=(const BigRational& lhs, const BigRational& rhs)
{
    return !(lhs == rhs);
}

inline bool operator<(const BigRational& lhs, const BigRational& rhs)
{
    // a/b < c/d => ad < bc
    // watch out for sign – but we've normalized denominator to be positive
    return (lhs.numerator_ * rhs.denominator_) < (rhs.numerator_ * lhs.denominator_);
}

inline bool operator>(const BigRational& lhs, const BigRational& rhs)
{
    return rhs < lhs;
}

inline bool operator<=(const BigRational& lhs, const BigRational& rhs)
{
    return !(rhs < lhs);
}

inline bool operator>=(const BigRational& lhs, const BigRational& rhs)
{
    return !(lhs < rhs);
}

// stream output
// ff denominator == 1 => just print numerator.
// otherwise => "numerator/denominator" with no spaces.
inline std::ostream& operator<<(std::ostream& os, const BigRational& r)
{
    if (r.denominator_ == BigInteger(1)) {
        os << r.numerator_;
    } else {
        os << r.numerator_ << '/' << r.denominator_;
    }
    return os;
}
