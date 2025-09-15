#include <iostream>
#include <array>
#include "complex.h"
#include "sort.h"

int main() {
    std::array<Complex, 5> arr {{
        {3.0, 4.0},
        {1.0, 1.0},
        {-2.0, 0.0},
        {0.0, -3.0},
        {0.5, 0.2}
    }};

    std::cout << "Before sort:  ";
    for (const auto& c : arr) {
        std::cout << c << "   ";
    }

    sort(arr);

    std::cout << "\nAfter sort:  ";
    for (auto c : arr) {
        std::cout << c << "   ";
    }
    std::cout << std::endl;

    Complex a{1.0, 2.0}, b{2.0, -1.0};

    std::cout << a << " + " << b << " = " << (a + b) << '\n';
    std::cout << a << " - " << b << " = " << (a - b) << '\n';
    std::cout << a << " * 2.5 = " << (a * 2.5) << '\n';
    std::cout << "|" << a << "| = " << a.abs() << '\n';

    return 0;
}

