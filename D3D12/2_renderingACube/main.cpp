#include "d3d12App.hpp"
#include "dataDescription.hpp"
#include "math.hpp"
#include "mesh.hpp"
#include "uploadBuffer.hpp"

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

		THROW_IF_FAILED(command_list->Reset(command_allocator.Get(), nullptr));

		buildDescriptorHeaps();
		buildConstantBuffers();
		buildRootSignature();
		buildShadersAndInputLayout();
		buildGeometry();
		buildPSO();

		THROW_IF_FAILED(command_list->Close());

		ID3D12CommandList* command_lists[]
		{
			command_list.Get()
		};

		command_queue->ExecuteCommandLists(1, command_lists);

		flushCommandQueue();

		return true;
	}

private:
	void buildDescriptorHeaps()
	{
		D3D12_DESCRIPTOR_HEAP_DESC cbv_heap_desc;
		cbv_heap_desc.NumDescriptors = 1;
		cbv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		cbv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		cbv_heap_desc.NodeMask = 0;

		THROW_IF_FAILED(device->CreateDescriptorHeap(&cbv_heap_desc, IID_PPV_ARGS(&cbv_heap)));
	}

	void buildConstantBuffers()
	{
		object_cb = std::make_unique<UploadBuffer<ObjectConstants>>(device, 1, true);
		
		UINT object_cb_size_in_bytes = calcConstantBufferSizeInBytes(sizeof(ObjectConstants));
		D3D12_GPU_VIRTUAL_ADDRESS cb_address = object_cb->resource()->GetGPUVirtualAddress();

		int cube_cb_index = 0;
		cb_address += cube_cb_index * object_cb_size_in_bytes;

		D3D12_CONSTANT_BUFFER_VIEW_DESC cbv_desc;
		cbv_desc.BufferLocation = cb_address;
		cbv_desc.SizeInBytes = object_cb_size_in_bytes;

		device->CreateConstantBufferView(
			&cbv_desc,
			cbv_heap->GetCPUDescriptorHandleForHeapStart());
	}

	void buildRootSignature()
	{
		CD3DX12_ROOT_PARAMETER slot_root_parameter[1];

		CD3DX12_DESCRIPTOR_RANGE cbv_table;
		cbv_table.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
		slot_root_parameter[0].InitAsDescriptorTable(1, &cbv_table);

		CD3DX12_ROOT_SIGNATURE_DESC root_signature_desc(
			1,
			slot_root_parameter,
			0,
			nullptr,
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		Microsoft::WRL::ComPtr<ID3DBlob> serialized_root_signature;
		Microsoft::WRL::ComPtr<ID3DBlob> error_blob;

		HRESULT result = D3D12SerializeRootSignature(
			&root_signature_desc,
			D3D_ROOT_SIGNATURE_VERSION_1,
			&serialized_root_signature,
			&error_blob);

		if (error_blob)
		{
			OutputDebugStringA(reinterpret_cast<char*>(error_blob->GetBufferPointer()));
		}

		THROW_IF_FAILED(result);

		THROW_IF_FAILED(device->CreateRootSignature(
			0,
			serialized_root_signature->GetBufferPointer(),
			serialized_root_signature->GetBufferSize(),
			IID_PPV_ARGS(&root_signature)));
	}

	void buildShadersAndInputLayout()
	{
		HRESULT result = S_OK;

		vertex_shader_byte_code = compileShader(L"color.hlsl", nullptr, "VS", "vs_5_0");
		pixel_shader_byte_code = compileShader(L"color.hlsl", nullptr, "PS", "ps_5_0");
	}

	void buildGeometry()
	{
		UINT const vertex_buffer_size_in_bytes = (UINT)sizeof(vertices);
		UINT const index_buffer_size_in_bytes = (UINT)sizeof(indices);

		cube = std::make_unique<Mesh>();
		cube->name = "cube";

		THROW_IF_FAILED(D3DCreateBlob(
			vertex_buffer_size_in_bytes,
			&cube->vertex_buffer_cpu));

		CopyMemory(
			cube->vertex_buffer_cpu->GetBufferPointer(),
			vertices,
			vertex_buffer_size_in_bytes);

		cube->vertex_buffer_gpu = createDefaultBuffer(
			device,
			command_list,
			vertices,
			vertex_buffer_size_in_bytes,
			cube->vertex_buffer_uploader);

		THROW_IF_FAILED(D3DCreateBlob(
			index_buffer_size_in_bytes,
			&cube->index_buffer_cpu));

		CopyMemory(
			cube->index_buffer_cpu->GetBufferPointer(),
			indices,
			index_buffer_size_in_bytes);

		cube->index_buffer_gpu = createDefaultBuffer(
			device,
			command_list,
			indices,
			index_buffer_size_in_bytes,
			cube->index_buffer_uploader);

		cube->vertex_buffer_stride_in_bytes = sizeof(Vertex);
		cube->vertex_buffer_size_in_bytes = vertex_buffer_size_in_bytes;
		cube->index_buffer_format = DXGI_FORMAT_R16_UINT;
		cube->index_buffer_size_in_bytes = index_buffer_size_in_bytes;

		SubmeshGeometry submesh;
		submesh.index_count = _countof(indices);
		submesh.start_index_location = 0;
		submesh.base_vertex_location = 0;

		cube->draw_args["cube"] = submesh;
	}
	
	void buildPSO()
	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC pso_desc = {};
		pso_desc.InputLayout = { input_layout, _countof(input_layout) };
		pso_desc.pRootSignature = root_signature.Get();
		pso_desc.VS =
		{
			reinterpret_cast<BYTE*>(vertex_shader_byte_code->GetBufferPointer()),
			vertex_shader_byte_code->GetBufferSize()
		};
		pso_desc.PS =
		{
			reinterpret_cast<BYTE*>(pixel_shader_byte_code->GetBufferPointer()),
			pixel_shader_byte_code->GetBufferSize()
		};
		pso_desc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		pso_desc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		pso_desc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		pso_desc.SampleMask = UINT_MAX;
		pso_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		pso_desc.NumRenderTargets = 1;
		pso_desc.RTVFormats[0] = swap_chain_buffer_format;
		pso_desc.SampleDesc = { 1, 0 };
		pso_desc.DSVFormat = depth_stencil_buffer_format;

		THROW_IF_FAILED(device->CreateGraphicsPipelineState(&pso_desc, IID_PPV_ARGS(&pso)));
	}

	virtual void onResize() override
	{
		D3D12App::onResize();

		XMStoreFloat4x4(
			&proj_matrix,
			DirectX::XMMatrixPerspectiveFovLH(0.25f * PI, aspectRatio(), 1.0f, 1000.0f));
	}

	virtual void update(GameTimer const& timer) override
	{
		float x = radius * sinf(phi) * cosf(theta);
		float y = radius * cosf(phi);
		float z = radius * sinf(phi) * sinf(theta);

		DirectX::XMVECTOR pos = DirectX::XMVectorSet(x, y, z, 1.0f);
		DirectX::XMVECTOR target = DirectX::XMVectorZero();
		DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

		DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(pos, target, up);
		DirectX::XMStoreFloat4x4(&view_matrix, view);

		DirectX::XMMATRIX world = DirectX::XMLoadFloat4x4(&world_matrix);
		DirectX::XMMATRIX proj = DirectX::XMLoadFloat4x4(&proj_matrix);
		DirectX::XMMATRIX world_view_proj = world * view * proj;

		ObjectConstants object_constants;

		DirectX::XMStoreFloat4x4(
			&object_constants.world_view_proj,
			DirectX::XMMatrixTranspose(world_view_proj));

		object_cb->copyData(0, object_constants);
	}

	virtual void draw(GameTimer const& timer) override
	{
		THROW_IF_FAILED(command_allocator->Reset());
		THROW_IF_FAILED(command_list->Reset(command_allocator.Get(), pso.Get()));

		command_list->RSSetViewports(1, &viewport);
		command_list->RSSetScissorRects(1, &scissor);

		auto transition = CD3DX12_RESOURCE_BARRIER::Transition(
			currentSwapChainBuffer().Get(),
			D3D12_RESOURCE_STATE_PRESENT,
			D3D12_RESOURCE_STATE_RENDER_TARGET);

		command_list->ResourceBarrier(1, &transition);

		auto rtv = currentSwapChainBufferView();
		auto dsv = depthStencilView();

		command_list->ClearRenderTargetView(rtv, clear_color, 0, nullptr);
		command_list->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
		command_list->OMSetRenderTargets(1, &rtv, TRUE, &dsv);

		ID3D12DescriptorHeap* descriptor_heaps[]
		{
			cbv_heap.Get()
		};

		auto vbv = cube->vertexBufferView();
		auto ibv = cube->indexBufferView();

		command_list->SetDescriptorHeaps(_countof(descriptor_heaps), descriptor_heaps);
		command_list->SetGraphicsRootSignature(root_signature.Get());
		command_list->IASetVertexBuffers(0, 1, &vbv);
		command_list->IASetIndexBuffer(&ibv);
		command_list->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		command_list->SetGraphicsRootDescriptorTable(0, cbv_heap->GetGPUDescriptorHandleForHeapStart());
		command_list->DrawIndexedInstanced(cube->draw_args["cube"].index_count, 1, 0, 0, 0);

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

		command_queue->ExecuteCommandLists(1, command_lists);

		THROW_IF_FAILED(swap_chain->Present(0, 0));

		current_swap_chain_buffer = swap_chain->GetCurrentBackBufferIndex();

		flushCommandQueue();
	}

	virtual void onMouseDown(WPARAM state, int x, int y) override
	{
		last_mouse_pos.x = x;
		last_mouse_pos.y = y;

		SetCapture(window_handle);
	}

	virtual void onMouseUp(WPARAM state, int x, int y) override
	{
		ReleaseCapture();
	}

	virtual void onMouseMove(WPARAM state, int x, int y) override
	{
		if ((state & MK_LBUTTON) != 0)
		{
			float dx = DirectX::XMConvertToRadians(0.25f * static_cast<float>(x - last_mouse_pos.x));
			float dy = DirectX::XMConvertToRadians(0.25f * static_cast<float>(y - last_mouse_pos.y));

			theta += dx;
			phi += dy;

			phi = clamp(phi, 0.1f, PI - 0.1f);
		}
		else if ((state & MK_RBUTTON) != 0)
		{
			float dx = 0.005f * static_cast<float>(x - last_mouse_pos.x);
			float dy = 0.005f * static_cast<float>(y - last_mouse_pos.y);

			radius += dx - dy;

			radius = clamp(radius, 3.0f, 15.0f);
		}

		last_mouse_pos.x = x;
		last_mouse_pos.y = y;
	}

	FLOAT const clear_color[4] = { 0.1f, 0.3f, 0.1f, 1.0f };

	Microsoft::WRL::ComPtr<ID3D12RootSignature> root_signature;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> cbv_heap;

	std::unique_ptr<UploadBuffer<ObjectConstants>> object_cb;
	std::unique_ptr<Mesh> cube;

	Microsoft::WRL::ComPtr<ID3DBlob> vertex_shader_byte_code;
	Microsoft::WRL::ComPtr<ID3DBlob> pixel_shader_byte_code;

	Microsoft::WRL::ComPtr<ID3D12PipelineState> pso;

	DirectX::XMFLOAT4X4 world_matrix = identity4x4();
	DirectX::XMFLOAT4X4 view_matrix = identity4x4();
	DirectX::XMFLOAT4X4 proj_matrix = identity4x4();

	float theta = 1.5f * DirectX::XM_PI;
	float phi = DirectX::XM_PIDIV4;
	float radius = 5.0f;

	POINT last_mouse_pos;
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
