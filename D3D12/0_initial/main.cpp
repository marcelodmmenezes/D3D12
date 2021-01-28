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

#include <algorithm>
#include <cassert>
#include <chrono>
#include <exception>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

using Microsoft::WRL::ComPtr;
using namespace DirectX;

inline void ThrowIfFailed(HRESULT hr) {
	if (FAILED(hr))
		throw std::exception();
}

LRESULT CALLBACK windowProc(HWND, UINT, WPARAM, LPARAM);

struct Window {
	HWND h_wnd;
	RECT rect;

	int width = 1024;
	int height = 768;

	bool vsync = true;
	bool fullscreen = false;
} window;

constexpr int N_FRAMEBUFFERS = 2;

struct D3D12 {
	bool initialized = false;

	ComPtr<IDXGISwapChain4> swap_chain;
	ComPtr<ID3D12Device2> device;
	ComPtr<ID3D12CommandQueue> command_queue;
	ComPtr<ID3D12Resource> framebuffers[N_FRAMEBUFFERS];
	ComPtr<ID3D12GraphicsCommandList> command_list;
	ComPtr<ID3D12CommandAllocator> command_allocators[N_FRAMEBUFFERS];
	ComPtr<ID3D12DescriptorHeap> rtv_descriptor_heap;
	UINT rtv_descriptor_size;
	UINT current_framebuffer_id;

	ComPtr<ID3D12Fence> fence;
	uint64_t fence_value = 0;
	uint64_t frame_fence_values[N_FRAMEBUFFERS] = {};
	HANDLE fence_event;
} d3d12;

void enableDebugLayer() {
	ComPtr<ID3D12Debug> debug;
	ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debug)));
	debug->EnableDebugLayer();
}

void registerWindowClass(HINSTANCE h_instance, wchar_t const* name) {
	WNDCLASSEX wc = {};

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = &windowProc;
	wc.hInstance = h_instance;
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszClassName = name;

	static ATOM atom = RegisterClassEx(&wc);
	assert(atom > 0);
}

HWND createWindow(
	wchar_t const* class_name,
	HINSTANCE h_instance,
	wchar_t const* title,
	int width,
	int height) {

	RECT rect = { 0, 0, width, height };
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

	int w = rect.right - rect.left;
	int h = rect.bottom - rect.top;

	int x = std::max<int>(0, (width - w) / 2);
	int y = std::max<int>(0, (height - h) / 2);

	HWND h_wnd = CreateWindowEx(
		0,
		class_name,
		title,
		WS_OVERLAPPEDWINDOW,
		x,
		y,
		w,
		h,
		nullptr,
		nullptr,
		h_instance,
		nullptr);

	assert(h_wnd && "Failed to create window");

	return h_wnd;
}

ComPtr<IDXGIAdapter4> getAdapter() {
	ComPtr<IDXGIFactory4> dxgi_factory;
	UINT create_factory_flags = 0;

#if defined (_DEBUG)
	create_factory_flags = DXGI_CREATE_FACTORY_DEBUG;
#endif

	ThrowIfFailed(CreateDXGIFactory2(create_factory_flags, IID_PPV_ARGS(&dxgi_factory)));

	ComPtr<IDXGIAdapter1> dxgi_adapter_1;
	ComPtr<IDXGIAdapter4> dxgi_adapter_4;

	SIZE_T max_dedicated_video_memory = 0;

	for (UINT i = 0; dxgi_factory->EnumAdapters1(i, &dxgi_adapter_1) != DXGI_ERROR_NOT_FOUND; ++i) {
		DXGI_ADAPTER_DESC1 dxgi_adapter_desc_1;
		dxgi_adapter_1->GetDesc1(&dxgi_adapter_desc_1);

		if ((dxgi_adapter_desc_1.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0 &&
			SUCCEEDED(D3D12CreateDevice(dxgi_adapter_1.Get(),
				D3D_FEATURE_LEVEL_12_1, __uuidof(ID3D12Device), nullptr)) &&
			dxgi_adapter_desc_1.DedicatedVideoMemory > max_dedicated_video_memory) {
			max_dedicated_video_memory = dxgi_adapter_desc_1.DedicatedVideoMemory;
			ThrowIfFailed(dxgi_adapter_1.As(&dxgi_adapter_4));
		}
	}

	return dxgi_adapter_4;
}

ComPtr<ID3D12Device2> createDevice(ComPtr<IDXGIAdapter4> adapter) {
	ComPtr<ID3D12Device2> d3d12_device_2;

	ThrowIfFailed(D3D12CreateDevice(adapter.Get(),
		D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&d3d12_device_2)));

#if defined (_DEBUG)
	ComPtr<ID3D12InfoQueue> info_queue;

	if (SUCCEEDED(d3d12_device_2.As(&info_queue))) {
		info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
		info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
		info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);
	}
