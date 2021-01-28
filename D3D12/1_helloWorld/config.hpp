#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#if defined(max)
#undef max
#endif

#if defined(min)
#undef min
#endif

#include <wrl.h>

#include <dxgi1_6.h>
#include <d3d12.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

#include "../d3dx12.h"

#include <cassert>
#include <chrono>
#include <cstdint>
#include <exception>

inline void throwIfFailed(HRESULT hr)
{
	if (FAILED(hr))
	{
		throw std::exception();
	}
}
