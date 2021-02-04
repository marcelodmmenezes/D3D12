#pragma once

#include "config.hpp"

Microsoft::WRL::ComPtr<ID3D12Resource> createDefaultBuffer(
	Microsoft::WRL::ComPtr<ID3D12Device> device,
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list,
	void const* data,
	UINT64 size_in_bytes,
	Microsoft::WRL::ComPtr<ID3D12Resource>& upload_buffer)
{
	Microsoft::WRL::ComPtr<ID3D12Resource> default_buffer;

	auto default_heap_properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	auto aux_buffer = CD3DX12_RESOURCE_DESC::Buffer(size_in_bytes);

	THROW_IF_FAILED(device->CreateCommittedResource(
		&default_heap_properties,
		D3D12_HEAP_FLAG_NONE,
		&aux_buffer,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&default_buffer)));

	auto upload_heap_properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	aux_buffer = CD3DX12_RESOURCE_DESC::Buffer(size_in_bytes);

	THROW_IF_FAILED(device->CreateCommittedResource(
		&upload_heap_properties,
		D3D12_HEAP_FLAG_NONE,
		&aux_buffer,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&upload_buffer)));

	D3D12_SUBRESOURCE_DATA subresource_data = {};
	subresource_data.pData = data;
	subresource_data.RowPitch = size_in_bytes;
	subresource_data.SlicePitch = subresource_data.RowPitch;

	auto transition = CD3DX12_RESOURCE_BARRIER::Transition(
		default_buffer.Get(),
		D3D12_RESOURCE_STATE_COMMON,
		D3D12_RESOURCE_STATE_COPY_DEST);

	command_list->ResourceBarrier(1, &transition);

	UpdateSubresources<1>(
		command_list.Get(),
		default_buffer.Get(),
		upload_buffer.Get(),
		0,
		0,
		1,
		&subresource_data);

	transition = CD3DX12_RESOURCE_BARRIER::Transition(
		default_buffer.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST,
		D3D12_RESOURCE_STATE_GENERIC_READ);

	command_list->ResourceBarrier(1, &transition);

	return default_buffer;
}

UINT calcConstantBufferSizeInBytes(UINT size_in_bytes)
{
	return (size_in_bytes + 255) & ~255;
}

Microsoft::WRL::ComPtr<ID3DBlob> compileShader(
	std::wstring const& file,
	D3D_SHADER_MACRO* defines,
	std::string const& entry_point,
	std::string const& target)
{
	UINT compile_flags = 0;

#if defined(_DEBUG)
	compile_flags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	HRESULT result = S_OK;

	Microsoft::WRL::ComPtr<ID3DBlob> byte_code;
	Microsoft::WRL::ComPtr<ID3DBlob> errors;

	result = D3DCompileFromFile(
		file.c_str(),
		defines,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		entry_point.c_str(),
		target.c_str(),
		compile_flags,
		0,
		&byte_code,
		&errors);

	if (errors)
	{
		OutputDebugStringA(reinterpret_cast<char*>(errors->GetBufferPointer()));
	}

	THROW_IF_FAILED(result);

	return byte_code;
}
