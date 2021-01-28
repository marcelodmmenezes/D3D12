#include "d3d12.hpp"

#include <string>

void D3D12::enableDebugLayer()
{
	Microsoft::WRL::ComPtr<ID3D12Debug> debug;
	throwIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debug)));
	debug->EnableDebugLayer();
}

D3D12::D3D12(Window* window)
	:
	window{ window }
{
	back_buffers.resize(n_back_buffers);

	throwIfFailed(CreateDXGIFactory(IID_PPV_ARGS(&dxgi_factory)));
	selectAdapter();
	createDevice();
	createFence();
	createCommandStructure();
}

void D3D12::selectAdapter()
{
	IDXGIAdapter* adapter = nullptr;
	DXGI_ADAPTER_DESC adapter_desc;
	std::wstring msg;

	// Selects adapter by size of dedicated video memory
	SIZE_T max_dedicated_video_memory = 0u;

	for (UINT i = 0;
		dxgi_factory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND;
		++i)
	{
		adapter->GetDesc(&adapter_desc);

		if (adapter_desc.DedicatedVideoMemory > max_dedicated_video_memory)
		{
			dxgi_adapter = adapter;
			max_dedicated_video_memory = adapter_desc.DedicatedVideoMemory;
		}

		msg = L"\tAdapter: ";
		msg += adapter_desc.Description;
		msg += L"\n";

		OutputDebugString(msg.c_str());

		logAdapterOutputs(adapter);
		adapter->Release();
	}
}

void D3D12::logAdapterOutputs(IDXGIAdapter* adapter)
{
	IDXGIOutput* output = nullptr;
	DXGI_OUTPUT_DESC output_desc;
	std::wstring msg;

	for (UINT i = 0;
		adapter->EnumOutputs(i, &output) != DXGI_ERROR_NOT_FOUND;
		++i)
	{
		output->GetDesc(&output_desc);

		msg = L"\t\tOutput: ";
		msg += output_desc.DeviceName;
		msg += L"\n";

		OutputDebugString(msg.c_str());

		logOutputDisplayModes(output, DXGI_FORMAT_B8G8R8A8_UNORM);
		output->Release();
	}
}

void D3D12::logOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format)
{
	std::wstring msg;

	UINT count = 0;
	UINT flags = 0;

	output->GetDisplayModeList(format, flags, &count, nullptr);

	std::vector<DXGI_MODE_DESC> mode_list(count);

	output->GetDisplayModeList(format, flags, &count, mode_list.data());

	for (auto& it : mode_list)
	{
		UINT num = it.RefreshRate.Numerator;
		UINT den = it.RefreshRate.Denominator;

		msg = L"\t\t\tResolution: " + std::to_wstring(it.Width);
		msg += L" x " + std::to_wstring(it.Height);
		msg += L"   " + std::to_wstring((double)num / den);
		msg += L"Hz\n";

		OutputDebugString(msg.c_str());
	}
}

void D3D12::createDevice()
{
	throwIfFailed(D3D12CreateDevice(dxgi_adapter.Get(),
		D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&device)));

#if defined (_DEBUG)
	Microsoft::WRL::ComPtr<ID3D12InfoQueue> info_queue;

	if (SUCCEEDED(device.As(&info_queue))) {
		info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
		info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
		info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);
	}
#endif

	rtv_descriptor_size =
		device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	dsv_descriptor_size =
		device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	cbv_srv_uav_descriptor_size =
		device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	checkMSAASupport();
}

void D3D12::checkMSAASupport()
{
	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS ms_quality_levels;
	ms_quality_levels.Format = back_buffer_format;
	ms_quality_levels.SampleCount = 4;
	ms_quality_levels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	ms_quality_levels.NumQualityLevels = 0;

	throwIfFailed(device->CheckFeatureSupport(
		D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
		&ms_quality_levels, sizeof(ms_quality_levels)));

	msaa_quality = ms_quality_levels.NumQualityLevels;

	assert(ms_quality_levels.NumQualityLevels > 0 &&
		"Quality levels for 4xMSAA should higher than 0");
}

void D3D12::createFence()
{
	throwIfFailed(device->CreateFence(0,
		D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));
}

void D3D12::createCommandStructure()
{
	D3D12_COMMAND_QUEUE_DESC queue_desc = {};
	queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	throwIfFailed(device->CreateCommandQueue(&queue_desc,
		IID_PPV_ARGS(&command_queue)));

	throwIfFailed(device->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(&command_allocator)));

	throwIfFailed(device->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		command_allocator.Get(),
		nullptr,
		IID_PPV_ARGS(&command_list)));

	command_list->Close();
}

