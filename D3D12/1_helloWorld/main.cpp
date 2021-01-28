#include "window.hpp"
#include "timer.hpp"
#include "d3d12.hpp"

LRESULT CALLBACK windowCallback(
	HWND handle,
	UINT msg,
	WPARAM w_param,
	LPARAM l_param)
{
	switch (msg) {
	case WM_PAINT:
		// TODO
		return 0;

	case WM_SYSKEYDOWN:
	case WM_KEYDOWN:
		// TODO
		return 0;

	case WM_SYSCHAR:
		return 0;

	case WM_SIZE:
		// TODO
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	default:
		return DefWindowProc(handle, msg, w_param, l_param);
	}
}

void registerWindowClass(HINSTANCE instance, wchar_t const* class_name)
{
	WNDCLASS wc = {};

	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = &windowCallback;
	wc.hInstance = instance;
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszClassName = class_name;

	static ATOM atom = RegisterClass(&wc);
	assert(atom > 0);
}

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int)
{
	SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
	registerWindowClass(instance, L"WINDOW_CLASS_NAME");

#if defined(_DEBUG)
	D3D12::enableDebugLayer();
#endif

	Window window(instance, L"WINDOW_CLASS_NAME", L"D3D12");
	RelativeTimer timer;
	D3D12 d3d12(&window);
	MSG msg = {};

	window.show();
	timer.reset();

	std::wstring output;

	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			timer.tick();

			output = std::to_wstring(timer.deltaTime());
			output += L" - ";
			output += std::to_wstring(timer.totalTime());
			output += L"\n";

			OutputDebugString(output.c_str());
		}
	}
}
