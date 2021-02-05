#pragma once

#include "config.hpp"
#include "helpers.hpp"

template <typename T>
class UploadBuffer
{
public:
	UploadBuffer(UploadBuffer const&) = delete;
	UploadBuffer& operator=(UploadBuffer const&) = delete;

	UploadBuffer(
		Microsoft::WRL::ComPtr<ID3D12Device> device,
		UINT n_elements,
		bool is_constant_buffer)
		:
		is_constant_buffer{ is_constant_buffer }
	{
		element_size_in_bytes = sizeof(T);

		if (is_constant_buffer)
		{
			element_size_in_bytes = calcConstantBufferSizeInBytes(sizeof(T));
		}

		auto heap_properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		auto buffer = CD3DX12_RESOURCE_DESC::Buffer(element_size_in_bytes * n_elements);

		THROW_IF_FAILED(device->CreateCommittedResource(
			&heap_properties,
			D3D12_HEAP_FLAG_NONE,
			&buffer,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&upload_buffer)));

		THROW_IF_FAILED(upload_buffer->Map(0, nullptr, reinterpret_cast<void**>(&mapped_data)));
	}

	~UploadBuffer()
	{
		if (upload_buffer)
		{
			upload_buffer->Unmap(0, nullptr);
		}

		mapped_data = nullptr;
	}

	Microsoft::WRL::ComPtr<ID3D12Resource> resource() const
	{
		return upload_buffer;
	}

	void copyData(int element_index, T const& data)
	{
		memcpy(&mapped_data[element_index * element_size_in_bytes], &data, sizeof(T));
	}

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> upload_buffer;
	BYTE* mapped_data = nullptr;

	UINT element_size_in_bytes = 0;
	bool is_constant_buffer = false;
};
