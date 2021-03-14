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
#include <comdef.h>

#include <dxgi1_6.h>
#include <d3d12.h>
#include <d3dcompiler.h>

#include <DirectXCollision.h>
#include <DirectXColors.h>
#include <DirectXMath.h>

#include "../d3dx12.h"

#include <cassert>
#include <chrono>
#include <cstdint>

class Exception
{
public:
	Exception() = default;
	Exception(
		HRESULT hr,
		std::string const& func,
		std::string const& file,
		int line)
		:
		result{ hr },
		func{ func },
		file{ file },
		line{ line }
	{}

	std::wstring toWString() const
	{
		_com_error error(result);

		std::wstring w_func(func.begin(), func.end());
		std::wstring w_file(file.begin(), file.end());

		return w_func + L" failed in " + w_file + L"; line " +
			std::to_wstring(line) + L"; error: " + error.ErrorMessage() + L"\n";
	}

	HRESULT result = S_OK;
	std::string func;
	std::string file;
	int line = -1;
};

#define THROW_IF_FAILED(fn)                               \
	{                                                     \
		HRESULT hr = (fn);                                \
		if (FAILED(hr))                                   \
			throw Exception(hr, #fn, __FILE__, __LINE__); \
	}
