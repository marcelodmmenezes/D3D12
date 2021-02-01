#pragma once

#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include "config.hpp"
#include "gameTimer.hpp"

#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

class D3D12App
{
public:
	static D3D12App* getApp();

	HINSTANCE appInstance() const;
	HWND windowHandle() const;
	float aspectRatio() const;

	int run();

	virtual bool init();
	virtual LRESULT msgProc(
		HWND hwnd,
		UINT msg,
		WPARAM w_param,
		LPARAM l_param);

protected:
	D3D12App(HINSTANCE instance);
	D3D12App(D3D12App const&) = delete;
	D3D12App& operator=(D3D12App const&) = delete;
	virtual ~D3D12App();

	static D3D12App* app;

	virtual void createRTVAndDSVDescriptorHeaps();
	virtual void onResize();

	virtual void update(GameTimer const& timer) = 0;
	virtual void draw(GameTimer const& timer) = 0;

	virtual void onMouseDown(WPARAM state, int x, int y) {}
	virtual void onMouseUp(WPARAM state, int x, int y) {}
	virtual void onMouseMove(WPARAM state, int x, int y) {}

	bool initWindow();
	bool initDirect3D();
	void createCommandStructure();
	void createSwapChain();

	void flushCommandQueue();

	Microsoft::WRL::ComPtr<ID3D12Resource> currentSwapChainBuffer() const;
	D3D12_CPU_DESCRIPTOR_HANDLE currentSwapChainBufferView() const;
	D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView() const;

	void calcFrameStats();

	void logAdapters();
	void logAdapterOutputs(Microsoft::WRL::ComPtr<IDXGIAdapter> adapter);
	void logOutputDisplayModes(
		Microsoft::WRL::ComPtr<IDXGIOutput> output,
		DXGI_FORMAT format);

	HINSTANCE app_instance = nullptr;
	HWND window_handle = nullptr;
	bool paused = false;
	bool minimized = false;
	bool maximized = false;
	bool resizing = false;
	bool fullscreen_state = false;

	GameTimer timer;

	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgi_factory;
	Microsoft::WRL::ComPtr<IDXGISwapChain4> swap_chain;

	Microsoft::WRL::ComPtr<ID3D12Device8> device;
	Microsoft::WRL::ComPtr<ID3D12Fence> fence;
	UINT64 fence_value = 0u;

	Microsoft::WRL::ComPtr<ID3D12CommandQueue> command_queue;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> command_allocator;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList6> command_list;

	static UINT const n_swap_chain_buffers = 2;
	UINT current_swap_chain_buffer = 0u;
	Microsoft::WRL::ComPtr<ID3D12Resource> swap_chain_buffers[n_swap_chain_buffers];
	Microsoft::WRL::ComPtr<ID3D12Resource> depth_stencil_buffer;

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtv_heap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsv_heap;

	D3D12_VIEWPORT viewport = {};
	D3D12_RECT scissor = {};

	UINT rtv_descriptor_size = 0u;
	UINT dsv_descriptor_size = 0u;
	UINT cbv_srv_uav_descriptor_size = 0u;

	std::wstring window_title = L"D3D12";
	D3D_DRIVER_TYPE driver_type = D3D_DRIVER_TYPE_HARDWARE;
	DXGI_FORMAT swap_chain_buffer_format = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT depth_stencil_buffer_format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	int client_width = 1366;
	int client_height = 768;
};
