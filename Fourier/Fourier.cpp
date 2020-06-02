#include "Fourier.h"
#include <algorithm>
#include <execution>

Fourier::Fourier(const std::vector<complex>& funcPoints, const int& oscilators)
{
	std::vector<int> nums = { 0 };
	for (int n = 1; n <= oscilators; n++)
	{
		nums.push_back(n);
		nums.push_back(-n);
	}

	float dt = 1.f / funcPoints.size();

	for (const int& n : nums)
	{
		complex integ = 0.f;
		int i = 0;
		for (float t = 0.f; t <= 1; t += dt)
		{
			integ += std::exp(-2.f * (float)PI * 1_i * (float)n * t) * funcPoints[i] * dt;

			i++;
		}
		m_coefs.push_back({ n, integ });

	}
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

	std::for_each(std::execution::par_unseq, nums.begin(), nums.end(),
		[&](const int& n)
		{
			complex integ = 0.f;
			int i = 0;
			for (float t = 0.f; t <= 1; t += dt)
			{
				integ += std::exp(-2.f * (float)PI * 1_i * (float)n * t) * func(t) * dt;

				i++;
			}

			mutex.lock();
			m_coefs.push_back({ n, integ });
			mutex.unlock();
		});
}

complex Fourier::get(const float& x)
{
	complex y = 0;
	for (const auto& [n, c] : m_coefs)
	{
		y += c * std::exp(n * 2.f * (float)PI * 1_i * x);
	}
	return y;
}

const std::vector<std::pair<int, complex>>& Fourier::GetCoefs()
{
	return m_coefs;
}
