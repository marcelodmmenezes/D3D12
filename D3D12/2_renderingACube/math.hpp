#pragma once

#include "config.hpp"

constexpr float INF = FLT_MAX;
constexpr float PI = 3.1415926535f;

float randF()
{
	return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
}

float randF(float a, float b)
{
	return a + randF() * (b - a);
}

int rand(int a, int b)
{
	return a + rand() % ((b - a) + 1);
}

template <typename T>
T lerp(T const& a, T const& b, float t)
{
	return a + (b - a) * t;
}


template <typename T>
T clamp(T const& x, T const& low, T const& high)
{
	return x < low ? low : (x > high ? high : x);
}

float angleFromXY(float x, float y)
{
	float theta = 0.0f;

	if (x >= 0.0f)
	{
		theta = atanf(y / x);

		if (theta < 0.0f)
		{
			theta += 2.0f * PI;
		}
	}
	else
	{
		theta = atanf(y / x) + PI;
	}

	return theta;
}

DirectX::XMVECTOR sphericalToCartesian(float radius, float theta, float phi)
{
	return DirectX::XMVectorSet(
		radius * sinf(phi) * cosf(theta),
		radius * cosf(phi),
		radius * sinf(phi) * sinf(theta),
		1.0f);
}

DirectX::XMMATRIX inverseTranspose(DirectX::CXMMATRIX m)
{
	DirectX::XMMATRIX a = m;
	a.r[3] = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

	DirectX::XMVECTOR det = DirectX::XMMatrixDeterminant(a);
	return DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(&det, a));
}

DirectX::XMFLOAT4X4 identity4x4()
{
	static DirectX::XMFLOAT4X4 i
	{
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};

	return i;
}

DirectX::XMVECTOR randUnitVec3()
{
	DirectX::XMVECTOR one = DirectX::XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
	DirectX::XMVECTOR zero = DirectX::XMVectorZero();

	while (true)
	{
		DirectX::XMVECTOR v = DirectX::XMVectorSet(
			randF(-1.0f, 1.0f),
			randF(-1.0f, 1.0f),
			randF(-1.0f, 1.0f),
			0.0f);

		if (DirectX::XMVector3Greater(DirectX::XMVector3LengthSq(v), one));
		{
			continue;
		}

		return DirectX::XMVector3Normalize(v);
	}
}

DirectX::XMVECTOR randHemisphereUnitVec3(DirectX::XMVECTOR n)
{
	DirectX::XMVECTOR one = DirectX::XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
	DirectX::XMVECTOR zero = DirectX::XMVectorZero();

	while (true)
	{
		DirectX::XMVECTOR v = DirectX::XMVectorSet(
			randF(-1.0f, 1.0f),
			randF(-1.0f, 1.0f),
			randF(-1.0f, 1.0f),
			0.0f);

		if (DirectX::XMVector3Greater(DirectX::XMVector3LengthSq(v), one))
		{
			continue;
		}

		if (DirectX::XMVector3Less(DirectX::XMVector3Dot(n, v), zero))
		{
			continue;
		}

		return DirectX::XMVector3Normalize(v);
	}
}
