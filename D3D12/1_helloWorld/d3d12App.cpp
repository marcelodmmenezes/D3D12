#include "d3d12App.hpp"

#include <windowsx.h>

LRESULT CALLBACK windowProc(
	HWND hwnd,
	UINT msg,
	WPARAM w_param,
	LPARAM l_param)
{
	return D3D12App::getApp()->msgProc(hwnd, msg, w_param, l_param);
}

D3D12App* D3D12App::app = nullptr;

D3D12App* D3D12App::getApp()
{
	return app;
}

D3D12App::D3D12App(HINSTANCE instance)
{
	assert(app == nullptr);
	app = this;
}

D3D12App::~D3D12App()
{
	if (device)
	{
		flushCommandQueue();
	}
}

HINSTANCE D3D12App::appInstance() const
{
	return app_instance;
}

HWND D3D12App::windowHandle() const
{
	return window_handle;
}

float D3D12App::aspectRatio() const
{
	return static_cast<float>(client_width) / client_height;
}

int D3D12App::run()
{
	MSG msg = {};

	timer.reset();

	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, nullptr, 0u, 0u, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			timer.tick();

			if (!paused)
			{
				calcFrameStats();
				update(timer);
				draw(timer);
			}
			else
			{
				Sleep(100);
			}
		}
	}

	return static_cast<int>(msg.wParam);
}

bool D3D12App::init()
{
	if (!initWindow())
	{
		return false;
	}

	if (!initDirect3D())
	{
		return false;
	}

	onResize();

	return true;
}

void D3D12App::createRTVAndDSVDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC rtv_heap_desc;
	rtv_heap_desc.NumDescriptors = n_swap_chain_buffers;
	rtv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtv_heap_desc.NodeMask = 0;

	THROW_IF_FAILED(device->CreateDescriptorHeap(&rtv_heap_desc, IID_PPV_ARGS(&rtv_heap)));

	D3D12_DESCRIPTOR_HEAP_DESC dsv_heap_desc;
	dsv_heap_desc.NumDescriptors = 1;
	dsv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsv_heap_desc.NodeMask = 0;

	THROW_IF_FAILED(device->CreateDescriptorHeap(&dsv_heap_desc, IID_PPV_ARGS(&dsv_heap)));
}

