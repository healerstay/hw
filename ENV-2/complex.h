#ifndef COMPLEX_H
#define COMPLEX_H

#include <iostream>
#include <cmath>

class Complex {
private:
	double r;
	double i;
public:
	Complex() : r(0.0), i(0.0) {}
	Complex(const double real, const double im) : r(real), i(im) {}

	double real() const;
	double imag() const;

	Complex operator+(const Complex& c) const;
	Complex operator-(const Complex& c) const;
	Complex operator*(double num) const;

	double abs() const;

	friend std::ostream& operator<<(std::ostream& os, const Complex& c);
};

#endif //COMPLEX_H
