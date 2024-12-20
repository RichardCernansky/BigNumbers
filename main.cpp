#include <iostream>
#include "big_nums.h"

int main() {
    // Test BigInteger
    BigInteger a("12345678901234567890");
    BigInteger b(987654321);
    BigInteger c = a + b;

    std::cout << "BigInteger Addition: " << a << " + " << b << " = " << c << std::endl;

    BigInteger d = a * b;
    std::cout << "BigInteger Multiplication: " << a << " * " << b << " = " << d << std::endl;

    // Test BigRational
    BigRational r1("123", "456");
    BigRational r2("789", "123");
    BigRational r3 = r1 + r2;

    std::cout << "BigRational Addition: " << r1 << " + " << r2 << " = " << r3 << std::endl;

    BigRational r4 = r1 / r2;
    std::cout << "BigRational Division: " << r1 << " / " << r2 << " = " << r4 << std::endl;

    return 0;
}