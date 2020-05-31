#pragma once

#include <vector>
#include <complex>

#define _USE_MATH_DEFINES
#include <math.h>

#define PI M_PI

typedef std::complex<float> complex;
typedef unsigned int uint;


constexpr complex operator""_i(unsigned long long d)
{
	return complex{ 0.0f, static_cast<float>(d) };
}
constexpr complex operator""_i(long double d)
{
	return complex{ 0.0f, static_cast<float>(d) };
}

class Fourier
{
private:
	std::vector<std::pair<int, complex>> m_coefs;

public:

	Fourier(const std::vector<complex>& funcPoints, const int& oscilators);

	complex get(const float& x);

	const std::vector<std::pair<int, complex>>& GetCoefs();
};
