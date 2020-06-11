#include "Fourier.h"
#include <algorithm>
#include <execution>
#include <iostream>


Fourier::Fourier(const std::vector<complex>& funcPoints, const int& oscilators)
{
	std::vector<int> nums = { 0 };
	for (int n = 1; n <= oscilators; n++)
	{
		nums.push_back(n);
		nums.push_back(-n);
	}

	float dt = 1.f / funcPoints.size();

	std::mutex mutex;

	std::for_each(std::execution::par_unseq, nums.begin(), nums.end(),
	[&](const int& n)
	{
		complex integ = 0.f;
		int i = 0;
		for (float t = 0.f; t <= 1; t += dt)
		{
			integ += std::exp(-2.f * (float)PI * 1_i * (float)n * t) * funcPoints[i] * dt;

			i++;
		}

		mutex.lock();
		m_coeffs.push_back({ n, integ });
		mutex.unlock();
	});
}

Fourier::Fourier(const std::function<complex(const float& t)>& func, const int& oscilators, const float& dt)
{
	std::vector<int> nums = { 0 };
	for (int n = 1; n <= oscilators; n++)
	{
		nums.push_back(n);
		nums.push_back(-n);
	}
	

	std::mutex mutex;

	std::for_each(std::execution::par, nums.begin(), nums.end(),
	[&](const int& n)
	{
		complex integ = 0.f + 0_i;
		for (float t = 0.f; t <= 1; t += dt)
		{
			integ += std::exp(-2.f * (float)PI * 1_i * (float)n * t) * func(t) * dt;
		}

		std::lock_guard<std::mutex> lock(mutex);
		m_coeffs.push_back({ n, integ });
	});

	std::sort(m_coeffs.begin(), m_coeffs.end(), [&](const std::pair<int, complex>& l, const std::pair<int, complex>& r)->bool
	{
		if (std::abs(l.first) != std::abs(r.first))
		{
			return std::abs(l.first) < std::abs(r.first);
		}
		else
		{
			return l.first > r.first;
		}
	});
}

complex Fourier::get(const float& x)
{
	complex y = 0;
	for (const auto& [n, c] : m_coeffs)
	{
		y += c * std::exp(n * 2.f * (float)PI * 1_i * x);
	}
	return y;
}

const std::vector<std::pair<int, complex>>& Fourier::GetCoeffs()
{
	return m_coeffs;
}