#endif

	return d3d12_device_2;
}

ComPtr<ID3D12CommandQueue> createCommandQueue(
	ComPtr<ID3D12Device2> device,
	D3D12_COMMAND_LIST_TYPE type) {

	ComPtr<ID3D12CommandQueue> d3d12_command_queue;

	D3D12_COMMAND_QUEUE_DESC desc = {};

	desc.Type = type;
	desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	desc.NodeMask = 0;

	ThrowIfFailed(device->CreateCommandQueue(&desc, IID_PPV_ARGS(&d3d12_command_queue)));

	return d3d12_command_queue;
}

ComPtr<IDXGISwapChain4> createSwapChain(
	HWND h_wnd,
	ComPtr<ID3D12CommandQueue> command_queue,
	uint32_t width,
	uint32_t height,
	uint32_t buffer_count) {

	ComPtr<IDXGISwapChain4> swap_chain_4;
	ComPtr<IDXGIFactory4> dxgi_factory_4;

	UINT create_factory_flags = 0;

#if defined (_DEBUG)
	create_factory_flags = DXGI_CREATE_FACTORY_DEBUG;
#endif

	ThrowIfFailed(CreateDXGIFactory2(create_factory_flags, IID_PPV_ARGS(&dxgi_factory_4)));

	DXGI_SWAP_CHAIN_DESC1 scd = {};

	scd.Width = width;
	scd.Height = height;
	scd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scd.Stereo = FALSE;
	scd.SampleDesc = { 1, 0 };
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scd.BufferCount = buffer_count;
	scd.Scaling = DXGI_SCALING_STRETCH;
	scd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	scd.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	scd.Flags = 0;

	ComPtr<IDXGISwapChain1> swap_chain_1;

	ThrowIfFailed(dxgi_factory_4->CreateSwapChainForHwnd(
		command_queue.Get(),
		h_wnd,
		&scd,
		nullptr,
		nullptr,
		&swap_chain_1));

	ThrowIfFailed(dxgi_factory_4->MakeWindowAssociation(h_wnd, DXGI_MWA_NO_ALT_ENTER));
	ThrowIfFailed(swap_chain_1.As(&swap_chain_4));

	return swap_chain_4;
}

ComPtr<ID3D12DescriptorHeap> createDescriptorHeap(
	ComPtr<ID3D12Device2> device,
	D3D12_DESCRIPTOR_HEAP_TYPE type,
	uint32_t n_descriptors) {

	ComPtr<ID3D12DescriptorHeap> descriptor_heap;

	D3D12_DESCRIPTOR_HEAP_DESC desc = {};

	desc.NumDescriptors = n_descriptors;
	desc.Type = type;

	ThrowIfFailed(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&descriptor_heap)));

	return descriptor_heap;
}