void D3D12App::onResize()
{
	assert(device);
	assert(swap_chain);
	assert(command_allocator);

	flushCommandQueue();

	THROW_IF_FAILED(command_list->Reset(command_allocator.Get(), nullptr));

	for (int i = 0; i < n_swap_chain_buffers; ++i)
	{
		swap_chain_buffers[i].Reset();
	}

	depth_stencil_buffer.Reset();

	THROW_IF_FAILED(swap_chain->ResizeBuffers(
		n_swap_chain_buffers,
		client_width,
		client_height,
		swap_chain_buffer_format,
		DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

	current_swap_chain_buffer = swap_chain->GetCurrentBackBufferIndex();
	
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtv_heap_handle(
		rtv_heap->GetCPUDescriptorHandleForHeapStart());

	for (UINT i = 0; i < n_swap_chain_buffers; ++i)
	{
		THROW_IF_FAILED(swap_chain->GetBuffer(i, IID_PPV_ARGS(&swap_chain_buffers[i])));

		device->CreateRenderTargetView(swap_chain_buffers[i].Get(), nullptr, rtv_heap_handle);

		rtv_heap_handle.Offset(1, rtv_descriptor_size);
	}

	D3D12_RESOURCE_DESC depth_stencil_desc;
	depth_stencil_desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depth_stencil_desc.Alignment = 0;
	depth_stencil_desc.Width = client_width;
	depth_stencil_desc.Height = client_height;
	depth_stencil_desc.DepthOrArraySize = 1;
	depth_stencil_desc.MipLevels = 1;
	depth_stencil_desc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	depth_stencil_desc.SampleDesc = { 1, 0 };
	depth_stencil_desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depth_stencil_desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE clear_value;
	clear_value.Format = depth_stencil_buffer_format;
	clear_value.DepthStencil = { 1.0f, 0 };

	auto heap_properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

	THROW_IF_FAILED(device->CreateCommittedResource(
		&heap_properties,
		D3D12_HEAP_FLAG_NONE,
		&depth_stencil_desc,
		D3D12_RESOURCE_STATE_COMMON,
		&clear_value,
		IID_PPV_ARGS(&depth_stencil_buffer)));

	D3D12_DEPTH_STENCIL_VIEW_DESC dsv_desc;
	dsv_desc.Flags = D3D12_DSV_FLAG_NONE;
	dsv_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsv_desc.Format = depth_stencil_buffer_format;
	dsv_desc.Texture2D.MipSlice = 0;
	device->CreateDepthStencilView(depth_stencil_buffer.Get(), &dsv_desc, depthStencilView());

	auto transition = CD3DX12_RESOURCE_BARRIER::Transition(
		depth_stencil_buffer.Get(),
		D3D12_RESOURCE_STATE_COMMON,
		D3D12_RESOURCE_STATE_DEPTH_WRITE);

	command_list->ResourceBarrier(1, &transition);

	THROW_IF_FAILED(command_list->Close());
	
	ID3D12CommandList* command_lists[]
	{
		command_list.Get()
	};

	command_queue->ExecuteCommandLists(1, command_lists);

	flushCommandQueue();

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = static_cast<float>(client_width);
	viewport.Height = static_cast<float>(client_height);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	scissor = { 0, 0, client_width, client_height };
}

LRESULT D3D12App::msgProc(
	HWND hwnd,
	UINT msg,
	WPARAM w_param,
	LPARAM l_param)
{
	switch (msg)
	{
	case WM_ACTIVATE:
		if (LOWORD(w_param) == WA_INACTIVE)
		{
			paused = true;
			timer.stop();
		}
		else
		{
			paused = false;
			timer.start();
		}

		return 0;

	case WM_SIZE:
		client_width = LOWORD(l_param);
		client_height = HIWORD(w_param);

		if (device)
		{
			if (w_param == SIZE_MINIMIZED)
			{
				paused = true;
				minimized = true;
				maximized = false;
			}
			else if (w_param == SIZE_MAXIMIZED)
			{
				paused = false;
				minimized = false;
				maximized = true;

				onResize();
			}
			else if (w_param == SIZE_RESTORED)
			{
				if (minimized)
				{
					paused = false;
					minimized = false;

					onResize();
				}
				else if (maximized)
				{
					paused = false;
					maximized = false;

					onResize();
				}
				else if (!resizing)
				{
					onResize();
				}
			}
		}

		return 0;

	case WM_ENTERSIZEMOVE:
		paused = true;
		resizing = true;
		timer.stop();

		return 0;

	case WM_EXITSIZEMOVE:
		paused = false;
		resizing = false;
		timer.start();
		
		onResize();

		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);

		return 0;

	case WM_MENUCHAR:
		return MAKELRESULT(0, MNC_CLOSE);

	case WM_GETMINMAXINFO:
		((MINMAXINFO*)l_param)->ptMinTrackSize.x = 200;
		((MINMAXINFO*)l_param)->ptMinTrackSize.y = 200;

		return 0;

	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		onMouseDown(w_param, GET_X_LPARAM(l_param), GET_Y_LPARAM(l_param));

		return 0;

	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		onMouseUp(w_param, GET_X_LPARAM(l_param), GET_Y_LPARAM(l_param));

		return 0;

	case WM_MOUSEMOVE:
		onMouseMove(w_param, GET_X_LPARAM(l_param), GET_Y_LPARAM(l_param));

		return 0;

	case WM_KEYUP:
		if (w_param == VK_ESCAPE)
		{
			PostQuitMessage(0);
		}

		return 0;
	}

	return DefWindowProc(hwnd, msg, w_param, l_param);
}

bool D3D12App::initWindow()
{
	WNDCLASS window_class = {};
	window_class.style = CS_HREDRAW | CS_VREDRAW;
	window_class.lpfnWndProc = windowProc;
	window_class.hInstance = app_instance;
	window_class.hIcon = LoadIcon(0, IDI_APPLICATION);
	window_class.hCursor = LoadCursor(0, IDC_ARROW);
	window_class.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	window_class.lpszClassName = L"MAIN_WINDOW";

	if (!RegisterClass(&window_class))
	{
		MessageBox(0, L"RegisterClass failed.", 0, 0);
		return false;
	}

	RECT rect = { 0, 0, client_width, client_height };
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;

	window_handle = CreateWindow(
		L"MAIN_WINDOW",
		window_title.c_str(),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		width,
		height,
		nullptr,
		nullptr,
		app_instance,
		nullptr);

	if (!window_handle)
	{
		MessageBox(0, L"CreateWindow failed.", 0, 0);
		return false;
	}

	ShowWindow(window_handle, SW_SHOW);
	UpdateWindow(window_handle);

	return true;
}

bool D3D12App::initDirect3D()
{
#if defined(_DEBUG)
	{
		Microsoft::WRL::ComPtr<ID3D12Debug> debug_controller;
		THROW_IF_FAILED(D3D12GetDebugInterface(IID_PPV_ARGS(&debug_controller)));
		debug_controller->EnableDebugLayer();
	}
#endif

	THROW_IF_FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&dxgi_factory)));
	THROW_IF_FAILED(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&device)));
	THROW_IF_FAILED(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));

	rtv_descriptor_size = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	dsv_descriptor_size = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	cbv_srv_uav_descriptor_size = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

#if defined(_DEBUG)
	logAdapters();
#endif

	createCommandStructure();
	createSwapChain();
	createRTVAndDSVDescriptorHeaps();

	return true;
}

void D3D12App::createCommandStructure()
{
	D3D12_COMMAND_QUEUE_DESC queue_desc = {};
	queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	THROW_IF_FAILED(device->CreateCommandQueue(
		&queue_desc,
		IID_PPV_ARGS(&command_queue)));

	THROW_IF_FAILED(device->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(&command_allocator)));

	THROW_IF_FAILED(device->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		command_allocator.Get(),
		nullptr,
		IID_PPV_ARGS(&command_list)));

	command_list->Close();
}

