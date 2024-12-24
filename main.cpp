#include <iostream>
#include <stdexcept>
#include <string>
#include "big_nums.h"


int main() {
    try {
        // Test BigInteger
        std::cout << "Testing BigInteger:\n";
        BigInteger a = 1234567890123;
        BigInteger b = -987654321098;
        BigInteger c("123456789012345678901234567890");
        BigInteger d = a + b;
        BigInteger e = c - a;
        BigInteger f = a * b;
        BigInteger g = c / a;
        BigInteger h = c % a;

        std::cout << "a = " << a << "\n";
        std::cout << "b = " << b << "\n";
        std::cout << "c = " << c << "\n";
        std::cout << "d = a + b = " << d << "\n";
        std::cout << "e = c - a = " << e << "\n";
        std::cout << "f = a * b = " << f << "\n";
        std::cout << "g = c / a = " << g << "\n";
        std::cout << "h = c % a = " << h << "\n";

        // Test BigInteger sqrt
        std::cout << "sqrt(c) = " << c.sqrt() << "\n";

        // Test BigRational
        std::cout << "\nTesting BigRational:\n";
        BigRational r1(1, 2);
        BigRational r2(3, 4);
        BigRational r3("1234567890123456789", "9876543210987654321");
        BigRational r4 = r1 + r2;
        BigRational r5 = r2 - r1;
        BigRational r6 = r1 * r3;
        BigRational r7 = r3 / r2;

        std::cout << "r1 = " << r1 << "\n";
        std::cout << "r2 = " << r2 << "\n";
        std::cout << "r3 = " << r3 << "\n";
        std::cout << "r4 = r1 + r2 = " << r4 << "\n";
        std::cout << "r5 = r2 - r1 = " << r5 << "\n";
        std::cout << "r6 = r1 * r3 = " << r6 << "\n";
        std::cout << "r7 = r3 / r2 = " << r7 << "\n";

        // Test BigRational sqrt
        std::cout << "sqrt(r3) = " << r3.sqrt() << "\n";

        // Edge cases
        std::cout << "\nEdge cases:\n";
        BigInteger zero(0);
        BigInteger negative("-123456789");
        std::cout << "zero = " << zero << "\n";
        std::cout << "negative = " << negative << "\n";

        BigRational edge1(0, 1);
        BigRational edge2(123456, -1);
        std::cout << "edge1 = " << edge1 << "\n";
        std::cout << "edge2 = " << edge2 << "\n";

        // Division by zero test
        std::cout << "Attempting division by zero:\n";
        BigRational invalid(1, 0); // Should throw an exception
        std::cout << invalid << "\n"; // This line won't execute

    } catch (const std::exception& ex) {
        std::cerr << "Exception: " << ex.what() << "\n";
    }

    return 0;
}