void updateRenderTargetViews(
	ComPtr<ID3D12Device2> device,
	ComPtr<IDXGISwapChain4> swap_chain,
	ComPtr<ID3D12DescriptorHeap> descriptor_heap) {

	auto rtv_descriptor_size =
		device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtv_handle(descriptor_heap->GetCPUDescriptorHandleForHeapStart());

	for (int i = 0; i < N_FRAMEBUFFERS; ++i) {
		ComPtr<ID3D12Resource> framebuffer;
		ThrowIfFailed(swap_chain->GetBuffer(i, IID_PPV_ARGS(&framebuffer)));

		device->CreateRenderTargetView(framebuffer.Get(), nullptr, rtv_handle);

		d3d12.framebuffers[i] = framebuffer;
		rtv_handle.Offset(rtv_descriptor_size);
	}
}

ComPtr<ID3D12CommandAllocator> createCommandAllocator(
	ComPtr<ID3D12Device2> device,
	D3D12_COMMAND_LIST_TYPE type) {

	ComPtr<ID3D12CommandAllocator> command_allocator;
	ThrowIfFailed(device->CreateCommandAllocator(type, IID_PPV_ARGS(&command_allocator)));

	return command_allocator;
}

ComPtr<ID3D12GraphicsCommandList> createCommandList(
	ComPtr<ID3D12Device2> device,
	ComPtr<ID3D12CommandAllocator> command_allocator,
	D3D12_COMMAND_LIST_TYPE type) {

	ComPtr<ID3D12GraphicsCommandList> command_list;
	ThrowIfFailed(device->CreateCommandList(0, type,
		command_allocator.Get(), nullptr, IID_PPV_ARGS(&command_list)));

	ThrowIfFailed(command_list->Close());

	return command_list;
}

ComPtr<ID3D12Fence> createFence(ComPtr<ID3D12Device2> device) {
	ComPtr<ID3D12Fence> fence;

	ThrowIfFailed(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));

	return fence;
}

HANDLE createEventHandle() {
	HANDLE fence_event;

	fence_event = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	assert(fence_event && "Failed to create fence event");

	return fence_event;
}

uint64_t signal(
	ComPtr<ID3D12CommandQueue> command_queue,
	ComPtr<ID3D12Fence> fence,
	uint64_t& fence_value) {

	uint64_t value = ++fence_value;
	ThrowIfFailed(command_queue->Signal(fence.Get(), value));

	return value;
}

void waitForFenceValue(
	ComPtr<ID3D12Fence> fence,
	uint64_t fence_value,
	HANDLE fence_event,
	std::chrono::milliseconds duration = std::chrono::milliseconds::max()) {

	if (fence->GetCompletedValue() < fence_value) {
		ThrowIfFailed(fence->SetEventOnCompletion(fence_value, fence_event));
		WaitForSingleObject(fence_event, static_cast<DWORD>(duration.count()));
	}
}

void flush(
	ComPtr<ID3D12CommandQueue> command_queue,
	ComPtr<ID3D12Fence> fence,
	uint64_t& fence_value,
	HANDLE fence_event) {

	uint64_t fence_value_for_signal = signal(command_queue, fence, fence_value);
	waitForFenceValue(fence, fence_value, fence_event);
}

void update() {
	static uint64_t frame_counter = 0;
	static double elapsed_seconds = 0.0;
	static std::chrono::high_resolution_clock clock;
	static auto t0 = clock.now();

	++frame_counter;
	auto t1 = clock.now();
	auto delta_time = t1 - t0;
	t0 = t1;
	elapsed_seconds += delta_time.count() * 1e-9;

	if (elapsed_seconds > 1.0) {
		wchar_t buffer[500];
		auto fps = frame_counter / elapsed_seconds;
		wsprintf(buffer, L"FPS: %f\n", fps);
		OutputDebugString(buffer);
		frame_counter = 0;
		elapsed_seconds = 0.0;
	}
}

