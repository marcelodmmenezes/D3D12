#include "frameResource.hpp"

FrameResource::FrameResource(ID3D12Device* device, UINT pass_count, UINT object_count)
{
	THROW_IF_FAILED(device->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(&command_list_allocator)));

	pass_cb = std::make_unique<UploadBuffer<PassConstants>>(device, pass_count, true);
	object_cb = std::make_unique<UploadBuffer<ObjectConstants>>(device, object_count, true);
}

FrameResource::~FrameResource()
{

}
