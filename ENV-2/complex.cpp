#include "complex.h"

double Complex::real() const { return r; }
double Complex::imag() const { return i; }

Complex Complex::operator+(const Complex& c) const {
	return Complex(r + c.r, i + c.i);
}

Complex Complex::operator-(const Complex& c) const {
	return Complex(r - c.r, i - c.i);
}

Complex Complex::operator*(double num) const {
	return Complex(r * num, i * num);
}

double Complex::abs() const {
	return std::sqrt(r * r + i * i);
}

std::ostream& operator<<(std::ostream& os, const Complex& c) {
	if (c.r != 0) {
	       	os << c.r;
	}
	if (c.i > 0) {
		os << "+" << c.i << "i";
	}
	if (c.i < 0) {
		os << c.i << "i";
	}
	return os;
}

