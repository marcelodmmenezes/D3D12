#pragma once

#include "window.hpp"

#include <vector>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

class D3D12
{
public:
	static void enableDebugLayer();

	D3D12(Window* window);

private:
	void selectAdapter();
	void logAdapterOutputs(IDXGIAdapter* adapter);
	void logOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format);

	void createDevice();
	void checkMSAASupport();

	void createFence();
	void createCommandStructure();

	void createSwapChain();
	void createDescriptorHeaps();

	D3D12_CPU_DESCRIPTOR_HANDLE currentRTV() const;
	D3D12_CPU_DESCRIPTOR_HANDLE currentDSV() const;

	void createRTV();
	void createDSV();

	void setViewportAndScissors();

	Window* window;

	Microsoft::WRL::ComPtr<IDXGIFactory> dxgi_factory;
	Microsoft::WRL::ComPtr<IDXGIAdapter> dxgi_adapter;

	Microsoft::WRL::ComPtr<ID3D12Device> device;

	Microsoft::WRL::ComPtr<ID3D12Fence> fence;
	UINT64 fence_value;

	Microsoft::WRL::ComPtr<ID3D12CommandQueue> command_queue;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> command_allocator;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list;

	Microsoft::WRL::ComPtr<IDXGISwapChain> swap_chain;
	
	UINT const n_back_buffers = 2;
	UINT current_back_buffer;
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> back_buffers;
	Microsoft::WRL::ComPtr<ID3D12Resource> depth_buffer;

	UINT const msaa_n_samples = 4;
	UINT msaa_quality;

	DXGI_FORMAT const back_buffer_format = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT const depth_buffer_format = DXGI_FORMAT_R32_FLOAT;

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtv_heap;
	UINT rtv_descriptor_size;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsv_heap;
	UINT dsv_descriptor_size;

	UINT cbv_srv_uav_descriptor_size;
};