void D3D12App::createSwapChain()
{
	swap_chain.Reset();

	DXGI_SWAP_CHAIN_DESC1 swap_chain_desc = {};
	swap_chain_desc.Width = client_width;
	swap_chain_desc.Height = client_height;
	swap_chain_desc.Format = swap_chain_buffer_format;
	swap_chain_desc.Stereo = FALSE;
	swap_chain_desc.SampleDesc = { 1, 0 };
	swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swap_chain_desc.BufferCount = n_swap_chain_buffers;
	swap_chain_desc.Scaling = DXGI_SCALING_STRETCH;
	swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swap_chain_desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	swap_chain_desc.Flags = 0;

	Microsoft::WRL::ComPtr<IDXGISwapChain1> swap_chain_1;

	THROW_IF_FAILED(dxgi_factory->CreateSwapChainForHwnd(
		command_queue.Get(),
		window_handle,
		&swap_chain_desc,
		nullptr,
		nullptr,
		&swap_chain_1));

	THROW_IF_FAILED(dxgi_factory->MakeWindowAssociation(window_handle, DXGI_MWA_NO_ALT_ENTER));
	THROW_IF_FAILED(swap_chain_1.As(&swap_chain));
}

void D3D12App::flushCommandQueue()
{
	++fence_value;

	THROW_IF_FAILED(command_queue->Signal(fence.Get(), fence_value));

	if (fence->GetCompletedValue() < fence_value)
	{
		HANDLE event_handle = CreateEventEx(nullptr, FALSE, FALSE, EVENT_ALL_ACCESS);

		THROW_IF_FAILED(fence->SetEventOnCompletion(fence_value, event_handle));

		WaitForSingleObject(event_handle, INFINITE);
		CloseHandle(event_handle);
	}
}

Microsoft::WRL::ComPtr<ID3D12Resource> D3D12App::currentSwapChainBuffer() const
{
	return swap_chain_buffers[current_swap_chain_buffer];
}

D3D12_CPU_DESCRIPTOR_HANDLE D3D12App::currentSwapChainBufferView() const
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(
		rtv_heap->GetCPUDescriptorHandleForHeapStart(),
		current_swap_chain_buffer,
		rtv_descriptor_size);
}

D3D12_CPU_DESCRIPTOR_HANDLE D3D12App::depthStencilView() const
{
	return dsv_heap->GetCPUDescriptorHandleForHeapStart();
}

void D3D12App::calcFrameStats()
{
	static int frame_count = 0;
	static float time_elapsed = 0.0f;

	++frame_count;

	if (timer.totalTime() - time_elapsed >= 1.0f)
	{
		float fps = static_cast<float>(frame_count);
		float mspf = 1000.0f / fps;

		std::wstring fps_str = std::to_wstring(fps);
		std::wstring mspf_str = std::to_wstring(mspf);

		std::wstring window_text = window_title +
			L"    fps: " + fps_str +
			L"   mspf: " + mspf_str;

		SetWindowText(window_handle, window_text.c_str());

		frame_count = 0;
		time_elapsed += 1.0f;
	}
}

void D3D12App::logAdapters()
{
	Microsoft::WRL::ComPtr<IDXGIAdapter> adapter;

	for (UINT i = 0;
		dxgi_factory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND;
		++i)
	{
		DXGI_ADAPTER_DESC adapter_desc;

		adapter->GetDesc(&adapter_desc);

		std::wstring msg = L"\tADAPTER: ";
		msg += adapter_desc.Description;
		msg += L"\n";

		OutputDebugString(msg.c_str());

		logAdapterOutputs(adapter);
	}
}

void D3D12App::logAdapterOutputs(Microsoft::WRL::ComPtr<IDXGIAdapter> adapter)
{
	Microsoft::WRL::ComPtr<IDXGIOutput> output;

	for (UINT i = 0;
		adapter->EnumOutputs(i, &output) != DXGI_ERROR_NOT_FOUND;
		++i)
	{
		DXGI_OUTPUT_DESC output_desc;

		output->GetDesc(&output_desc);

		std::wstring msg = L"\t\tOutput: ";
		msg += output_desc.DeviceName;
		msg += L"\n";

		OutputDebugString(msg.c_str());

		logOutputDisplayModes(output, swap_chain_buffer_format);
	}
}

void D3D12App::logOutputDisplayModes(
	Microsoft::WRL::ComPtr<IDXGIOutput> output,
	DXGI_FORMAT format)
{
	UINT count = 0;
	UINT flags = 0;

	output->GetDisplayModeList(format, flags, &count, nullptr);

	std::vector<DXGI_MODE_DESC> mode_descs(count);

	output->GetDisplayModeList(format, flags, &count, mode_descs.data());

	for (auto& it : mode_descs)
	{
		auto [num, den] = it.RefreshRate;

		std::wstring msg =
			L"Width = " + std::to_wstring(it.Width) + L" " +
			L"Height = " + std::to_wstring(it.Height) + L" " +
			L"Refresh = " + std::to_wstring(num) + L"/" + std::to_wstring(den) +
			L"\n";

		OutputDebugString(msg.c_str());
	}
}
