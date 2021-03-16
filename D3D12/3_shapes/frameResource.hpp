#pragma once

#include "math.hpp"
#include "uploadBuffer.hpp"

struct ObjectConstants
{
	DirectX::XMFLOAT4X4 world = identity4x4();
};

struct PassConstants
{
	DirectX::XMFLOAT4X4 view = identity4x4();
	DirectX::XMFLOAT4X4 inv_view = identity4x4();
	DirectX::XMFLOAT4X4 proj = identity4x4();
	DirectX::XMFLOAT4X4 inv_proj = identity4x4();
	DirectX::XMFLOAT4X4 view_proj = identity4x4();
	DirectX::XMFLOAT4X4 inv_view_proj = identity4x4();
	DirectX::XMFLOAT3 eye_pos_w = { 0.0f, 0.0f, 0.0f };
	float cb_per_object_pad_1 = 0.0f;
	DirectX::XMFLOAT2 render_target_size = { 0.0f, 0.0f };
	DirectX::XMFLOAT2 inv_render_target_size = { 0.0f, 0.0f };
	float near_z = 0.0f;
	float far_z = 0.0f;
	float total_time = 0.0f;
	float delta_time = 0.0f;
};

struct Vertex
{
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT4 color;
};

struct FrameResource
{
	FrameResource(ID3D12Device* device, UINT pass_count, UINT object_count)
	{
		THROW_IF_FAILED(device->CreateCommandAllocator(
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_PPV_ARGS(&command_list_allocator)));

		pass_cb = std::make_unique<UploadBuffer<PassConstants>>(device, pass_count, true);
		object_cb = std::make_unique<UploadBuffer<ObjectConstants>>(device, object_count, true);
	}

	~FrameResource()
	{

	}

	FrameResource(FrameResource const&) = delete;
	FrameResource& operator=(FrameResource const&) = delete;

	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> command_list_allocator;

	std::unique_ptr<UploadBuffer<PassConstants>> pass_cb = nullptr;
	std::unique_ptr<UploadBuffer<ObjectConstants>> object_cb = nullptr;

	UINT64 fence = 0;
};