void D3D12::createSwapChain()
{
	swap_chain.Reset();

	DXGI_SWAP_CHAIN_DESC swap_chain_desc;
	swap_chain_desc.BufferDesc.Width = window->getWidth();
	swap_chain_desc.BufferDesc.Height = window->getHeight();
	swap_chain_desc.BufferDesc.RefreshRate.Numerator = 60;
	swap_chain_desc.BufferDesc.RefreshRate.Denominator = 1;
	swap_chain_desc.BufferDesc.Format = back_buffer_format;
	swap_chain_desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swap_chain_desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swap_chain_desc.SampleDesc.Count = msaa_n_samples;
	swap_chain_desc.SampleDesc.Quality = msaa_quality;
	swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swap_chain_desc.BufferCount = n_back_buffers;
	swap_chain_desc.OutputWindow = window->getHandle();
	swap_chain_desc.Windowed = true;
	swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swap_chain_desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	throwIfFailed(dxgi_factory->CreateSwapChain(command_queue.Get(),
		&swap_chain_desc, &swap_chain));
}

void D3D12::createDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC rtv_heap_desc;
	rtv_heap_desc.NumDescriptors = n_back_buffers;
	rtv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtv_heap_desc.NodeMask = 0;

	throwIfFailed(device->CreateDescriptorHeap(&rtv_heap_desc, IID_PPV_ARGS(&rtv_heap)));

	D3D12_DESCRIPTOR_HEAP_DESC dsv_heap_desc;
	dsv_heap_desc.NumDescriptors = 1;
	dsv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsv_heap_desc.NodeMask = 0;

	throwIfFailed(device->CreateDescriptorHeap(&dsv_heap_desc, IID_PPV_ARGS(&dsv_heap)));
}

D3D12_CPU_DESCRIPTOR_HANDLE D3D12::currentRTV() const
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(
		rtv_heap->GetCPUDescriptorHandleForHeapStart(),
		current_back_buffer, rtv_descriptor_size);
}

D3D12_CPU_DESCRIPTOR_HANDLE D3D12::currentDSV() const
{
	return dsv_heap->GetCPUDescriptorHandleForHeapStart();
}

void D3D12::createRTV()
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtv_heap_handle(
		rtv_heap->GetCPUDescriptorHandleForHeapStart());

	for (UINT i = 0; i < n_back_buffers; ++i)
	{
		throwIfFailed(swap_chain->GetBuffer(i, IID_PPV_ARGS(&back_buffers[i])));

		device->CreateRenderTargetView(back_buffers[i].Get(), nullptr, rtv_heap_handle);

		rtv_heap_handle.Offset(1, rtv_descriptor_size);
	}
}

void D3D12::createDSV()
{
	D3D12_RESOURCE_DESC dsv_desc;
	dsv_desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	dsv_desc.Alignment = 0;
	dsv_desc.Width = window->getWidth();
	dsv_desc.Height = window->getHeight();
	dsv_desc.DepthOrArraySize = 1;
	dsv_desc.MipLevels = 1;
	dsv_desc.Format = depth_buffer_format;
	dsv_desc.SampleDesc.Count = msaa_n_samples;
	dsv_desc.SampleDesc.Quality = msaa_quality;
	dsv_desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	dsv_desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE clear_value;
	clear_value.Format = depth_buffer_format;
	clear_value.DepthStencil.Depth = 1.0f;
	clear_value.DepthStencil.Stencil = 0;

	auto heap_properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

	throwIfFailed(device->CreateCommittedResource(
		&heap_properties,
		D3D12_HEAP_FLAG_NONE,
		&dsv_desc,
		D3D12_RESOURCE_STATE_COMMON,
		&clear_value,
		IID_PPV_ARGS(&depth_buffer)));

	device->CreateDepthStencilView(
		depth_buffer.Get(), nullptr, currentDSV());

	auto transition = CD3DX12_RESOURCE_BARRIER::Transition(
		depth_buffer.Get(),
		D3D12_RESOURCE_STATE_COMMON,
		D3D12_RESOURCE_STATE_DEPTH_WRITE);

	command_list->ResourceBarrier(1, &transition);
}

void D3D12::setViewportAndScissors()
{
	D3D12_VIEWPORT viewport;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.Width = static_cast<float>(window->getWidth());
	viewport.Height = static_cast<float>(window->getHeight());
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	command_list->RSSetViewports(1, &viewport);

	D3D12_RECT scissor;
	scissor.left = 0;
	scissor.top = 0;
	scissor.right = window->getWidth() / 2;
	scissor.bottom = window->getHeight() / 2;

	command_list->RSSetScissorRects(1, &scissor);
}
