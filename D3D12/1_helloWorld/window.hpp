#pragma once

#include "config.hpp"

#include <string>

class Window
{
public:
	Window(
		HINSTANCE instance,
		wchar_t const* class_name,
		wchar_t const* title,
		bool vsync = true,
		bool fullscreen = false,
		int w = 1366,
		int h = 768)
		:
		title{ title },
		vsync{ vsync },
		fullscreen{ fullscreen },
		width{ w },
		height{ h }
	{
		rect = { 0, 0, width, height };
		AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

		handle = CreateWindow(
			class_name,
			title,
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			width,
			height,
			nullptr,
			nullptr,
			instance,
			nullptr);

		assert(handle && "Failed to create window");
	}

	void show() const
	{
		ShowWindow(handle, SW_SHOW);
	}

	HWND getHandle() const
	{
		return handle;
	}

	int getWidth() const
	{
		return width;
	}

	int getHeight() const
	{
		return height;
	}

private:
	HWND handle;
	RECT rect;

	std::wstring title;

	bool vsync;
	bool fullscreen;

	int width;
	int height;
};
