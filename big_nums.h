#pragma once
#include <vector>
#include <string>
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <cmath>

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
        if (is_negative == rhs.is_negative) {
            // compare absolute values to see which is bigger
            if (abs() >= rhs.abs()) {
                BaseType borrow = 0;
                for (size_t i = 0; i < rhs.digits.size() || borrow; ++i) {
                    if (i == digits.size()) {
                        digits.push_back(0);
                    }
                    DoubleBaseType diff = static_cast<DoubleBaseType>(digits[i])
                                        - (i < rhs.digits.size() ? rhs.digits[i] : 0)
                                        - borrow;

                    // if diff < 0 in 64-bit signed sense, top bit of diff will be 1
                    borrow = (diff >> 63) & 1;
                    if (borrow) {
                        diff += BASE;
                    }
                    digits[i] = static_cast<BaseType>(diff);
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

    BigInteger& operator*=(const BigInteger& rhs)
{
    // 1) if either operand is zero, result = 0
    if ((digits.size() == 1 && digits[0] == 0) ||
        (rhs.digits.size() == 1 && rhs.digits[0] == 0))
    {
        digits.resize(1);
        digits[0] = 0;
        is_negative = false;
        return *this;
    }

    // 2) determine final sign by XOR of signs, then take absolute values
    bool newSign = (is_negative != rhs.is_negative);
    BigInteger A = this->abs();
    BigInteger B = rhs.abs();


    // (A) Multiply naive (O(n^2)) for smaller inputs
    auto multiplyNaive = [&](const BigInteger& x, const BigInteger& y) -> BigInteger
    {
        BigInteger result;
        result.digits.resize(x.digits.size() + y.digits.size(), 0);

        for (size_t i = 0; i < x.digits.size(); ++i) {
            DoubleBaseType carry = 0;
            for (size_t j = 0; j < y.digits.size() || carry; ++j) {
                DoubleBaseType prod =
                    result.digits[i + j]
                    + (DoubleBaseType)x.digits[i] * (j < y.digits.size() ? y.digits[j] : 0)
                    + carry;

                result.digits[i + j] = static_cast<BaseType>(prod % BASE);
                carry = static_cast<BaseType>(prod / BASE);
            }
        }
        result.removeLeadingZeros();
        return result;
    };

    auto shiftLeft = [&](const BigInteger& val, size_t digitCount) -> BigInteger
    {
        if ((val.digits.size() == 1 && val.digits[0] == 0) || digitCount == 0) {
            return val; // no shift needed if zero or shift=0
        }
        BigInteger out = val;
        out.digits.insert(out.digits.begin(), digitCount, 0);
        out.removeLeadingZeros();
        return out;
    };

    static const size_t KARATSUBA_THRESHOLD = 64; // tune as needed

    std::function<BigInteger(const BigInteger&, const BigInteger&)> karatsuba =
    [&](const BigInteger& x, const BigInteger& y) -> BigInteger
    {
        if ((x.digits.size() == 1 && x.digits[0] == 0) ||
            (y.digits.size() == 1 && y.digits[0] == 0))
        {
            return BigInteger(0);
        }

        size_t n = std::max(x.digits.size(), y.digits.size());
        if (n < KARATSUBA_THRESHOLD) {
            return multiplyNaive(x, y);
        }

        BigInteger a = x;
        BigInteger b = y;
        a.digits.resize(n, 0);
        b.digits.resize(n, 0);

        size_t m = n / 2; // "half" in digits
        // a_low = a[0..m-1], a_high = a[m..n-1]
        BigInteger a_low, a_high, b_low, b_high;
        a_low.digits.assign(a.digits.begin(), a.digits.begin() + m);
        a_high.digits.assign(a.digits.begin() + m, a.digits.end());
        b_low.digits.assign(b.digits.begin(), b.digits.begin() + m);
        b_high.digits.assign(b.digits.begin() + m, b.digits.end());

        a_low.removeLeadingZeros();
        a_high.removeLeadingZeros();
        b_low.removeLeadingZeros();
        b_high.removeLeadingZeros();

        // p0 = a_low * b_low
        // p2 = a_high * b_high
        // p1 = (a_low+a_high)*(b_low+b_high) - p0 - p2
        BigInteger p0 = karatsuba(a_low, b_low);
        BigInteger p2 = karatsuba(a_high, b_high);

        BigInteger sumA = a_low;  sumA += a_high;
        BigInteger sumB = b_low;  sumB += b_high;

        BigInteger p1 = karatsuba(sumA, sumB);
        p1 -= p0;
        p1 -= p2;

        // Combine
        // => p0 + (p1 << m) + (p2 << (2*m))
        BigInteger res = shiftLeft(p2, 2*m);
        BigInteger mid = shiftLeft(p1, m);
        res += mid;
        res += p0;
        res.removeLeadingZeros();
        return res;
    };

    // 3) do Karatsuba multiply with abs(A), abs(B)
    BigInteger product = karatsuba(A, B);

    // 4) reapply sign and remove leading zeros
    product.is_negative = newSign;
    product.removeLeadingZeros();

    // 5) store in *this
    *this = product;
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

inline std::ostream& operator<<(std::ostream& os, const BigInteger& n) {

    if (n.digits.empty() || (n.digits.size() == 1 && n.digits[0] == 0)) {
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


// ---------------------------------------------------------------------------
// BIG RATIONAL
// ---------------------------------------------------------------------------

// Forward-declare a gcd function for BigInteger
// (We’ll define it after the BigRational class).
static BigInteger gcd(const BigInteger& a, const BigInteger& b);

class BigRational
{
private:
    // The fraction is stored as numerator / denominator.
    // We always keep denominator > 0 (unless numerator == 0, then denom=1).
    // The sign is carried in the numerator's is_negative flag.
    BigInteger numerator;
    BigInteger denominator;

    // Helper function: fix sign so that denominator is always positive
    void fixSign() {
        // If denominator < 0, multiply both by -1
        // BigInteger's sign is stored in `is_negative` of the object,
        // so we can check denominators in a consistent way via comparisons.
        if (denominator < BigInteger(0)) {
            numerator = -numerator;
            denominator = -denominator;
        }
        // If numerator = 0, force denominator = 1 and remove sign from numerator
        if (numerator == BigInteger(0)) {
            denominator = BigInteger(1);
        }
    }

    // Reduce fraction by gcd
    void reduce() {
        // gcd(0, something) is 'something', but we also handle sign in fixSign
        BigInteger g = gcd(numerator < BigInteger(0) ? -numerator : numerator,
                           denominator);
        if (g != BigInteger(0)) {
            numerator /= g;
            denominator /= g;
        }
        fixSign();
    }

public:
    // -----------------------------------------------------------------------
    // Constructors
    // -----------------------------------------------------------------------

    // Default constructor => 0/1
    BigRational()
        : numerator(0), denominator(1)
    {
        // Already normalized as 0/1
    }

    // Construct from two 64-bit integers => BigRational(a, b) = a/b
    // (throws if b == 0)
    BigRational(int64_t a, int64_t b)
        : numerator(a), denominator(b)
    {
        if (b == 0) {
            throw std::invalid_argument("BigRational construction with denominator=0");
        }
        reduce();
    }

    // Construct from two strings => BigRational(a, b) = a/b
    // (throws if b == 0 or if a/b are not valid BigInteger strings)
    BigRational(const std::string& a, const std::string& b)
        : numerator(a), denominator(b)
    {
        if (denominator == BigInteger(0)) {
            throw std::invalid_argument("BigRational construction with denominator=0");
        }
        reduce();
    }

    // Copy constructor and assignment (the defaults are fine here)
    BigRational(const BigRational&) = default;
    BigRational& operator=(const BigRational&) = default;

    // -----------------------------------------------------------------------
    // Unary operators
    // -----------------------------------------------------------------------

    // +r just returns r
    const BigRational& operator+() const {
        return *this;
    }

    // -r flips the sign of the numerator
    BigRational operator-() const {
        BigRational result = *this;
        result.numerator = -(result.numerator);
        return result;
    }

    // -----------------------------------------------------------------------
    // Arithmetic compound operators
    // -----------------------------------------------------------------------
    // a/b += c/d => (a/b) + (c/d) = (ad + bc) / bd
    BigRational& operator+=(const BigRational& rhs) {
        // Common denominator
        BigInteger new_num = numerator * rhs.denominator + rhs.numerator * denominator;
        BigInteger new_den = denominator * rhs.denominator;
        numerator = new_num;
        denominator = new_den;
        reduce();
        return *this;
    }

    // a/b -= c/d => (a/b) - (c/d) = (a d - b c) / (b d)
    BigRational& operator-=(const BigRational& rhs) {
        BigInteger new_num = numerator * rhs.denominator - rhs.numerator * denominator;
        BigInteger new_den = denominator * rhs.denominator;
        numerator = new_num;
        denominator = new_den;
        reduce();
        return *this;
    }

    // a/b *= c/d => (a c) / (b d)
    BigRational& operator*=(const BigRational& rhs) {
        numerator *= rhs.numerator;
        denominator *= rhs.denominator;
        reduce();
        return *this;
    }

    // a/b /= c/d => (a/b) / (c/d) = (a d) / (b c)
    // (throws if rhs.numerator == 0)
    BigRational& operator/=(const BigRational& rhs) {
        if (rhs.numerator == BigInteger(0)) {
            throw std::invalid_argument("Division by zero in BigRational");
        }
        numerator *= rhs.denominator;
        denominator *= rhs.numerator;
        reduce();
        return *this;
    }

    // -----------------------------------------------------------------------
    // Arithmetic operators
    // -----------------------------------------------------------------------
    friend inline BigRational operator+(BigRational lhs, const BigRational& rhs) {
        lhs += rhs;
        return lhs;
    }
    friend inline BigRational operator-(BigRational lhs, const BigRational& rhs) {
        lhs -= rhs;
        return lhs;
    }
    friend inline BigRational operator*(BigRational lhs, const BigRational& rhs) {
        lhs *= rhs;
        return lhs;
    }
    friend inline BigRational operator/(BigRational lhs, const BigRational& rhs) {
        lhs /= rhs;
        return lhs;
    }

    // -----------------------------------------------------------------------
    // Comparison operators
    // For <, >, <=, >=, ==, !=
    //
    // The simplest approach is to compare cross-products:
    // a/b < c/d <=> a*d < c*b, provided b,d > 0 (which we maintain).
    // -----------------------------------------------------------------------
    friend inline bool operator==(const BigRational& lhs, const BigRational& rhs) {
        return (lhs.numerator == rhs.numerator) && (lhs.denominator == rhs.denominator);
    }

    friend inline bool operator!=(const BigRational& lhs, const BigRational& rhs) {
        return !(lhs == rhs);
    }

    friend inline bool operator<(const BigRational& lhs, const BigRational& rhs) {
        return (lhs.numerator * rhs.denominator) < (rhs.numerator * lhs.denominator);
    }

    friend inline bool operator>(const BigRational& lhs, const BigRational& rhs) {
        return rhs < lhs;
    }

    friend inline bool operator<=(const BigRational& lhs, const BigRational& rhs) {
        return !(rhs < lhs);
    }

    friend inline bool operator>=(const BigRational& lhs, const BigRational& rhs) {
        return !(lhs < rhs);
    }

    // -----------------------------------------------------------------------
    // sqrt()
    // Returns a double approximation of the rational number's square root.
    // If the value is negative, throw exception. If the number is huge,
    // this might lose precision or throw an out_of_range on some platforms.
    // -----------------------------------------------------------------------
    double sqrt() const {
        // If numerator < 0 => negative => throw
        if (numerator < BigInteger(0)) {
            throw std::runtime_error("Cannot take sqrt of negative BigRational");
        }
        // 0 => return 0.0
        if (numerator == BigInteger(0)) {
            return 0.0;
        }
        // Convert numerator and denominator to double and compute sqrt
        double num_d = this->numerator.sqrt();   // already checks out_of_range
        double den_d = this->denominator.sqrt(); // likewise
        return num_d / den_d;
    }

    // -----------------------------------------------------------------------
    // Output: normalized form.
    // - If denom == 1, we print just the numerator.
    // - Otherwise, we print "num/den".
    // -----------------------------------------------------------------------
    friend inline std::ostream& operator<<(std::ostream& os, const BigRational& x) {
        // If denominator == 1 or numerator == 0 => just print numerator
        if (x.denominator == BigInteger(1) || x.numerator == BigInteger(0)) {
            os << x.numerator;
        } else {
            os << x.numerator << "/" << x.denominator;
        }
        return os;
    }
};

// ---------------------------------------------------------------------------
// gcd implementation for BigInteger, using Euclid’s algorithm
// Note: We treat gcd(0,0) as 0, which generally you won't hit
// unless you do something degenerate.
// ---------------------------------------------------------------------------
static BigInteger gcd(const BigInteger& a, const BigInteger& b)
{
    // We'll implement the iterative version
    // (you can use recursion if you prefer).
    BigInteger x = (a < BigInteger(0)) ? -a : a;
    BigInteger y = (b < BigInteger(0)) ? -b : b;
    while (y != BigInteger(0)) {
        BigInteger r = x % y;
        x = y;
        y = r;
    }
    return x; // x is now the gcd
}

