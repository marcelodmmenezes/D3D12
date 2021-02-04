#include "d3d12App.hpp"

class App : public D3D12App
{
public:
	App(HINSTANCE instance)
		:
		D3D12App(instance) {}

	~App() {}

	virtual bool init() override
	{
		if (!D3D12App::init())
			return false;

		return true;
	}

private:
	virtual void onResize() override
	{
		D3D12App::onResize();
	}

	virtual void update(GameTimer const& timer) override {}

	virtual void draw(GameTimer const& timer) override
	{
		THROW_IF_FAILED(command_allocator->Reset());
		THROW_IF_FAILED(command_list->Reset(command_allocator.Get(), nullptr));
		
		auto transition = CD3DX12_RESOURCE_BARRIER::Transition(
			currentSwapChainBuffer().Get(),
			D3D12_RESOURCE_STATE_PRESENT,
			D3D12_RESOURCE_STATE_RENDER_TARGET);

		command_list->ResourceBarrier(1, &transition);
		command_list->RSSetViewports(1, &viewport);
		command_list->RSSetScissorRects(1, &scissor);

		command_list->ClearRenderTargetView(
			currentSwapChainBufferView(),
			clear_color,
			0,
			nullptr);

		command_list->ClearDepthStencilView(
			depthStencilView(),
			D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
			1.0f,
			0,
			0,
			nullptr);

		auto rtv = currentSwapChainBufferView();
		auto dsv = depthStencilView();

		command_list->OMSetRenderTargets(1, &rtv, true, &dsv);

		transition = CD3DX12_RESOURCE_BARRIER::Transition(
			currentSwapChainBuffer().Get(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PRESENT);

		command_list->ResourceBarrier(1, &transition);
		THROW_IF_FAILED(command_list->Close());

		ID3D12CommandList* command_lists[]
		{
			command_list.Get()
		};

		command_queue->ExecuteCommandLists(_countof(command_lists), command_lists);

		THROW_IF_FAILED(swap_chain->Present(0, 0));

		current_swap_chain_buffer = swap_chain->GetCurrentBackBufferIndex();

		flushCommandQueue();
	}

	FLOAT const clear_color[4] = { 0.1f, 0.3f, 0.1f, 1.0f };
};

int WINAPI wWinMain(HINSTANCE h_instance, HINSTANCE, PWSTR, int)
{
#if defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	try
	{
		App app(h_instance);

		if (!app.init())
			return 0;

		return app.run();
	}
	catch (Exception& ex)
	{
		MessageBox(nullptr, ex.toWString().c_str(), L"Fatal Error", MB_OK);
		return 0;
	}
}
