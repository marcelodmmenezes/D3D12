#pragma once

#include "config.hpp"

#include <map>

struct SubmeshGeometry
{
	UINT index_count = 0;
	UINT start_index_location = 0;
	INT base_vertex_location = 0;

	DirectX::BoundingBox bounds;
};

struct Mesh
{
	std::string name;

	Microsoft::WRL::ComPtr<ID3DBlob> vertex_buffer_cpu;
	Microsoft::WRL::ComPtr<ID3DBlob> index_buffer_cpu;

	Microsoft::WRL::ComPtr<ID3D12Resource> vertex_buffer_gpu;
	Microsoft::WRL::ComPtr<ID3D12Resource> index_buffer_gpu;

	Microsoft::WRL::ComPtr<ID3D12Resource> vertex_buffer_uploader;
	Microsoft::WRL::ComPtr<ID3D12Resource> index_buffer_uploader;

	UINT vertex_buffer_stride_in_bytes = 0;
	UINT vertex_buffer_size_in_bytes = 0;
	DXGI_FORMAT index_buffer_format = DXGI_FORMAT_R16_UINT;
	UINT index_buffer_size_in_bytes = 0;

	std::map<std::string, SubmeshGeometry> draw_args;

	D3D12_VERTEX_BUFFER_VIEW vertexBufferView() const
	{
		D3D12_VERTEX_BUFFER_VIEW view;
		view.BufferLocation = vertex_buffer_gpu->GetGPUVirtualAddress();
		view.StrideInBytes = vertex_buffer_stride_in_bytes;
		view.SizeInBytes = vertex_buffer_size_in_bytes;

		return view;
	}

	D3D12_INDEX_BUFFER_VIEW indexBufferView() const
	{
		D3D12_INDEX_BUFFER_VIEW view;
		view.BufferLocation = index_buffer_gpu->GetGPUVirtualAddress();
		view.Format = index_buffer_format;
		view.SizeInBytes = index_buffer_size_in_bytes;

		return view;
	}

	void disposeUploaders()
	{
		vertex_buffer_uploader = nullptr;
		index_buffer_uploader = nullptr;
	}
};
