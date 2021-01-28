#include "window.hpp"
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
		break;

	case WM_SYSKEYDOWN:
	case WM_KEYDOWN:
		// TODO
		break;

	case WM_SYSCHAR:
		break;

	case WM_SIZE:
		// TODO
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

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
	D3D12 d3d12(&window);

	window.show();

	MSG msg = {};

	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
}