void render() {
	auto command_allocator = d3d12.command_allocators[d3d12.current_framebuffer_id];
	auto framebuffer = d3d12.framebuffers[d3d12.current_framebuffer_id];

	command_allocator->Reset();
	d3d12.command_list->Reset(command_allocator.Get(), nullptr);

	CD3DX12_RESOURCE_BARRIER barrier_1 = CD3DX12_RESOURCE_BARRIER::Transition(
		framebuffer.Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

	d3d12.command_list->ResourceBarrier(1, &barrier_1);

	FLOAT clear_color[] = { 0.0f, 0.2f, 0.0f, 1.0f };
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtv(
		d3d12.rtv_descriptor_heap->GetCPUDescriptorHandleForHeapStart(),
		d3d12.current_framebuffer_id, d3d12.rtv_descriptor_size);

	d3d12.command_list->ClearRenderTargetView(rtv, clear_color, 0, nullptr);

	CD3DX12_RESOURCE_BARRIER barrier_2 = CD3DX12_RESOURCE_BARRIER::Transition(
		framebuffer.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

	d3d12.command_list->ResourceBarrier(1, &barrier_2);
	ThrowIfFailed(d3d12.command_list->Close());

	ID3D12CommandList* const command_lists[] = {
		d3d12.command_list.Get()
	};

	d3d12.command_queue->ExecuteCommandLists(_countof(command_lists), command_lists);

	UINT sync_interval = window.vsync ? 1 : 0;
	UINT present_flags = 0;

	ThrowIfFailed(d3d12.swap_chain->Present(sync_interval, present_flags));

	d3d12.frame_fence_values[d3d12.current_framebuffer_id] =
		signal(d3d12.command_queue, d3d12.fence, d3d12.fence_value);

	d3d12.current_framebuffer_id = d3d12.swap_chain->GetCurrentBackBufferIndex();
	waitForFenceValue(d3d12.fence,
		d3d12.frame_fence_values[d3d12.current_framebuffer_id], d3d12.fence_event);
}

void resize(uint32_t width, uint32_t height) {
	if (window.width != width || window.height != height) {
		window.width = std::max(1u, width);
		window.height = std::max(1u, height);

		flush(d3d12.command_queue, d3d12.fence, d3d12.fence_value, d3d12.fence_event);

		for (int i = 0; i < N_FRAMEBUFFERS; ++i) {
			d3d12.framebuffers[i].Reset();
			d3d12.frame_fence_values[i] = d3d12.frame_fence_values[d3d12.current_framebuffer_id];
		}

		DXGI_SWAP_CHAIN_DESC scd = {};

		ThrowIfFailed(d3d12.swap_chain->GetDesc(&scd));
		ThrowIfFailed(d3d12.swap_chain->ResizeBuffers(N_FRAMEBUFFERS,
			window.width, window.height, scd.BufferDesc.Format, scd.Flags));

		d3d12.current_framebuffer_id = d3d12.swap_chain->GetCurrentBackBufferIndex();

		updateRenderTargetViews(d3d12.device, d3d12.swap_chain, d3d12.rtv_descriptor_heap);
	}
}

void setFullscreen(bool fs) {
	if (window.fullscreen != fs) {
		window.fullscreen = fs;

		if (window.fullscreen) {
			GetWindowRect(window.h_wnd, &window.rect);

			HMONITOR monitor = MonitorFromWindow(window.h_wnd, MONITOR_DEFAULTTONEAREST);
			MONITORINFOEX mi = {};
			mi.cbSize = sizeof(MONITORINFOEX);
			GetMonitorInfo(monitor, &mi);

			SetWindowPos(window.h_wnd, HWND_TOP,
				mi.rcMonitor.left,
				mi.rcMonitor.top,
				mi.rcMonitor.right - mi.rcMonitor.left,
				mi.rcMonitor.bottom - mi.rcMonitor.top,
				SWP_FRAMECHANGED | SWP_NOACTIVATE);

			ShowWindow(window.h_wnd, SW_MAXIMIZE);
		}
		else {
			SetWindowLong(window.h_wnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);

			SetWindowPos(window.h_wnd, HWND_NOTOPMOST,
				window.rect.left,
				window.rect.top,
				window.rect.right - window.rect.left,
				window.rect.bottom - window.rect.top,
				SWP_FRAMECHANGED | SWP_NOACTIVATE);

			ShowWindow(window.h_wnd, SW_NORMAL);
		}
	}
}

LRESULT CALLBACK windowProc(HWND h_wnd, UINT msg, WPARAM w_param, LPARAM l_param) {
	if (d3d12.initialized) {
		switch (msg) {
			case WM_PAINT:
				update();
				render();
				break;

			case WM_SYSKEYDOWN:
			case WM_KEYDOWN: {
				bool alt = (GetAsyncKeyState(VK_MENU) & 0x8000) != 0;

				switch (w_param) {
					case 'V':
						window.vsync != window.vsync;
						break;

					case VK_ESCAPE:
						PostQuitMessage(0);
						break;

					case VK_RETURN:
						if (alt)
							setFullscreen(!window.fullscreen);
						break;

					case VK_F11:
						setFullscreen(!window.fullscreen);
				}

				break;
			}

			case WM_SYSCHAR:
				break;

			case WM_SIZE: {
				RECT client = {};
				GetClientRect(h_wnd, &client);

				int w = client.right - client.left;
				int h = client.bottom - client.top;

				resize(w, h);

				break;
			}

			case WM_DESTROY:
				PostQuitMessage(0);
				break;

			default:
				return DefWindowProcW(h_wnd, msg, w_param, l_param);
		}
	}
	else
		return DefWindowProcW(h_wnd, msg, w_param, l_param);

	return 0;
}

int WINAPI wWinMain(HINSTANCE h_instance, HINSTANCE, PWSTR, int n_cmd_show) {
	SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

	wchar_t const* wc_name = L"D3D12wc";

	enableDebugLayer();
	registerWindowClass(h_instance, wc_name);
	window.h_wnd = createWindow(wc_name, h_instance, L"D3D12", window.width, window.height);
	GetWindowRect(window.h_wnd, &window.rect);

	ComPtr<IDXGIAdapter4> dxgi_adapter_4 = getAdapter();
	d3d12.device = createDevice(dxgi_adapter_4);
	d3d12.command_queue = createCommandQueue(d3d12.device, D3D12_COMMAND_LIST_TYPE_DIRECT);

	d3d12.swap_chain = createSwapChain(window.h_wnd, d3d12.command_queue,
		window.width, window.height, N_FRAMEBUFFERS);
	d3d12.current_framebuffer_id = d3d12.swap_chain->GetCurrentBackBufferIndex();
	d3d12.rtv_descriptor_heap = createDescriptorHeap(d3d12.device,
		D3D12_DESCRIPTOR_HEAP_TYPE_RTV, N_FRAMEBUFFERS);
	d3d12.rtv_descriptor_size = d3d12.device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	updateRenderTargetViews(d3d12.device, d3d12.swap_chain, d3d12.rtv_descriptor_heap);

	for (int i = 0; i < N_FRAMEBUFFERS; ++i)
		d3d12.command_allocators[i] = createCommandAllocator(d3d12.device, D3D12_COMMAND_LIST_TYPE_DIRECT);

	d3d12.command_list = createCommandList(d3d12.device,
		d3d12.command_allocators[d3d12.current_framebuffer_id], D3D12_COMMAND_LIST_TYPE_DIRECT);

	d3d12.fence = createFence(d3d12.device);
	d3d12.fence_event = createEventHandle();

	d3d12.initialized = true;

	ShowWindow(window.h_wnd, SW_SHOW);

	MSG msg = {};

	while (msg.message != WM_QUIT) {
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	flush(d3d12.command_queue, d3d12.fence, d3d12.fence_value, d3d12.fence_event);

	CloseHandle(d3d12.fence_event);

	return 0;
}
